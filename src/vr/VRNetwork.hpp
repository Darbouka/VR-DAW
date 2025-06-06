#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <glm/glm.hpp>

namespace VR_DAW {

class VRNetwork {
public:
    VRNetwork();
    ~VRNetwork();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update();

    // Netzwerk-Status
    bool isConnected() const;
    bool isHost() const;
    bool isClient() const;
    std::string getConnectionStatus() const;

    // Verbindungsmanagement
    bool startHost(int port);
    bool connectToHost(const std::string& address, int port);
    void disconnect();
    void setMaxConnections(int maxConnections);

    // Datenübertragung
    void sendData(const std::string& channel, const void* data, size_t size);
    void broadcastData(const std::string& channel, const void* data, size_t size);
    void registerDataHandler(const std::string& channel, std::function<void(const void*, size_t)> handler);

    // Synchronisation
    void syncTransform(const std::string& objectId, const glm::vec3& position, const glm::quat& rotation);
    void syncAudio(const std::string& audioId, const void* audioData, size_t size);
    void syncEvent(const std::string& eventType, const void* eventData, size_t size);

    // Latenz und Qualität
    float getLatency() const;
    float getPacketLoss() const;
    void setQualitySettings(int quality);
    void setCompressionLevel(int level);

    // Benutzer-Management
    struct User {
        std::string id;
        std::string name;
        bool isHost;
        float latency;
    };
    std::vector<User> getConnectedUsers() const;
    void kickUser(const std::string& userId);
    void banUser(const std::string& userId);

    // Debug
    void enableDebugLogging(bool enable);
    void showNetworkStats();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool debugEnabled;
    
    // Netzwerk-Status
    bool connected;
    bool isHosting;
    int maxConnections;
    
    // Qualitätseinstellungen
    int qualityLevel;
    int compressionLevel;
    
    // Event-Handler
    std::map<std::string, std::vector<std::function<void(const void*, size_t)>>> dataHandlers;
    
    void processIncomingData();
    void updateNetworkStats();
    void handleConnectionEvents();
    void compressData(const void* data, size_t size, void* compressed, size_t& compressedSize);
    void decompressData(const void* compressed, size_t compressedSize, void* data, size_t& size);
};

} // namespace VR_DAW 