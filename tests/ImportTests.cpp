#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWImportTest : public ::testing::Test {
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

// Audio-Import Test
TEST_F(VRDAWImportTest, AudioImport) {
    auto projectId = daw->createProject("Import Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Importiere Audio
    auto importResult = daw->importAudio(trackId, "test_audio.wav");
    EXPECT_TRUE(importResult.success);
    
    // Überprüfe importierte Audio
    auto audioInfo = daw->getTrackAudioInfo(trackId);
    EXPECT_EQ(audioInfo.sampleRate, 44100);
    EXPECT_EQ(audioInfo.bitDepth, 24);
    EXPECT_EQ(audioInfo.channels, 2);
    EXPECT_GT(audioInfo.duration, 0.0f);
}

// MIDI-Import Test
TEST_F(VRDAWImportTest, MIDIImport) {
    auto projectId = daw->createProject("MIDI Import Test");
    auto trackId = daw->createTrack(projectId, "MIDI Track");
    
    // Importiere MIDI
    auto importResult = daw->importMIDI(trackId, "test_midi.mid");
    EXPECT_TRUE(importResult.success);
    
    // Überprüfe importierte MIDI
    auto midiInfo = daw->getTrackMIDIInfo(trackId);
    EXPECT_EQ(midiInfo.trackCount, 1);
    EXPECT_GT(midiInfo.noteCount, 0);
    EXPECT_EQ(midiInfo.format, 1);
}

// Projekt-Import Test
TEST_F(VRDAWImportTest, ProjectImport) {
    // Importiere Projekt
    auto importResult = daw->importProject("test_project.vrdaw");
    EXPECT_TRUE(importResult.success);
    
    // Überprüfe importiertes Projekt
    auto projectId = importResult.projectId;
    auto projectInfo = daw->getProjectInfo(projectId);
    EXPECT_GT(projectInfo.trackCount, 0);
    EXPECT_GT(projectInfo.pluginCount, 0);
    EXPECT_TRUE(projectInfo.hasAudio);
}

// Import-Performance Test
TEST_F(VRDAWImportTest, ImportPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int importCount = 0;
    int failedImports = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Erstelle Test-Projekt
        auto projectId = daw->createProject("Performance Test");
        auto trackId = daw->createTrack(projectId, "Test Track");
        
        // Führe Import durch
        auto importResult = daw->importAudio(trackId, "test_audio.wav");
        importCount++;
        
        if (!importResult.success) {
            failedImports++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getImportMetrics();
        EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
        EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB RAM
        EXPECT_LT(metrics.diskUsage, 100); // Maximal 100MB Festplatte
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedImports / importCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Import-Fehlerbehandlung Test
TEST_F(VRDAWImportTest, ImportErrorHandling) {
    auto projectId = daw->createProject("Error Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Teste ungültige Dateipfade
    auto importResult = daw->importAudio(trackId, "");
    EXPECT_FALSE(importResult.success);
    
    // Teste nicht existierende Dateien
    importResult = daw->importAudio(trackId, "nonexistent.wav");
    EXPECT_FALSE(importResult.success);
    
    // Teste ungültige Formate
    importResult = daw->importAudio(trackId, "test.txt");
    EXPECT_FALSE(importResult.success);
}

// Import-Integration Test
TEST_F(VRDAWImportTest, ImportIntegration) {
    auto projectId = daw->createProject("Integration Test");
    
    // Importiere verschiedene Dateitypen
    auto track1Id = daw->createTrack(projectId, "Audio Track");
    auto audioImport = daw->importAudio(track1Id, "test_audio.wav");
    EXPECT_TRUE(audioImport.success);
    
    auto track2Id = daw->createTrack(projectId, "MIDI Track");
    auto midiImport = daw->importMIDI(track2Id, "test_midi.mid");
    EXPECT_TRUE(midiImport.success);
    
    // Überprüfe Import-Integration
    auto importInfo = daw->getImportInfo(projectId);
    EXPECT_EQ(importInfo.audioImportCount, 1);
    EXPECT_EQ(importInfo.midiImportCount, 1);
}

// Import-Konvertierung Test
TEST_F(VRDAWImportTest, ImportConversion) {
    auto projectId = daw->createProject("Conversion Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Importiere mit Konvertierung
    auto importConfig = daw->createImportConfig();
    importConfig.targetSampleRate = 48000;
    importConfig.targetBitDepth = 32;
    importConfig.targetChannels = 2;
    
    auto importResult = daw->importAudioWithConversion(trackId, "test_audio.wav", importConfig);
    EXPECT_TRUE(importResult.success);
    
    // Überprüfe konvertierte Audio
    auto audioInfo = daw->getTrackAudioInfo(trackId);
    EXPECT_EQ(audioInfo.sampleRate, 48000);
    EXPECT_EQ(audioInfo.bitDepth, 32);
    EXPECT_EQ(audioInfo.channels, 2);
}

// Import-Batch Test
TEST_F(VRDAWImportTest, ImportBatch) {
    auto projectId = daw->createProject("Batch Test");
    
    // Erstelle Batch-Import
    std::vector<std::string> files = {
        "test_audio1.wav",
        "test_audio2.wav",
        "test_audio3.wav"
    };
    
    auto batchResult = daw->importAudioBatch(projectId, files);
    EXPECT_TRUE(batchResult.success);
    
    // Überprüfe Batch-Import
    auto projectInfo = daw->getProjectInfo(projectId);
    EXPECT_EQ(projectInfo.trackCount, files.size());
    
    // Überprüfe importierte Dateien
    for (const auto& file : files) {
        auto trackId = daw->findTrackByAudioFile(projectId, file);
        EXPECT_TRUE(daw->isTrackValid(trackId));
    }
}

} // namespace Tests
} // namespace VR_DAW 