#include "CollaborationManager.hpp"
#include <json/json.h>
#include <chrono>
#include <thread>

namespace VR_DAW {

CollaborationManager& CollaborationManager::getInstance() {
    static CollaborationManager instance;
    return instance;
}

void CollaborationManager::initialize() {
    // WebSocket-Client initialisieren
    wsClient = std::make_unique<websocketpp::client<websocketpp::config::asio>>();
    wsClient->clear_access_channels(websocketpp::log::alevel::all);
    wsClient->set_access_channels(websocketpp::log::alevel::connect);
    wsClient->set_access_channels(websocketpp::log::alevel::disconnect);
    wsClient->set_access_channels(websocketpp::log::alevel::app);
    
    // WebSocket-Server initialisieren
    wsServer = std::make_unique<websocketpp::server<websocketpp::config::asio>>();
    wsServer->clear_access_channels(websocketpp::log::alevel::all);
    wsServer->set_access_channels(websocketpp::log::alevel::connect);
    wsServer->set_access_channels(websocketpp::log::alevel::disconnect);
    wsServer->set_access_channels(websocketpp::log::alevel::app);
}

void CollaborationManager::shutdown() {
    disconnect();
    wsClient.reset();
    wsServer.reset();
}

void CollaborationManager::connect(const std::string& serverUrl) {
    if (connected) return;
    
    try {
        wsClient->init_asio();
        
        wsClient->set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio::message_type::ptr msg) {
            handleWebSocketMessage(msg->get_payload());
        });
        
        wsClient->set_open_handler([this](websocketpp::connection_hdl hdl) {
            handleWebSocketConnection(hdl.lock()->get_remote_endpoint());
        });
        
        wsClient->set_close_handler([this](websocketpp::connection_hdl hdl) {
            handleWebSocketDisconnection(hdl.lock()->get_remote_endpoint());
        });
        
        websocketpp::lib::error_code ec;
        websocketpp::client<websocketpp::config::asio>::connection_ptr con = wsClient->get_connection(serverUrl, ec);
        
        if (ec) {
            throw std::runtime_error("Verbindungsfehler: " + ec.message());
        }
        
        wsClient->connect(con);
        wsClient->run();
        
        connected = true;
    }
    catch (const std::exception& e) {
        // Fehlerbehandlung
        connected = false;
    }
}

void CollaborationManager::disconnect() {
    if (!connected) return;
    
    try {
        wsClient->stop();
        connected = false;
    }
    catch (const std::exception& e) {
        // Fehlerbehandlung
    }
}

bool CollaborationManager::isConnected() const {
    return connected;
}

