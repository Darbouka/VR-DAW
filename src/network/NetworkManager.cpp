#include "NetworkManager.hpp"
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <json/json.h>
#include <thread>
#include <mutex>
#include <queue>

namespace VR_DAW {

class NetworkManager::Impl {
public:
    using Client = websocketpp::client<websocketpp::config::asio_tls_client>;
    using MessagePtr = websocketpp::config::asio_client::message_type::ptr;
    using ConnectionHandle = websocketpp::connection_hdl;
    
    Client client;
    std::thread networkThread;
    std::mutex mutex;
    std::queue<NetworkMessage> messageQueue;
    bool isConnected;
    std::string serverUrl;
    std::string sessionId;
    std::vector<NetworkUser> connectedUsers;
    
    Impl() : isConnected(false) {
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.set_access_channels(websocketpp::log::alevel::connect);
        client.set_access_channels(websocketpp::log::alevel::disconnect);
        client.set_access_channels(websocketpp::log::alevel::app);
        
        client.init_asio();
        
        client.set_tls_init_handler([](ConnectionHandle) {
            return websocketpp::lib::make_shared<boost::asio::ssl::context>(
                boost::asio::ssl::context::tlsv12);
        });
        
        client.set_message_handler([this](ConnectionHandle hdl, MessagePtr msg) {
            handleMessage(hdl, msg);
        });
        
        client.set_open_handler([this](ConnectionHandle hdl) {
            handleConnection(hdl);
        });
        
        client.set_close_handler([this](ConnectionHandle hdl) {
            handleDisconnection(hdl);
        });
    }
    
    void handleMessage(ConnectionHandle hdl, MessagePtr msg) {
        try {
            Json::Value root;
            Json::Reader reader;
            if (reader.parse(msg->get_payload(), root)) {
                NetworkMessage networkMsg;
                networkMsg.type = static_cast<NetworkMessage::Type>(root["type"].asInt());
                networkMsg.senderId = root["senderId"].asString();
                networkMsg.data = root["data"].asString();
                
                std::lock_guard<std::mutex> lock(mutex);
                messageQueue.push(networkMsg);
            }
        } catch (const std::exception& e) {
            // Fehlerbehandlung
        }
    }
    
    void handleConnection(ConnectionHandle hdl) {
        isConnected = true;
        // Verbindungsaufbau-Logik
    }
    
    void handleDisconnection(ConnectionHandle hdl) {
        isConnected = false;
        // Verbindungsabbau-Logik
    }
};

NetworkManager::NetworkManager() : pImpl(std::make_unique<Impl>()) {}

NetworkManager::~NetworkManager() {
    disconnect();
}

void NetworkManager::connect(const std::string& url) {
    try {
        pImpl->serverUrl = url;
        websocketpp::lib::error_code ec;
        pImpl->client.connect(url, ec);
        
        if (ec) {
            // Handle connection error
        }
        
        pImpl->networkThread = std::thread([this]() {
            pImpl->client.run();
        });
    } catch (const std::exception& e) {
        // Handle connection exception
    }
}

void NetworkManager::disconnect() {
    if (pImpl->isConnected) {
        pImpl->client.close(websocketpp::close::status::normal, "Disconnecting");
        if (pImpl->networkThread.joinable()) {
            pImpl->networkThread.join();
        }
    }
}

void NetworkManager::sendMessage(const std::string& message) {
    // Implementation needed
}

std::vector<std::string> NetworkManager::getMessages() {
    // Implementation needed
    return {};
}

bool NetworkManager::isConnected() const {
    return pImpl->isConnected;
}

std::vector<NetworkUser> NetworkManager::getConnectedUsers() const {
    return pImpl->connectedUsers;
}

void NetworkManager::updateUserStatus(const NetworkUser& user) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = std::find_if(pImpl->connectedUsers.begin(), pImpl->connectedUsers.end(),
        [&user](const NetworkUser& u) { return u.id == user.id; });
    
    if (it != pImpl->connectedUsers.end()) {
        *it = user;
    } else {
        pImpl->connectedUsers.push_back(user);
    }
}

void NetworkManager::removeUser(const std::string& userId) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->connectedUsers.erase(
        std::remove_if(pImpl->connectedUsers.begin(), pImpl->connectedUsers.end(),
            [&userId](const NetworkUser& u) { return u.id == userId; }),
        pImpl->connectedUsers.end()
    );
}

} // namespace VR_DAW 