#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWConfigurationTest : public ::testing::Test {
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

// Konfigurations-Initialisierung Test
TEST_F(VRDAWConfigurationTest, ConfigurationInitialization) {
    auto config = daw->createConfiguration();
    config.audio.sampleRate = 48000;
    config.audio.bufferSize = 1024;
    config.audio.channels = 2;
    
    auto initResult = daw->initializeConfiguration(config);
    EXPECT_TRUE(initResult.success);
    
    auto configInfo = daw->getConfigurationInfo();
    EXPECT_EQ(configInfo.audio.sampleRate, 48000);
    EXPECT_EQ(configInfo.audio.bufferSize, 1024);
    EXPECT_EQ(configInfo.audio.channels, 2);
}

// Audio-Konfiguration Test
TEST_F(VRDAWConfigurationTest, AudioConfiguration) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Konfiguriere Audio
    auto audioConfig = daw->createAudioConfig();
    audioConfig.sampleRate = 48000;
    audioConfig.bufferSize = 1024;
    audioConfig.channels = 2;
    audioConfig.enableASIO = true;
    
    daw->setAudioConfiguration(audioConfig);
    
    // Überprüfe Audio-Konfiguration
    auto audioInfo = daw->getAudioConfiguration();
    EXPECT_EQ(audioInfo.sampleRate, 48000);
    EXPECT_EQ(audioInfo.bufferSize, 1024);
    EXPECT_EQ(audioInfo.channels, 2);
    EXPECT_TRUE(audioInfo.asioEnabled);
}

// MIDI-Konfiguration Test
TEST_F(VRDAWConfigurationTest, MIDIConfiguration) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Konfiguriere MIDI
    auto midiConfig = daw->createMIDIConfig();
    midiConfig.enableMIDI = true;
    midiConfig.enableMIDIClock = true;
    midiConfig.midiChannels = 16;
    
    daw->setMIDIConfiguration(midiConfig);
    
    // Überprüfe MIDI-Konfiguration
    auto midiInfo = daw->getMIDIConfiguration();
    EXPECT_TRUE(midiInfo.midiEnabled);
    EXPECT_TRUE(midiInfo.midiClockEnabled);
    EXPECT_EQ(midiInfo.midiChannels, 16);
}

// Plugin-Konfiguration Test
TEST_F(VRDAWConfigurationTest, PluginConfiguration) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Konfiguriere Plugins
    auto pluginConfig = daw->createPluginConfig();
    pluginConfig.pluginDirectory = "/plugins";
    pluginConfig.enableVST3 = true;
    pluginConfig.enableAU = true;
    
    daw->setPluginConfiguration(pluginConfig);
    
    // Überprüfe Plugin-Konfiguration
    auto pluginInfo = daw->getPluginConfiguration();
    EXPECT_EQ(pluginInfo.pluginDirectory, "/plugins");
    EXPECT_TRUE(pluginInfo.vst3Enabled);
    EXPECT_TRUE(pluginInfo.auEnabled);
}

// Konfigurations-Performance Test
TEST_F(VRDAWConfigurationTest, ConfigurationPerformance) {
    const int DURATION_SECONDS = 30;
    
    daw->initializeConfiguration(daw->createConfiguration());
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int configCount = 0;
    int failedConfigs = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe Konfigurations-Änderungen durch
        auto config = daw->createConfiguration();
        config.audio.sampleRate = 44100 + (configCount % 2) * 4800;
        config.audio.bufferSize = 512 << (configCount % 3);
        
        auto configResult = daw->updateConfiguration(config);
        configCount++;
        
        if (!configResult.success) {
            failedConfigs++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getConfigurationMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 256); // Maximal 256MB RAM
        EXPECT_LT(metrics.updateTime, 100); // Maximal 100ms Update-Zeit
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedConfigs / configCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Konfigurations-Fehlerbehandlung Test
TEST_F(VRDAWConfigurationTest, ConfigurationErrorHandling) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Teste ungültige Konfigurationen
    auto invalidConfig = daw->createConfiguration();
    invalidConfig.audio.sampleRate = 0;
    invalidConfig.audio.bufferSize = 0;
    invalidConfig.audio.channels = 0;
    
    auto configResult = daw->updateConfiguration(invalidConfig);
    EXPECT_FALSE(configResult.success);
    
    // Teste ungültige Plugin-Konfiguration
    auto invalidPluginConfig = daw->createPluginConfig();
    invalidPluginConfig.pluginDirectory = "";
    
    auto pluginResult = daw->setPluginConfiguration(invalidPluginConfig);
    EXPECT_FALSE(pluginResult.success);
}

// Konfigurations-Integration Test
TEST_F(VRDAWConfigurationTest, ConfigurationIntegration) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Überprüfe Konfigurations-Integration
    auto configInfo = daw->getConfigurationInfo();
    EXPECT_TRUE(configInfo.hasAudioConfig);
    EXPECT_TRUE(configInfo.hasMIDIConfig);
    EXPECT_TRUE(configInfo.hasPluginConfig);
    
    // Überprüfe Konfigurations-Zugriff
    auto audioConfig = daw->getAudioConfiguration();
    auto midiConfig = daw->getMIDIConfiguration();
    auto pluginConfig = daw->getPluginConfiguration();
    
    EXPECT_TRUE(audioConfig.isValid);
    EXPECT_TRUE(midiConfig.isValid);
    EXPECT_TRUE(pluginConfig.isValid);
}

// Konfigurations-Speicherung Test
TEST_F(VRDAWConfigurationTest, ConfigurationStorage) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Konfiguriere System
    auto config = daw->createConfiguration();
    config.audio.sampleRate = 48000;
    config.audio.bufferSize = 1024;
    config.audio.channels = 2;
    
    daw->setConfiguration(config);
    
    // Speichere Konfiguration
    auto saveResult = daw->saveConfiguration("config.json");
    EXPECT_TRUE(saveResult.success);
    
    // Lade Konfiguration
    auto loadResult = daw->loadConfiguration("config.json");
    EXPECT_TRUE(loadResult.success);
    
    // Überprüfe geladene Konfiguration
    auto loadedConfig = daw->getConfiguration();
    EXPECT_EQ(loadedConfig.audio.sampleRate, 48000);
    EXPECT_EQ(loadedConfig.audio.bufferSize, 1024);
    EXPECT_EQ(loadedConfig.audio.channels, 2);
}

// Konfigurations-Validierung Test
TEST_F(VRDAWConfigurationTest, ConfigurationValidation) {
    daw->initializeConfiguration(daw->createConfiguration());
    
    // Teste Konfigurations-Validierung
    auto config = daw->createConfiguration();
    
    // Valide Konfiguration
    config.audio.sampleRate = 48000;
    config.audio.bufferSize = 1024;
    config.audio.channels = 2;
    
    auto validationResult = daw->validateConfiguration(config);
    EXPECT_TRUE(validationResult.success);
    
    // Ungültige Konfiguration
    config.audio.sampleRate = 0;
    config.audio.bufferSize = 0;
    config.audio.channels = 0;
    
    validationResult = daw->validateConfiguration(config);
    EXPECT_FALSE(validationResult.success);
}

} // namespace Tests
} // namespace VR_DAW 