#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWNetworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
};

// Netzwerk-Initialisierung Test
TEST_F(VRDAWNetworkTest, NetworkInitialization) {
    auto networkConfig = daw->createNetworkConfig();
    networkConfig.port = 8080;
    networkConfig.maxConnections = 10;
    
    auto initResult = daw->initializeNetwork(networkConfig);
    EXPECT_TRUE(initResult.success);
    
    auto networkInfo = daw->getNetworkInfo();
    EXPECT_EQ(networkInfo.port, 8080);
    EXPECT_EQ(networkInfo.maxConnections, 10);
    EXPECT_EQ(networkInfo.connectionCount, 0);
}

// Verbindungs-Test
TEST_F(VRDAWNetworkTest, ConnectionTest) {
    daw->initializeNetwork(daw->createNetworkConfig());
    
    // Simuliere Client-Verbindung
    auto connectionResult = daw->simulateClientConnection("127.0.0.1");
    EXPECT_TRUE(connectionResult.success);
    
    auto networkInfo = daw->getNetworkInfo();
    EXPECT_EQ(networkInfo.connectionCount, 1);
    
    // Trenne Verbindung
    daw->disconnectClient(connectionResult.clientId);
    networkInfo = daw->getNetworkInfo();
    EXPECT_EQ(networkInfo.connectionCount, 0);
}

// Daten-Synchronisation Test
TEST_F(VRDAWNetworkTest, DataSynchronization) {
    daw->initializeNetwork(daw->createNetworkConfig());
    
    // Erstelle Test-Projekt
    auto projectId = daw->createProject("Sync Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Simuliere Client-Verbindung
    auto connectionResult = daw->simulateClientConnection("127.0.0.1");
    
    // Synchronisiere Projekt
    auto syncResult = daw->synchronizeProject(projectId, connectionResult.clientId);
    EXPECT_TRUE(syncResult.success);
    
    // Überprüfe Synchronisation
    auto syncInfo = daw->getSynchronizationInfo(projectId);
    EXPECT_EQ(syncInfo.syncStatus, "Synchronized");
    EXPECT_EQ(syncInfo.clientCount, 1);
}

// Netzwerk-Performance Test
TEST_F(VRDAWNetworkTest, NetworkPerformance) {
    const int DURATION_SECONDS = 30;
    
    daw->initializeNetwork(daw->createNetworkConfig());
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int messageCount = 0;
    int failedMessages = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Simuliere Client-Verbindung
        auto connectionResult = daw->simulateClientConnection("127.0.0.1");
        
        // Sende Test-Nachricht
        auto messageResult = daw->sendNetworkMessage(
            connectionResult.clientId,
            "Test Message",
            "Test Data"
        );
        messageCount++;
        
        if (!messageResult.success) {
            failedMessages++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getNetworkMetrics();
        EXPECT_LT(metrics.cpuUsage, 30.0f); // Maximal 30% CPU
        EXPECT_LT(metrics.memoryUsage, 512); // Maximal 512MB RAM
        EXPECT_LT(metrics.networkLatency, 100); // Maximal 100ms Latenz
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedMessages / messageCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Netzwerk-Fehlerbehandlung Test
TEST_F(VRDAWNetworkTest, NetworkErrorHandling) {
    daw->initializeNetwork(daw->createNetworkConfig());
    
    // Teste ungültige Verbindungen
    auto connectionResult = daw->simulateClientConnection("");
    EXPECT_FALSE(connectionResult.success);
    
    // Teste ungültige Nachrichten
    auto messageResult = daw->sendNetworkMessage(0, "", "");
    EXPECT_FALSE(messageResult.success);
    
    // Teste Verbindungsabbruch
    daw->simulateConnectionDrop(connectionResult.clientId);
    auto networkInfo = daw->getNetworkInfo();
    EXPECT_EQ(networkInfo.connectionCount, 0);
}

// Netzwerk-Integration Test
TEST_F(VRDAWNetworkTest, NetworkIntegration) {
    daw->initializeNetwork(daw->createNetworkConfig());
    
    // Erstelle Test-Projekt
    auto projectId = daw->createProject("Network Integration Test");
    
    // Simuliere mehrere Clients
    std::vector<int> clientIds;
    for (int i = 0; i < 5; i++) {
        auto connectionResult = daw->simulateClientConnection("127.0.0.1");
        clientIds.push_back(connectionResult.clientId);
    }
    
    // Synchronisiere Projekt mit allen Clients
    for (auto clientId : clientIds) {
        auto syncResult = daw->synchronizeProject(projectId, clientId);
        EXPECT_TRUE(syncResult.success);
    }
    
    // Überprüfe Netzwerk-Integration
    auto networkInfo = daw->getNetworkInfo();
    EXPECT_EQ(networkInfo.connectionCount, 5);
    
    auto syncInfo = daw->getSynchronizationInfo(projectId);
    EXPECT_EQ(syncInfo.clientCount, 5);
}

// Netzwerk-Sicherheit Test
TEST_F(VRDAWNetworkTest, NetworkSecurity) {
    auto networkConfig = daw->createNetworkConfig();
    networkConfig.enableEncryption = true;
    networkConfig.requireAuthentication = true;
    
    daw->initializeNetwork(networkConfig);
    
    // Teste Authentifizierung
    auto authResult = daw->authenticateClient("test_user", "test_password");
    EXPECT_TRUE(authResult.success);
    
    // Teste Verschlüsselung
    auto messageResult = daw->sendEncryptedMessage(
        authResult.clientId,
        "Test Message",
        "Test Data"
    );
    EXPECT_TRUE(messageResult.success);
    
    // Überprüfe Sicherheit
    auto securityInfo = daw->getNetworkSecurityInfo();
    EXPECT_TRUE(securityInfo.encryptionEnabled);
    EXPECT_TRUE(securityInfo.authenticationRequired);
    EXPECT_EQ(securityInfo.authenticatedClients, 1);
}

} // namespace Tests
} // namespace VR_DAW 