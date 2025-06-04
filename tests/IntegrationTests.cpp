#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"
#include "../src/audio/AudioEngine.hpp"
#include "../src/vr/VRInterface.hpp"
#include "../src/ui/VRControlPanel.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWIntegrationTest : public ::testing::Test {
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

// Audio-VR Integration Tests
TEST_F(VRDAWIntegrationTest, AudioVRIntegration) {
    // Erstelle Audio-Track
    auto trackId = daw->createAudioTrack("Test Track");
    EXPECT_TRUE(trackId != "");
    
    // Füge Plugin hinzu
    auto pluginId = daw->addPluginToTrack(trackId, "test_plugin.vst3");
    EXPECT_TRUE(pluginId != "");
    
    // Erstelle VR-Control für Plugin
    auto controlId = daw->createPluginControl(pluginId, "Parameter 1");
    EXPECT_TRUE(controlId != "");
    
    // Teste Parameter-Update
    daw->updatePluginParameter(pluginId, "Parameter 1", 0.5f);
    auto value = daw->getPluginParameterValue(pluginId, "Parameter 1");
    EXPECT_FLOAT_EQ(value, 0.5f);
}

// Plugin-Management Tests
TEST_F(VRDAWIntegrationTest, PluginManagement) {
    // Scanne Plugin-Verzeichnis
    auto plugins = daw->scanPluginDirectory();
    EXPECT_FALSE(plugins.empty());
    
    // Lade Plugin
    auto pluginId = daw->loadPlugin(plugins[0]);
    EXPECT_TRUE(pluginId != "");
    
    // Überprüfe Plugin-Status
    EXPECT_TRUE(daw->isPluginLoaded(pluginId));
    EXPECT_TRUE(daw->isPluginActive(pluginId));
    
    // Deaktiviere Plugin
    daw->setPluginActive(pluginId, false);
    EXPECT_FALSE(daw->isPluginActive(pluginId));
}

// Tutorial-System Tests
TEST_F(VRDAWIntegrationTest, TutorialSystem) {
    // Starte Tutorial
    auto tutorialId = daw->startTutorial("Basic Navigation");
    EXPECT_TRUE(tutorialId != "");
    
    // Überprüfe Tutorial-Status
    EXPECT_TRUE(daw->isTutorialActive(tutorialId));
    EXPECT_EQ(daw->getCurrentTutorialStep(tutorialId), 1);
    
    // Navigiere durch Tutorial
    daw->nextTutorialStep(tutorialId);
    EXPECT_EQ(daw->getCurrentTutorialStep(tutorialId), 2);
    
    // Beende Tutorial
    daw->finishTutorial(tutorialId);
    EXPECT_FALSE(daw->isTutorialActive(tutorialId));
}

// Performance-Monitoring Tests
TEST_F(VRDAWIntegrationTest, PerformanceMonitoring) {
    // Starte Performance-Monitoring
    daw->startPerformanceMonitoring();
    
    // Führe einige Operationen aus
    for (int i = 0; i < 100; ++i) {
        daw->processAudio();
        daw->updateVR();
    }
    
    // Überprüfe Performance-Metriken
    auto metrics = daw->getPerformanceMetrics();
    EXPECT_LT(metrics.cpuUsage, 100.0f);
    EXPECT_LT(metrics.memoryUsage, 1024);
    EXPECT_LT(metrics.audioLatency, 0.1f);
    EXPECT_LT(metrics.vrLatency, 0.1f);
}

// Fehlerbehandlung Tests
TEST_F(VRDAWIntegrationTest, ErrorHandling) {
    // Teste ungültige Track-Operationen
    EXPECT_FALSE(daw->createAudioTrack("").isValid());
    EXPECT_FALSE(daw->addPluginToTrack("invalidId", "test_plugin.vst3").isValid());
    
    // Teste ungültige Plugin-Operationen
    EXPECT_FALSE(daw->loadPlugin("invalid_plugin.vst3").isValid());
    EXPECT_FALSE(daw->updatePluginParameter("invalidId", "Parameter 1", 0.5f));
    
    // Teste ungültige Tutorial-Operationen
    EXPECT_FALSE(daw->startTutorial("NonExistentTutorial").isValid());
    EXPECT_FALSE(daw->nextTutorialStep("invalidId"));
}

// Speichermanagement Tests
TEST_F(VRDAWIntegrationTest, MemoryManagement) {
    // Erstelle viele Tracks und Plugins
    std::vector<std::string> trackIds;
    std::vector<std::string> pluginIds;
    
    for (int i = 0; i < 50; ++i) {
        auto trackId = daw->createAudioTrack("Track " + std::to_string(i));
        trackIds.push_back(trackId);
        
        auto pluginId = daw->addPluginToTrack(trackId, "test_plugin.vst3");
        pluginIds.push_back(pluginId);
    }
    
    // Überprüfe Speichernutzung
    auto metrics = daw->getPerformanceMetrics();
    EXPECT_LT(metrics.memoryUsage, 2048); // Maximal 2GB Speichernutzung
    
    // Bereinige Ressourcen
    for (const auto& pluginId : pluginIds) {
        daw->unloadPlugin(pluginId);
    }
    
    for (const auto& trackId : trackIds) {
        daw->deleteTrack(trackId);
    }
}

// Multithreading Tests
TEST_F(VRDAWIntegrationTest, Multithreading) {
    // Aktiviere Multithreading
    daw->setThreadCount(4);
    
    // Führe parallele Operationen aus
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([this, i]() {
            auto trackId = daw->createAudioTrack("Track " + std::to_string(i));
            daw->addPluginToTrack(trackId, "test_plugin.vst3");
            daw->processAudio();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Überprüfe Thread-Performance
    auto metrics = daw->getPerformanceMetrics();
    EXPECT_LT(metrics.cpuUsage, 100.0f);
}

} // namespace Tests
} // namespace VR_DAW 