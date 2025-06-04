#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWAITest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
        daw->initializeAI();
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
};

// KI-Mastering Test
TEST_F(VRDAWAITest, AIMastering) {
    // Lade Referenz-Track
    auto trackId = daw->createAudioTrack("Test Track");
    daw->loadAudioFile(trackId, "reference_track.wav");
    
    // Aktiviere KI-Mastering
    daw->enableAIMastering(true);
    
    // Führe Mastering durch
    auto result = daw->performAIMastering(trackId);
    EXPECT_TRUE(result.success);
    
    // Überprüfe Mastering-Parameter
    auto params = result.masteringParameters;
    EXPECT_GE(params.loudness, -14.0f); // LUFS Ziel
    EXPECT_LE(params.dynamicRange, 8.0f);
    EXPECT_GE(params.stereoWidth, 0.8f);
    
    // Überprüfe Audio-Qualität
    auto metrics = daw->getAudioQualityMetrics(trackId);
    EXPECT_GT(metrics.clarity, 0.8f);
    EXPECT_GT(metrics.balance, 0.8f);
    EXPECT_GT(metrics.warmth, 0.7f);
}

// KI-Mixing Test
TEST_F(VRDAWAITest, AIMixing) {
    // Erstelle Multi-Track Projekt
    std::vector<std::string> trackIds;
    for (int i = 0; i < 8; ++i) {
        auto trackId = daw->createAudioTrack("Track " + std::to_string(i));
        daw->loadAudioFile(trackId, "track_" + std::to_string(i) + ".wav");
        trackIds.push_back(trackId);
    }
    
    // Aktiviere KI-Mixing
    daw->enableAIMixing(true);
    
    // Führe Mixing durch
    auto result = daw->performAIMixing(trackIds);
    EXPECT_TRUE(result.success);
    
    // Überprüfe Mix-Parameter
    for (const auto& trackId : trackIds) {
        auto mixParams = daw->getTrackMixParameters(trackId);
        EXPECT_GE(mixParams.volume, -60.0f);
        EXPECT_LE(mixParams.volume, 0.0f);
        EXPECT_GE(mixParams.pan, -1.0f);
        EXPECT_LE(mixParams.pan, 1.0f);
    }
    
    // Überprüfe Mix-Balance
    auto balanceMetrics = daw->getMixBalanceMetrics();
    EXPECT_GT(balanceMetrics.frequencyBalance, 0.8f);
    EXPECT_GT(balanceMetrics.stereoBalance, 0.8f);
    EXPECT_GT(balanceMetrics.dynamicBalance, 0.8f);
}

// KI-Lernprozess Test
TEST_F(VRDAWAITest, AILearning) {
    // Lade Trainingsdaten
    std::vector<std::string> trainingFiles = {
        "mastered_track1.wav",
        "mastered_track2.wav",
        "mastered_track3.wav"
    };
    
    // Trainiere KI
    auto trainingResult = daw->trainAI(trainingFiles);
    EXPECT_TRUE(trainingResult.success);
    
    // Überprüfe Trainingsergebnisse
    EXPECT_GT(trainingResult.accuracy, 0.9f);
    EXPECT_LT(trainingResult.loss, 0.1f);
    
    // Teste KI auf neuem Track
    auto trackId = daw->createAudioTrack("Test Track");
    daw->loadAudioFile(trackId, "new_track.wav");
    
    auto predictionResult = daw->predictAIParameters(trackId);
    EXPECT_TRUE(predictionResult.success);
    EXPECT_GT(predictionResult.confidence, 0.8f);
}

// KI-Performance Test
TEST_F(VRDAWAITest, AIPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int processedTracks = 0;
    int failedPredictions = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        auto trackId = daw->createAudioTrack("Test Track");
        daw->loadAudioFile(trackId, "test_track.wav");
        
        auto result = daw->performAIMastering(trackId);
        processedTracks++;
        
        if (!result.success) {
            failedPredictions++;
        }
        
        // Überprüfe KI-Performance
        auto metrics = daw->getAIPerformanceMetrics();
        EXPECT_LT(metrics.inferenceTime, 0.1f); // Maximal 100ms pro Inferenz
        EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB für KI
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedPredictions / processedTracks;
    EXPECT_GT(successRate, 0.95f); // Mindestens 95% Erfolgsrate
}

// KI-Fehlerbehandlung Test
TEST_F(VRDAWAITest, AIErrorHandling) {
    // Teste ungültige Eingaben
    auto result1 = daw->performAIMastering("invalid_track");
    EXPECT_FALSE(result1.success);
    
    auto result2 = daw->trainAI({});
    EXPECT_FALSE(result2.success);
    
    auto result3 = daw->predictAIParameters("invalid_track");
    EXPECT_FALSE(result3.success);
    
    // Überprüfe Fehlerwiederherstellung
    EXPECT_TRUE(daw->isAIInitialized());
    EXPECT_TRUE(daw->isAIModelLoaded());
}

} // namespace Tests
} // namespace VR_DAW 