void CollaborationManager::createProject(const std::string& name) {
    Project project;
    project.id = generateUniqueId();
    project.name = name;
    projects[project.id] = project;
    
    CollaborationEvent event;
    event.type = "project_created";
    event.projectId = project.id;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::joinProject(const std::string& projectId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    currentProjectId = projectId;
    
    CollaborationEvent event;
    event.type = "project_joined";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::leaveProject(const std::string& projectId) {
    if (currentProjectId != projectId) return;
    
    CollaborationEvent event;
    event.type = "project_left";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
    
    currentProjectId.clear();
}

void CollaborationManager::shareProject(const std::string& projectId, const std::string& userId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    it->second.users.push_back(userId);
    
    CollaborationEvent event;
    event.type = "project_shared";
    event.projectId = projectId;
    event.userId = userId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::startCollaboration(const std::string& projectId) {
    if (currentProjectId != projectId) return;
    
    CollaborationEvent event;
    event.type = "collaboration_started";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::stopCollaboration(const std::string& projectId) {
    if (currentProjectId != projectId) return;
    
    CollaborationEvent event;
    event.type = "collaboration_stopped";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::syncProject(const std::string& projectId) {
    if (currentProjectId != projectId) return;
    
    syncProjectData(projectId);
    
    CollaborationEvent event;
    event.type = "project_synced";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::inviteUser(const std::string& userId) {
    if (currentProjectId.empty()) return;
    
    CollaborationEvent event;
    event.type = "user_invited";
    event.userId = userId;
    event.projectId = currentProjectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::removeUser(const std::string& userId) {
    if (currentProjectId.empty()) return;
    
    auto it = projects.find(currentProjectId);
    if (it == projects.end()) return;
    
    auto& users = it->second.users;
    users.erase(std::remove(users.begin(), users.end(), userId), users.end());
    
    CollaborationEvent event;
    event.type = "user_removed";
    event.userId = userId;
    event.projectId = currentProjectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::setUserRole(const std::string& userId, const std::string& role) {
    if (currentProjectId.empty()) return;
    
    auto it = projects.find(currentProjectId);
    if (it == projects.end()) return;
    
    it->second.userRoles[userId] = role;
    
    CollaborationEvent event;
    event.type = "user_role_changed";
    event.userId = userId;
    event.projectId = currentProjectId;
    event.data = role;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::sendMessage(const std::string& message) {
    if (currentProjectId.empty()) return;
    
    CollaborationEvent event;
    event.type = "message";
    event.projectId = currentProjectId;
    event.data = message;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::sendAudioMessage(const std::string& audioData) {
    if (currentProjectId.empty()) return;
    
    CollaborationEvent event;
    event.type = "audio_message";
    event.projectId = currentProjectId;
    event.data = audioData;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::sendVideoMessage(const std::string& videoData) {
    if (currentProjectId.empty()) return;
    
    CollaborationEvent event;
    event.type = "video_message";
    event.projectId = currentProjectId;
    event.data = videoData;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::createVersion(const std::string& projectId, const std::string& name) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    std::string versionId = generateUniqueId();
    it->second.versions.push_back(versionId);
    
    CollaborationEvent event;
    event.type = "version_created";
    event.projectId = projectId;
    event.data = versionId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::restoreVersion(const std::string& projectId, const std::string& versionId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    CollaborationEvent event;
    event.type = "version_restored";
    event.projectId = projectId;
    event.data = versionId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::compareVersions(const std::string& versionId1, const std::string& versionId2) {
    if (currentProjectId.empty()) return;
    
    CollaborationEvent event;
    event.type = "versions_compared";
    event.projectId = currentProjectId;
    event.data = versionId1 + ":" + versionId2;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::syncToCloud(const std::string& projectId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    // Cloud-Synchronisation implementieren
    // Hier würde die Implementierung der Cloud-Synchronisation folgen
    
    CollaborationEvent event;
    event.type = "cloud_sync_started";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::syncFromCloud(const std::string& projectId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    // Cloud-Synchronisation implementieren
    // Hier würde die Implementierung der Cloud-Synchronisation folgen
    
    CollaborationEvent event;
    event.type = "cloud_sync_completed";
    event.projectId = projectId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::setAutoSync(bool enable) {
    // Auto-Sync-Implementierung
    // Hier würde die Implementierung der Auto-Sync-Funktionalität folgen
}

void CollaborationManager::setPermission(const std::string& userId, const std::string& permission) {
    if (currentProjectId.empty()) return;
    
    auto it = projects.find(currentProjectId);
    if (it == projects.end()) return;
    
    it->second.permissions[userId].push_back(permission);
    
    CollaborationEvent event;
    event.type = "permission_granted";
    event.userId = userId;
    event.projectId = currentProjectId;
    event.data = permission;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::removePermission(const std::string& userId, const std::string& permission) {
    if (currentProjectId.empty()) return;
    
    auto it = projects.find(currentProjectId);
    if (it == projects.end()) return;
    
    auto& permissions = it->second.permissions[userId];
    permissions.erase(std::remove(permissions.begin(), permissions.end(), permission), permissions.end());
    
    CollaborationEvent event;
    event.type = "permission_removed";
    event.userId = userId;
    event.projectId = currentProjectId;
    event.data = permission;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

bool CollaborationManager::hasPermission(const std::string& userId, const std::string& permission) const {
    if (currentProjectId.empty()) return false;
    
    auto it = projects.find(currentProjectId);
    if (it == projects.end()) return false;
    
    const auto& permissions = it->second.permissions.at(userId);
    return std::find(permissions.begin(), permissions.end(), permission) != permissions.end();
}

void CollaborationManager::registerEventCallback(const std::string& eventType, EventCallback callback) {
    eventCallbacks[eventType].push_back(callback);
}

void CollaborationManager::unregisterEventCallback(const std::string& eventType) {
    eventCallbacks.erase(eventType);
}

void CollaborationManager::handleWebSocketMessage(const std::string& message) {
    Json::Value root;
    Json::Reader reader;
    
    if (reader.parse(message, root)) {
        CollaborationEvent event;
        event.type = root["type"].asString();
        event.userId = root["userId"].asString();
        event.projectId = root["projectId"].asString();
        event.data = root["data"].asString();
        event.timestamp = std::chrono::system_clock::now();
        
        processEvent(event);
    }
}

void CollaborationManager::handleWebSocketConnection(const std::string& connectionId) {
    CollaborationEvent event;
    event.type = "connection_established";
    event.data = connectionId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::handleWebSocketDisconnection(const std::string& connectionId) {
    CollaborationEvent event;
    event.type = "connection_closed";
    event.data = connectionId;
    event.timestamp = std::chrono::system_clock::now();
    broadcastEvent(event);
}

void CollaborationManager::broadcastEvent(const CollaborationEvent& event) {
    Json::Value root;
    root["type"] = event.type;
    root["userId"] = event.userId;
    root["projectId"] = event.projectId;
    root["data"] = event.data;
    root["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        event.timestamp.time_since_epoch()).count();
    
    Json::FastWriter writer;
    std::string message = writer.write(root);
    
    if (wsClient) {
        wsClient->send(message, websocketpp::frame::opcode::text);
    }
}

void CollaborationManager::processEvent(const CollaborationEvent& event) {
    auto it = eventCallbacks.find(event.type);
    if (it != eventCallbacks.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

void CollaborationManager::syncProjectData(const std::string& projectId) {
    auto it = projects.find(projectId);
    if (it == projects.end()) return;
    
    // Projekt-Daten synchronisieren
    // Hier würde die Implementierung der Daten-Synchronisation folgen
}

void CollaborationManager::handleVersionConflict(const std::string& projectId) {
    // Versionskonflikte behandeln
    // Hier würde die Implementierung der Konfliktbehandlung folgen
}

std::string CollaborationManager::generateUniqueId() {
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

} // namespace VR_DAW 