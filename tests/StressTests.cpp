#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <random>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWStressTest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
        daw->setThreadCount(8); // Maximale Thread-Anzahl für Stresstests
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
    std::random_device rd;
    std::mt19937 gen{rd()};
};

// Extremes Plugin-Loading Test
TEST_F(VRDAWStressTest, ExtremePluginLoading) {
    const int MAX_PLUGINS = 100;
    std::vector<std::string> pluginIds;
    
    // Lade viele Plugins parallel
    std::vector<std::thread> threads;
    for (int i = 0; i < MAX_PLUGINS; ++i) {
        threads.emplace_back([this, &pluginIds, i]() {
            auto pluginId = daw->loadPlugin("test_plugin_" + std::to_string(i) + ".vst3");
            if (pluginId != "") {
                std::lock_guard<std::mutex> lock(mutex);
                pluginIds.push_back(pluginId);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Überprüfe Plugin-Stabilität
    for (const auto& pluginId : pluginIds) {
        EXPECT_TRUE(daw->isPluginLoaded(pluginId));
        EXPECT_TRUE(daw->isPluginActive(pluginId));
    }
    
    // Überprüfe Speichernutzung
    auto metrics = daw->getPerformanceMetrics();
    EXPECT_LT(metrics.memoryUsage, 4096); // Maximal 4GB für extreme Tests
}

// Kontinuierliche Audio-Verarbeitung Test
TEST_F(VRDAWStressTest, ContinuousAudioProcessing) {
    const int DURATION_SECONDS = 60;
    const int SAMPLE_RATE = 44100;
    const int BLOCK_SIZE = 512;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int processedBlocks = 0;
    int bufferUnderruns = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        daw->processAudio();
        processedBlocks++;
        
        auto metrics = daw->getPerformanceMetrics();
        if (metrics.bufferUnderruns > 0) {
            bufferUnderruns++;
        }
        
        // Überprüfe Performance-Grenzen
        EXPECT_LT(metrics.cpuUsage, 90.0f);
        EXPECT_LT(metrics.audioLatency, 0.05f);
    }
    
    // Überprüfe Verarbeitungsrate
    float processingRate = processedBlocks / (float)DURATION_SECONDS;
    EXPECT_GT(processingRate, SAMPLE_RATE / (float)BLOCK_SIZE * 0.95f);
    EXPECT_LT(bufferUnderruns, processedBlocks * 0.01f); // Maximal 1% Buffer Underruns
}

// VR-Rendering Stresstest
TEST_F(VRDAWStressTest, VRRenderingStress) {
    const int DURATION_SECONDS = 30;
    const int TARGET_FPS = 90;
    
    // Erstelle komplexe VR-Szene
    for (int i = 0; i < 100; ++i) {
        daw->createAudioTrack("Track " + std::to_string(i));
        daw->addPluginToTrack("test_plugin.vst3");
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int frames = 0;
    int droppedFrames = 0;
    auto lastFrameTime = start;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        daw->updateVR();
        frames++;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime);
        
        if (frameTime.count() > 1000 / TARGET_FPS) {
            droppedFrames++;
        }
        
        lastFrameTime = currentTime;
        
        // Überprüfe VR-Performance
        auto metrics = daw->getPerformanceMetrics();
        EXPECT_LT(metrics.vrLatency, 0.016f); // Maximal 16ms für 90 FPS
    }
    
    // Überprüfe Framerate
    float fps = frames / (float)DURATION_SECONDS;
    EXPECT_GT(fps, TARGET_FPS * 0.9f);
    EXPECT_LT(droppedFrames, frames * 0.1f); // Maximal 10% Dropped Frames
}

// Speicher-Fragmentierung Test
TEST_F(VRDAWStressTest, MemoryFragmentation) {
    const int ITERATIONS = 1000;
    std::vector<std::string> trackIds;
    std::vector<std::string> pluginIds;
    
    for (int i = 0; i < ITERATIONS; ++i) {
        // Erstelle und lösche Tracks/Plugins
        auto trackId = daw->createAudioTrack("Track " + std::to_string(i));
        trackIds.push_back(trackId);
        
        auto pluginId = daw->addPluginToTrack(trackId, "test_plugin.vst3");
        pluginIds.push_back(pluginId);
        
        if (i % 10 == 0) {
            // Lösche alte Ressourcen
            for (int j = 0; j < 5; ++j) {
                if (!trackIds.empty()) {
                    daw->deleteTrack(trackIds.back());
                    trackIds.pop_back();
                }
                if (!pluginIds.empty()) {
                    daw->unloadPlugin(pluginIds.back());
                    pluginIds.pop_back();
                }
            }
        }
        
        // Überprüfe Speicherstabilität
        auto metrics = daw->getPerformanceMetrics();
        EXPECT_LT(metrics.memoryUsage, 2048);
    }
}

// Netzwerk-Performance Test
TEST_F(VRDAWStressTest, NetworkPerformance) {
    const int DURATION_SECONDS = 30;
    const int MAX_LATENCY_MS = 100;
    
    daw->enableNetworkFeatures(true);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int networkOperations = 0;
    int highLatencyOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        daw->syncWithNetwork();
        networkOperations++;
        
        auto metrics = daw->getPerformanceMetrics();
        if (metrics.networkLatency > MAX_LATENCY_MS) {
            highLatencyOperations++;
        }
        
        // Überprüfe Netzwerk-Performance
        EXPECT_LT(metrics.networkLatency, MAX_LATENCY_MS);
    }
    
    // Überprüfe Netzwerk-Stabilität
    EXPECT_LT(highLatencyOperations, networkOperations * 0.05f); // Maximal 5% hohe Latenz
}

// Fehlerwiederherstellung Test
TEST_F(VRDAWStressTest, ErrorRecovery) {
    const int ERROR_ITERATIONS = 100;
    int recoveredErrors = 0;
    
    for (int i = 0; i < ERROR_ITERATIONS; ++i) {
        // Simuliere verschiedene Fehler
        bool errorRecovered = false;
        
        try {
            // Ungültige Operationen
            daw->loadPlugin("invalid_plugin.vst3");
            daw->createAudioTrack("");
            daw->updatePluginParameter("invalid", "param", 0.5f);
        } catch (...) {
            errorRecovered = daw->recoverFromError();
            if (errorRecovered) {
                recoveredErrors++;
            }
        }
        
        // Überprüfe System-Stabilität
        EXPECT_TRUE(daw->isInitialized());
        EXPECT_TRUE(daw->isAudioEngineRunning());
        EXPECT_TRUE(daw->isVRInterfaceActive());
    }
    
    // Überprüfe Fehlerwiederherstellung
    EXPECT_GT(recoveredErrors, ERROR_ITERATIONS * 0.9f); // Mindestens 90% Wiederherstellung
}

} // namespace Tests
} // namespace VR_DAW 