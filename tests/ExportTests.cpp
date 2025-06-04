#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWExportTest : public ::testing::Test {
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

// Audio-Export Test
TEST_F(VRDAWExportTest, AudioExport) {
    auto projectId = daw->createProject("Export Test");
    
    // Erstelle Test-Tracks
    auto track1Id = daw->createTrack(projectId, "Track 1");
    auto track2Id = daw->createTrack(projectId, "Track 2");
    
    // Füge Audio hinzu
    daw->addAudioToTrack(track1Id, "test_audio1.wav");
    daw->addAudioToTrack(track2Id, "test_audio2.wav");
    
    // Konfiguriere Export
    auto exportConfig = daw->createExportConfig();
    exportConfig.format = "wav";
    exportConfig.sampleRate = 44100;
    exportConfig.bitDepth = 24;
    exportConfig.channels = 2;
    
    // Führe Export durch
    auto exportResult = daw->exportAudio(projectId, "exported_audio.wav", exportConfig);
    EXPECT_TRUE(exportResult.success);
    
    // Überprüfe Export-Datei
    auto fileInfo = daw->getAudioFileInfo("exported_audio.wav");
    EXPECT_EQ(fileInfo.sampleRate, 44100);
    EXPECT_EQ(fileInfo.bitDepth, 24);
    EXPECT_EQ(fileInfo.channels, 2);
    EXPECT_GT(fileInfo.duration, 0.0f);
}

// MIDI-Export Test
TEST_F(VRDAWExportTest, MIDIExport) {
    auto projectId = daw->createProject("MIDI Export Test");
    auto trackId = daw->createTrack(projectId, "MIDI Track");
    
    // Erstelle MIDI-Spur
    auto midiTrackId = daw->createMIDITrack(trackId);
    
    // Füge MIDI-Noten hinzu
    daw->addMIDINote(midiTrackId, 60, 0.0f, 1.0f, 100);
    daw->addMIDINote(midiTrackId, 64, 1.0f, 1.0f, 100);
    daw->addMIDINote(midiTrackId, 67, 2.0f, 1.0f, 100);
    
    // Führe MIDI-Export durch
    auto exportResult = daw->exportMIDI(projectId, "exported_midi.mid");
    EXPECT_TRUE(exportResult.success);
    
    // Überprüfe MIDI-Datei
    auto midiInfo = daw->getMIDIFileInfo("exported_midi.mid");
    EXPECT_EQ(midiInfo.trackCount, 1);
    EXPECT_EQ(midiInfo.noteCount, 3);
    EXPECT_EQ(midiInfo.format, 1);
}

// Projekt-Export Test
TEST_F(VRDAWExportTest, ProjectExport) {
    auto projectId = daw->createProject("Project Export Test");
    
    // Füge Projekt-Inhalte hinzu
    auto trackId = daw->createTrack(projectId, "Test Track");
    daw->addAudioToTrack(trackId, "test_audio.wav");
    
    auto pluginId = daw->addPlugin(projectId, "TestPlugin.vst3");
    daw->setPluginParameter(pluginId, "gain", 0.5f);
    
    // Führe Projekt-Export durch
    auto exportResult = daw->exportProject(projectId, "exported_project.vrdaw");
    EXPECT_TRUE(exportResult.success);
    
    // Überprüfe Projekt-Datei
    auto projectInfo = daw->getProjectFileInfo("exported_project.vrdaw");
    EXPECT_EQ(projectInfo.trackCount, 1);
    EXPECT_EQ(projectInfo.pluginCount, 1);
    EXPECT_TRUE(projectInfo.hasAudio);
}

// Export-Performance Test
TEST_F(VRDAWExportTest, ExportPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int exportCount = 0;
    int failedExports = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Erstelle Test-Projekt
        auto projectId = daw->createProject("Performance Test");
        auto trackId = daw->createTrack(projectId, "Test Track");
        daw->addAudioToTrack(trackId, "test_audio.wav");
        
        // Führe Export durch
        auto exportResult = daw->exportAudio(projectId, "export_" + std::to_string(exportCount) + ".wav");
        exportCount++;
        
        if (!exportResult.success) {
            failedExports++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getExportMetrics();
        EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
        EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB RAM
        EXPECT_LT(metrics.diskUsage, 100); // Maximal 100MB Festplatte
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedExports / exportCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Export-Fehlerbehandlung Test
TEST_F(VRDAWExportTest, ExportErrorHandling) {
    // Teste ungültige Export-Konfiguration
    auto projectId = daw->createProject("Error Test");
    auto exportConfig = daw->createExportConfig();
    exportConfig.sampleRate = 0; // Ungültige Sample-Rate
    
    auto exportResult = daw->exportAudio(projectId, "test.wav", exportConfig);
    EXPECT_FALSE(exportResult.success);
    
    // Teste ungültige Dateipfade
    exportResult = daw->exportAudio(projectId, "", exportConfig);
    EXPECT_FALSE(exportResult.success);
    
    // Teste ungültige Formate
    exportConfig.format = "invalid";
    exportResult = daw->exportAudio(projectId, "test.wav", exportConfig);
    EXPECT_FALSE(exportResult.success);
}

// Export-Integration Test
TEST_F(VRDAWExportTest, ExportIntegration) {
    auto projectId = daw->createProject("Integration Test");
    
    // Erstelle komplexes Projekt
    auto track1Id = daw->createTrack(projectId, "Audio Track");
    daw->addAudioToTrack(track1Id, "test_audio.wav");
    
    auto track2Id = daw->createTrack(projectId, "MIDI Track");
    auto midiTrackId = daw->createMIDITrack(track2Id);
    daw->addMIDINote(midiTrackId, 60, 0.0f, 1.0f, 100);
    
    auto pluginId = daw->addPlugin(projectId, "TestPlugin.vst3");
    daw->setPluginParameter(pluginId, "gain", 0.5f);
    
    // Führe verschiedene Exports durch
    auto audioExport = daw->exportAudio(projectId, "exported_audio.wav");
    EXPECT_TRUE(audioExport.success);
    
    auto midiExport = daw->exportMIDI(projectId, "exported_midi.mid");
    EXPECT_TRUE(midiExport.success);
    
    auto projectExport = daw->exportProject(projectId, "exported_project.vrdaw");
    EXPECT_TRUE(projectExport.success);
    
    // Überprüfe Export-Integration
    auto exportInfo = daw->getExportInfo(projectId);
    EXPECT_EQ(exportInfo.audioExportCount, 1);
    EXPECT_EQ(exportInfo.midiExportCount, 1);
    EXPECT_EQ(exportInfo.projectExportCount, 1);
}

} // namespace Tests
} // namespace VR_DAW 