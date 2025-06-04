#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWProjectTest : public ::testing::Test {
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

// Projekt-Erstellung Test
TEST_F(VRDAWProjectTest, ProjectCreation) {
    // Erstelle Test-Projekt
    auto projectId = daw->createProject("Test Project");
    EXPECT_TRUE(daw->isProjectValid(projectId));
    
    // Überprüfe Projekt-Informationen
    auto projectInfo = daw->getProjectInfo(projectId);
    EXPECT_EQ(projectInfo.name, "Test Project");
    EXPECT_EQ(projectInfo.trackCount, 0);
    EXPECT_EQ(projectInfo.pluginCount, 0);
    EXPECT_EQ(projectInfo.effectCount, 0);
}

// Track-Management Test
TEST_F(VRDAWProjectTest, TrackManagement) {
    auto projectId = daw->createProject("Track Test");
    
    // Erstelle Tracks
    std::vector<std::string> trackIds;
    for (int i = 0; i < 10; ++i) {
        auto trackId = daw->createTrack(projectId, "Track " + std::to_string(i));
        trackIds.push_back(trackId);
    }
    
    // Überprüfe Tracks
    auto projectInfo = daw->getProjectInfo(projectId);
    EXPECT_EQ(projectInfo.trackCount, trackIds.size());
    
    // Teste Track-Operationen
    for (const auto& trackId : trackIds) {
        // Füge Audio hinzu
        daw->addAudioToTrack(trackId, "test_audio.wav");
        
        // Füge Plugin hinzu
        daw->addPluginToTrack(trackId, "TestPlugin.vst3");
        
        // Setze Track-Parameter
        daw->setTrackParameter(trackId, "volume", 0.8f);
        daw->setTrackParameter(trackId, "pan", 0.0f);
    }
}

// Plugin-Management Test
TEST_F(VRDAWProjectTest, PluginManagement) {
    auto projectId = daw->createProject("Plugin Test");
    
    // Erstelle Plugin-Instanzen
    std::vector<std::string> pluginIds;
    for (int i = 0; i < 5; ++i) {
        auto pluginId = daw->addPlugin(projectId, "TestPlugin.vst3");
        pluginIds.push_back(pluginId);
    }
    
    // Überprüfe Plugins
    auto projectInfo = daw->getProjectInfo(projectId);
    EXPECT_EQ(projectInfo.pluginCount, pluginIds.size());
    
    // Teste Plugin-Operationen
    for (const auto& pluginId : pluginIds) {
        // Konfiguriere Plugin
        daw->setPluginParameter(pluginId, "gain", 0.5f);
        daw->setPluginParameter(pluginId, "pan", 0.0f);
        
        // Aktiviere/Deaktiviere Plugin
        daw->setPluginEnabled(pluginId, true);
        EXPECT_TRUE(daw->isPluginEnabled(pluginId));
        
        daw->setPluginEnabled(pluginId, false);
        EXPECT_FALSE(daw->isPluginEnabled(pluginId));
    }
}

// Projekt-Speicherung Test
TEST_F(VRDAWProjectTest, ProjectSaving) {
    auto projectId = daw->createProject("Save Test");
    
    // Füge Projekt-Inhalte hinzu
    auto trackId = daw->createTrack(projectId, "Test Track");
    daw->addAudioToTrack(trackId, "test_audio.wav");
    
    auto pluginId = daw->addPlugin(projectId, "TestPlugin.vst3");
    daw->setPluginParameter(pluginId, "gain", 0.5f);
    
    // Speichere Projekt
    auto saveResult = daw->saveProject(projectId, "test_project.vrdaw");
    EXPECT_TRUE(saveResult.success);
    
    // Lade Projekt
    auto loadResult = daw->loadProject("test_project.vrdaw");
    EXPECT_TRUE(loadResult.success);
    
    // Überprüfe geladenes Projekt
    auto loadedProjectId = loadResult.projectId;
    auto projectInfo = daw->getProjectInfo(loadedProjectId);
    EXPECT_EQ(projectInfo.trackCount, 1);
    EXPECT_EQ(projectInfo.pluginCount, 1);
}

// Projekt-Performance Test
TEST_F(VRDAWProjectTest, ProjectPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int operationCount = 0;
    int failedOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe Projekt-Operationen aus
        auto projectId = daw->createProject("Performance Test");
        auto trackId = daw->createTrack(projectId, "Test Track");
        auto pluginId = daw->addPlugin(projectId, "TestPlugin.vst3");
        
        operationCount++;
        
        if (!daw->isProjectValid(projectId) ||
            !daw->isTrackValid(trackId) ||
            !daw->isPluginValid(pluginId)) {
            failedOperations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getProjectMetrics(projectId);
        EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
        EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB RAM
        EXPECT_LT(metrics.diskUsage, 100); // Maximal 100MB Festplatte
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedOperations / operationCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Projekt-Fehlerbehandlung Test
TEST_F(VRDAWProjectTest, ProjectErrorHandling) {
    // Teste ungültige Projekt-Operationen
    auto invalidProject = daw->createProject("");
    EXPECT_FALSE(daw->isProjectValid(invalidProject));
    
    auto invalidTrack = daw->createTrack(invalidProject, "Test");
    EXPECT_FALSE(daw->isTrackValid(invalidTrack));
    
    auto invalidPlugin = daw->addPlugin(invalidProject, "invalid.vst3");
    EXPECT_FALSE(daw->isPluginValid(invalidPlugin));
    
    // Teste Projekt-Speicherung
    auto saveResult = daw->saveProject(invalidProject, "");
    EXPECT_FALSE(saveResult.success);
    
    // Teste Projekt-Laden
    auto loadResult = daw->loadProject("invalid.vrdaw");
    EXPECT_FALSE(loadResult.success);
}

} // namespace Tests
} // namespace VR_DAW 