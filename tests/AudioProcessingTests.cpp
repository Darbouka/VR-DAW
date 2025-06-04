#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWAudioTest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
        daw->initializeAudio();
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
};

// Audio-Initialisierung Test
TEST_F(VRDAWAudioTest, AudioInitialization) {
    EXPECT_TRUE(daw->isAudioInitialized());
    EXPECT_TRUE(daw->isAudioEngineActive());
    
    auto audioInfo = daw->getAudioInfo();
    EXPECT_GT(audioInfo.sampleRate, 0);
    EXPECT_GT(audioInfo.bufferSize, 0);
    EXPECT_GT(audioInfo.channelCount, 0);
}

// Audio-Verarbeitung Test
TEST_F(VRDAWAudioTest, AudioProcessing) {
    const int BLOCK_COUNT = 1000;
    const int BLOCK_SIZE = 512;
    
    // Erstelle Test-Signal
    auto signal = daw->generateTestSignal(BLOCK_SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        daw->processAudioBlock(signal);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Verarbeitungs-Performance
    EXPECT_LT(duration.count(), BLOCK_COUNT * 5); // Maximal 5ms pro Block
    
    auto metrics = daw->getAudioMetrics();
    EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
    EXPECT_LT(metrics.latency, 10.0f); // Maximal 10ms Latenz
    EXPECT_EQ(metrics.bufferUnderruns, 0); // Keine Buffer Underruns
}

// Plugin-Verarbeitung Test
TEST_F(VRDAWAudioTest, PluginProcessing) {
    // Lade Test-Plugin
    auto pluginId = daw->loadPlugin("TestPlugin.vst3");
    EXPECT_TRUE(daw->isPluginLoaded(pluginId));
    
    // Konfiguriere Plugin
    daw->setPluginParameter(pluginId, "gain", 0.5f);
    daw->setPluginParameter(pluginId, "pan", 0.0f);
    
    // Verarbeite Audio
    const int BLOCK_COUNT = 100;
    auto signal = daw->generateTestSignal(512);
    
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        daw->processAudioWithPlugin(pluginId, signal);
    }
    
    // Überprüfe Plugin-Performance
    auto metrics = daw->getPluginMetrics(pluginId);
    EXPECT_LT(metrics.cpuUsage, 30.0f); // Maximal 30% CPU pro Plugin
    EXPECT_LT(metrics.memoryUsage, 100); // Maximal 100MB pro Plugin
    EXPECT_LT(metrics.latency, 5.0f); // Maximal 5ms Plugin-Latenz
}

// Effekt-Kette Test
TEST_F(VRDAWAudioTest, EffectChain) {
    // Erstelle Effekt-Kette
    auto chainId = daw->createEffectChain();
    
    // Füge Effekte hinzu
    auto eqId = daw->addEffect(chainId, "EQ");
    auto compId = daw->addEffect(chainId, "Compressor");
    auto reverbId = daw->addEffect(chainId, "Reverb");
    
    // Konfiguriere Effekte
    daw->setEffectParameter(eqId, "low", 0.5f);
    daw->setEffectParameter(compId, "threshold", -20.0f);
    daw->setEffectParameter(reverbId, "wet", 0.3f);
    
    // Verarbeite Audio
    const int BLOCK_COUNT = 100;
    auto signal = daw->generateTestSignal(512);
    
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        daw->processAudioWithEffectChain(chainId, signal);
    }
    
    // Überprüfe Effekt-Kette
    auto metrics = daw->getEffectChainMetrics(chainId);
    EXPECT_LT(metrics.totalLatency, 20.0f); // Maximal 20ms Gesamtlatenz
    EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
    EXPECT_EQ(metrics.activeEffects, 3); // Alle Effekte aktiv
}

// Audio-Qualität Test
TEST_F(VRDAWAudioTest, AudioQuality) {
    // Generiere Test-Signal
    auto signal = daw->generateTestSignal(1024);
    
    // Verarbeite Signal
    daw->processAudioBlock(signal);
    
    // Überprüfe Audio-Qualität
    auto quality = daw->analyzeAudioQuality(signal);
    EXPECT_GT(quality.snr, 90.0f); // Mindestens 90dB SNR
    EXPECT_GT(quality.thd, -60.0f); // Maximal -60dB THD
    EXPECT_GT(quality.frequencyResponse, 0.9f); // Flache Frequenzgang
}

// Audio-Fehlerbehandlung Test
TEST_F(VRDAWAudioTest, AudioErrorHandling) {
    // Teste ungültige Buffer-Größe
    auto invalidSignal = daw->generateTestSignal(0);
    EXPECT_FALSE(daw->processAudioBlock(invalidSignal));
    
    // Teste ungültiges Plugin
    auto invalidPlugin = daw->loadPlugin("invalid.vst3");
    EXPECT_FALSE(daw->isPluginLoaded(invalidPlugin));
    
    // Teste ungültige Effekt-Kette
    auto invalidChain = daw->createEffectChain();
    daw->removeEffectChain(invalidChain);
    EXPECT_FALSE(daw->isEffectChainValid(invalidChain));
    
    // Überprüfe Fehlerwiederherstellung
    EXPECT_TRUE(daw->isAudioInitialized());
    EXPECT_TRUE(daw->isAudioEngineActive());
}

} // namespace Tests
} // namespace VR_DAW 