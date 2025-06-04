#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWErrorHandlingTest : public ::testing::Test {
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

// Fehlerbehandlungs-Initialisierung Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingInitialization) {
    auto errorConfig = daw->createErrorHandlingConfig();
    errorConfig.enableLogging = true;
    errorConfig.enableCrashReporting = true;
    errorConfig.enableErrorRecovery = true;
    
    auto initResult = daw->initializeErrorHandling(errorConfig);
    EXPECT_TRUE(initResult.success);
    
    auto errorInfo = daw->getErrorHandlingInfo();
    EXPECT_TRUE(errorInfo.loggingEnabled);
    EXPECT_TRUE(errorInfo.crashReportingEnabled);
    EXPECT_TRUE(errorInfo.errorRecoveryEnabled);
}

// Audio-Fehlerbehandlung Test
TEST_F(VRDAWErrorHandlingTest, AudioErrorHandling) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere Audio-Fehler
    daw->simulateAudioError("buffer_underrun");
    auto errorInfo = daw->getLastError();
    EXPECT_EQ(errorInfo.type, "audio_error");
    EXPECT_EQ(errorInfo.code, "buffer_underrun");
    
    // Überprüfe Fehlerbehandlung
    auto recoveryResult = daw->handleAudioError(errorInfo);
    EXPECT_TRUE(recoveryResult.success);
    
    // Überprüfe Fehlerprotokoll
    auto errorLog = daw->getErrorLog();
    EXPECT_FALSE(errorLog.empty());
    EXPECT_TRUE(errorLog.hasAudioErrors);
}

// MIDI-Fehlerbehandlung Test
TEST_F(VRDAWErrorHandlingTest, MIDIErrorHandling) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere MIDI-Fehler
    daw->simulateMIDIError("device_not_found");
    auto errorInfo = daw->getLastError();
    EXPECT_EQ(errorInfo.type, "midi_error");
    EXPECT_EQ(errorInfo.code, "device_not_found");
    
    // Überprüfe Fehlerbehandlung
    auto recoveryResult = daw->handleMIDIError(errorInfo);
    EXPECT_TRUE(recoveryResult.success);
    
    // Überprüfe Fehlerprotokoll
    auto errorLog = daw->getErrorLog();
    EXPECT_FALSE(errorLog.empty());
    EXPECT_TRUE(errorLog.hasMIDIErrors);
}

// Plugin-Fehlerbehandlung Test
TEST_F(VRDAWErrorHandlingTest, PluginErrorHandling) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere Plugin-Fehler
    daw->simulatePluginError("load_failed");
    auto errorInfo = daw->getLastError();
    EXPECT_EQ(errorInfo.type, "plugin_error");
    EXPECT_EQ(errorInfo.code, "load_failed");
    
    // Überprüfe Fehlerbehandlung
    auto recoveryResult = daw->handlePluginError(errorInfo);
    EXPECT_TRUE(recoveryResult.success);
    
    // Überprüfe Fehlerprotokoll
    auto errorLog = daw->getErrorLog();
    EXPECT_FALSE(errorLog.empty());
    EXPECT_TRUE(errorLog.hasPluginErrors);
}

// Fehlerbehandlungs-Performance Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingPerformance) {
    const int DURATION_SECONDS = 30;
    
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int errorCount = 0;
    int failedRecoveries = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Simuliere Fehler
        daw->simulateRandomError();
        errorCount++;
        
        // Behandle Fehler
        auto errorInfo = daw->getLastError();
        auto recoveryResult = daw->handleError(errorInfo);
        
        if (!recoveryResult.success) {
            failedRecoveries++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getErrorHandlingMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 256); // Maximal 256MB RAM
        EXPECT_LT(metrics.recoveryTime, 100); // Maximal 100ms Wiederherstellungszeit
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedRecoveries / errorCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Fehlerbehandlungs-Fehlerbehandlung Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingErrorHandling) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere Fehlerbehandlungs-Fehler
    daw->simulateErrorHandlingError("recovery_failed");
    auto errorInfo = daw->getLastError();
    EXPECT_EQ(errorInfo.type, "error_handling_error");
    EXPECT_EQ(errorInfo.code, "recovery_failed");
    
    // Überprüfe Fehlerbehandlung
    auto recoveryResult = daw->handleErrorHandlingError(errorInfo);
    EXPECT_TRUE(recoveryResult.success);
    
    // Überprüfe Fehlerprotokoll
    auto errorLog = daw->getErrorLog();
    EXPECT_FALSE(errorLog.empty());
    EXPECT_TRUE(errorLog.hasErrorHandlingErrors);
}

// Fehlerbehandlungs-Integration Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingIntegration) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Überprüfe Fehlerbehandlungs-Integration
    auto errorInfo = daw->getErrorHandlingInfo();
    EXPECT_TRUE(errorInfo.hasAudioErrorHandling);
    EXPECT_TRUE(errorInfo.hasMIDIErrorHandling);
    EXPECT_TRUE(errorInfo.hasPluginErrorHandling);
    
    // Überprüfe Fehlerbehandlungs-Zugriff
    auto audioErrorHandling = daw->getAudioErrorHandling();
    auto midiErrorHandling = daw->getMIDIErrorHandling();
    auto pluginErrorHandling = daw->getPluginErrorHandling();
    
    EXPECT_TRUE(audioErrorHandling.isValid);
    EXPECT_TRUE(midiErrorHandling.isValid);
    EXPECT_TRUE(pluginErrorHandling.isValid);
}

// Fehlerbehandlungs-Protokollierung Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingLogging) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere verschiedene Fehler
    daw->simulateAudioError("buffer_underrun");
    daw->simulateMIDIError("device_not_found");
    daw->simulatePluginError("load_failed");
    
    // Überprüfe Fehlerprotokoll
    auto errorLog = daw->getErrorLog();
    EXPECT_FALSE(errorLog.empty());
    
    // Überprüfe Protokoll-Details
    EXPECT_TRUE(errorLog.hasAudioErrors);
    EXPECT_TRUE(errorLog.hasMIDIErrors);
    EXPECT_TRUE(errorLog.hasPluginErrors);
    
    // Überprüfe Protokoll-Format
    EXPECT_TRUE(errorLog.hasTimestamps);
    EXPECT_TRUE(errorLog.hasErrorCodes);
    EXPECT_TRUE(errorLog.hasStackTraces);
}

// Fehlerbehandlungs-Wiederherstellung Test
TEST_F(VRDAWErrorHandlingTest, ErrorHandlingRecovery) {
    daw->initializeErrorHandling(daw->createErrorHandlingConfig());
    
    // Simuliere System-Fehler
    daw->simulateSystemError("crash");
    
    // Überprüfe Wiederherstellung
    auto recoveryResult = daw->recoverFromError();
    EXPECT_TRUE(recoveryResult.success);
    
    // Überprüfe System-Status
    auto systemStatus = daw->getSystemStatus();
    EXPECT_TRUE(systemStatus.isStable);
    EXPECT_FALSE(systemStatus.hasErrors);
    
    // Überprüfe Wiederherstellungs-Protokoll
    auto recoveryLog = daw->getRecoveryLog();
    EXPECT_FALSE(recoveryLog.empty());
    EXPECT_TRUE(recoveryLog.hasRecoverySteps);
}

} // namespace Tests
} // namespace VR_DAW 