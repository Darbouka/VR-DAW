#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>

namespace VR_DAW {

class CollaborationManager {
public:
    static CollaborationManager& getInstance();
    
    // Initialisierung
    void initialize();
    void shutdown();
    
    // Verbindungs-Management
    void connect(const std::string& serverUrl);
    void disconnect();
    bool isConnected() const;
    
    // Projekt-Management
    void createProject(const std::string& name);
    void joinProject(const std::string& projectId);
    void leaveProject(const std::string& projectId);
    void shareProject(const std::string& projectId, const std::string& userId);
    
    // Echtzeit-Zusammenarbeit
    void startCollaboration(const std::string& projectId);
    void stopCollaboration(const std::string& projectId);
    void syncProject(const std::string& projectId);
    
    // Benutzer-Management
    void inviteUser(const std::string& userId);
    void removeUser(const std::string& userId);
    void setUserRole(const std::string& userId, const std::string& role);
    
    // Chat und Kommunikation
    void sendMessage(const std::string& message);
    void sendAudioMessage(const std::string& audioData);
    void sendVideoMessage(const std::string& videoData);
    
    // Versionierung
    void createVersion(const std::string& projectId, const std::string& name);
    void restoreVersion(const std::string& projectId, const std::string& versionId);
    void compareVersions(const std::string& versionId1, const std::string& versionId2);
    
    // Cloud-Synchronisation
    void syncToCloud(const std::string& projectId);
    void syncFromCloud(const std::string& projectId);
    void setAutoSync(bool enable);
    
    // Berechtigungen
    void setPermission(const std::string& userId, const std::string& permission);
    void removePermission(const std::string& userId, const std::string& permission);
    bool hasPermission(const std::string& userId, const std::string& permission) const;
    
    // Events
    struct CollaborationEvent {
        std::string type;
        std::string userId;
        std::string projectId;
        std::string data;
        std::chrono::system_clock::time_point timestamp;
    };
    
    using EventCallback = std::function<void(const CollaborationEvent&)>;
    void registerEventCallback(const std::string& eventType, EventCallback callback);
    void unregisterEventCallback(const std::string& eventType);
    
private:
    CollaborationManager() = default;
    ~CollaborationManager() = default;
    
    CollaborationManager(const CollaborationManager&) = delete;
    CollaborationManager& operator=(const CollaborationManager&) = delete;
    
    // Interne Zustandsvariablen
    bool connected = false;
    std::string currentProjectId;
    std::map<std::string, std::vector<EventCallback>> eventCallbacks;
    
    // WebSocket-Komponenten
    std::unique_ptr<websocketpp::client<websocketpp::config::asio>> wsClient;
    std::unique_ptr<websocketpp::server<websocketpp::config::asio>> wsServer;
    
    // Projekt-Daten
    struct Project {
        std::string id;
        std::string name;
        std::vector<std::string> users;
        std::map<std::string, std::string> userRoles;
        std::map<std::string, std::vector<std::string>> permissions;
        std::vector<std::string> versions;
    };
    
    std::map<std::string, Project> projects;
    
    // Interne Hilfsfunktionen
    void handleWebSocketMessage(const std::string& message);
    void handleWebSocketConnection(const std::string& connectionId);
    void handleWebSocketDisconnection(const std::string& connectionId);
    void broadcastEvent(const CollaborationEvent& event);
    void processEvent(const CollaborationEvent& event);
    void syncProjectData(const std::string& projectId);
    void handleVersionConflict(const std::string& projectId);
};

} // namespace VR_DAW 