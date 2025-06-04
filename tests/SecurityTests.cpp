#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWSecurityTest : public ::testing::Test {
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

// Sicherheits-Initialisierung Test
TEST_F(VRDAWSecurityTest, SecurityInitialization) {
    auto securityConfig = daw->createSecurityConfig();
    securityConfig.enableEncryption = true;
    securityConfig.enableAuthentication = true;
    securityConfig.enableAuthorization = true;
    
    auto initResult = daw->initializeSecurity(securityConfig);
    EXPECT_TRUE(initResult.success);
    
    auto securityInfo = daw->getSecurityInfo();
    EXPECT_TRUE(securityInfo.encryptionEnabled);
    EXPECT_TRUE(securityInfo.authenticationEnabled);
    EXPECT_TRUE(securityInfo.authorizationEnabled);
}

// Verschlüsselungs-Test
TEST_F(VRDAWSecurityTest, EncryptionTest) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Teste Verschlüsselung
    auto data = "Test Data";
    auto encryptedData = daw->encryptData(data);
    EXPECT_FALSE(encryptedData.empty());
    
    // Teste Entschlüsselung
    auto decryptedData = daw->decryptData(encryptedData);
    EXPECT_EQ(decryptedData, data);
    
    // Überprüfe Verschlüsselungs-Status
    auto encryptionInfo = daw->getEncryptionInfo();
    EXPECT_TRUE(encryptionInfo.isEncrypted);
    EXPECT_TRUE(encryptionInfo.hasValidKey);
}

// Authentifizierungs-Test
TEST_F(VRDAWSecurityTest, AuthenticationTest) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Teste Benutzer-Authentifizierung
    auto authResult = daw->authenticateUser("test_user", "test_password");
    EXPECT_TRUE(authResult.success);
    
    // Überprüfe Authentifizierungs-Status
    auto authInfo = daw->getAuthenticationInfo();
    EXPECT_TRUE(authInfo.isAuthenticated);
    EXPECT_EQ(authInfo.username, "test_user");
}

// Autorisierungs-Test
TEST_F(VRDAWSecurityTest, AuthorizationTest) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Teste Benutzer-Autorisierung
    auto authResult = daw->authorizeUser("test_user", "admin");
    EXPECT_TRUE(authResult.success);
    
    // Überprüfe Autorisierungs-Status
    auto authInfo = daw->getAuthorizationInfo();
    EXPECT_TRUE(authInfo.isAuthorized);
    EXPECT_EQ(authInfo.role, "admin");
}

// Sicherheits-Performance Test
TEST_F(VRDAWSecurityTest, SecurityPerformance) {
    const int DURATION_SECONDS = 30;
    
    daw->initializeSecurity(daw->createSecurityConfig());
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int operationCount = 0;
    int failedOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe Sicherheits-Operationen durch
        auto data = "Test Data " + std::to_string(operationCount);
        auto encryptedData = daw->encryptData(data);
        auto decryptedData = daw->decryptData(encryptedData);
        
        operationCount++;
        
        if (decryptedData != data) {
            failedOperations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getSecurityMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 256); // Maximal 256MB RAM
        EXPECT_LT(metrics.operationTime, 100); // Maximal 100ms Operationszeit
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedOperations / operationCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Sicherheits-Fehlerbehandlung Test
TEST_F(VRDAWSecurityTest, SecurityErrorHandling) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Teste ungültige Authentifizierung
    auto authResult = daw->authenticateUser("", "");
    EXPECT_FALSE(authResult.success);
    
    // Teste ungültige Autorisierung
    auto authResult2 = daw->authorizeUser("test_user", "");
    EXPECT_FALSE(authResult2.success);
    
    // Teste ungültige Verschlüsselung
    auto encryptedData = daw->encryptData("");
    EXPECT_TRUE(encryptedData.empty());
}

// Sicherheits-Integration Test
TEST_F(VRDAWSecurityTest, SecurityIntegration) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Überprüfe Sicherheits-Integration
    auto securityInfo = daw->getSecurityInfo();
    EXPECT_TRUE(securityInfo.hasEncryption);
    EXPECT_TRUE(securityInfo.hasAuthentication);
    EXPECT_TRUE(securityInfo.hasAuthorization);
    
    // Überprüfe Sicherheits-Zugriff
    auto encryptionInfo = daw->getEncryptionInfo();
    auto authenticationInfo = daw->getAuthenticationInfo();
    auto authorizationInfo = daw->getAuthorizationInfo();
    
    EXPECT_TRUE(encryptionInfo.isValid);
    EXPECT_TRUE(authenticationInfo.isValid);
    EXPECT_TRUE(authorizationInfo.isValid);
}

// Sicherheits-Protokollierung Test
TEST_F(VRDAWSecurityTest, SecurityLogging) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Simuliere Sicherheits-Ereignisse
    daw->logSecurityEvent("login_attempt", "test_user");
    daw->logSecurityEvent("authorization_change", "test_user");
    daw->logSecurityEvent("encryption_operation", "test_data");
    
    // Überprüfe Sicherheitsprotokoll
    auto securityLog = daw->getSecurityLog();
    EXPECT_FALSE(securityLog.empty());
    
    // Überprüfe Protokoll-Details
    EXPECT_TRUE(securityLog.hasLoginAttempts);
    EXPECT_TRUE(securityLog.hasAuthorizationChanges);
    EXPECT_TRUE(securityLog.hasEncryptionOperations);
    
    // Überprüfe Protokoll-Format
    EXPECT_TRUE(securityLog.hasTimestamps);
    EXPECT_TRUE(securityLog.hasEventTypes);
    EXPECT_TRUE(securityLog.hasUserInfo);
}

// Sicherheits-Audit Test
TEST_F(VRDAWSecurityTest, SecurityAudit) {
    daw->initializeSecurity(daw->createSecurityConfig());
    
    // Führe Sicherheits-Audit durch
    auto auditResult = daw->performSecurityAudit();
    EXPECT_TRUE(auditResult.success);
    
    // Überprüfe Audit-Ergebnisse
    auto auditInfo = daw->getAuditInfo();
    EXPECT_TRUE(auditInfo.hasVulnerabilities);
    EXPECT_TRUE(auditInfo.hasRecommendations);
    
    // Überprüfe Sicherheits-Status
    auto securityStatus = daw->getSecurityStatus();
    EXPECT_TRUE(securityStatus.isSecure);
    EXPECT_FALSE(securityStatus.hasVulnerabilities);
}

} // namespace Tests
} // namespace VR_DAW 