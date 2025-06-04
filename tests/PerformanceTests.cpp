#include <gtest/gtest.h>
#include <chrono>
#include "../src/audio/AudioEngine.hpp"
#include "../src/audio/DynamicsProcessor.hpp"
#include "../src/vr/VRInterface.hpp"
#include "../src/ui/VRControlPanel.hpp"
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

// Audio-Engine Performance Tests
TEST(AudioEnginePerformanceTest, PluginProcessing) {
    AudioEngine engine;
    EXPECT_TRUE(engine.initialize());
    
    // Lade mehrere Plugins
    for (int i = 0; i < 10; ++i) {
        engine.loadPlugin("test_plugin_" + std::to_string(i));
    }
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    // Messung der Verarbeitungszeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Verarbeite 1000 Blöcke
    for (int i = 0; i < 1000; ++i) {
        engine.processBlock(buffer, midiMessages);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 1000 Blöcke
}

TEST(AudioEnginePerformanceTest, Multithreading) {
    AudioEngine engine;
    EXPECT_TRUE(engine.initialize());
    
    // Aktiviere Multithreading
    engine.setThreadCount(4);
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    // Messung der Verarbeitungszeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Verarbeite 1000 Blöcke
    for (int i = 0; i < 1000; ++i) {
        engine.processBlock(buffer, midiMessages);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance-Verbesserung
    EXPECT_LT(duration.count(), 500); // Maximal 0.5 Sekunden für 1000 Blöcke mit Multithreading
}

// Dynamics Processor Performance Tests
TEST(DynamicsProcessorPerformanceTest, CompressorChain) {
    DynamicsProcessor processor;
    
    // Erstelle Kompressor-Kette
    for (int i = 0; i < 5; ++i) {
        CompressorParameters params;
        params.threshold = -20.0f;
        params.ratio = 4.0f;
        params.attack = 10.0f;
        params.release = 100.0f;
        processor.setCompressorParameters(params);
    }
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    
    // Füge Test-Signal hinzu
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            buffer.setSample(channel, sample, 0.5f * sin(2.0f * M_PI * 440.0f * sample / 44100.0f));
        }
    }
    
    // Messung der Verarbeitungszeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Verarbeite 1000 Blöcke
    for (int i = 0; i < 1000; ++i) {
        processor.processBlock(buffer);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 500); // Maximal 0.5 Sekunden für 1000 Blöcke
}

// VR Interface Performance Tests
TEST(VRInterfacePerformanceTest, Rendering) {
    VRInterface vr;
    EXPECT_TRUE(vr.initialize());
    
    // Erstelle Test-Szene
    auto roomId = vr.createRoom("Test Room", glm::vec3(0.0f), glm::vec3(5.0f, 3.0f, 5.0f));
    
    for (int i = 0; i < 10; ++i) {
        vr.createWindow("Test Window " + std::to_string(i),
            glm::vec3(i * 0.5f, 0.0f, -2.0f),
            glm::vec2(1.0f, 1.0f));
    }
    
    // Messung der Rendering-Zeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Rendere 100 Frames
    for (int i = 0; i < 100; ++i) {
        vr.render();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 100 Frames
}

TEST(VRInterfacePerformanceTest, Physics) {
    VRInterface vr;
    EXPECT_TRUE(vr.initialize());
    
    // Erstelle Test-Objekte
    for (int i = 0; i < 100; ++i) {
        vr.createWindow("Test Window " + std::to_string(i),
            glm::vec3(i * 0.1f, 0.0f, -2.0f),
            glm::vec2(0.2f, 0.2f));
    }
    
    // Messung der Physik-Berechnungszeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Aktualisiere Physik 1000 mal
    for (int i = 0; i < 1000; ++i) {
        vr.updatePhysics(1.0f / 60.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 1000 Physik-Updates
}

// VR Control Panel Performance Tests
TEST(VRControlPanelPerformanceTest, ControlRendering) {
    VRControlPanel panel;
    
    // Erstelle viele Steuerelemente
    for (int i = 0; i < 100; ++i) {
        panel.addControl("Test Control " + std::to_string(i),
            ControlType::Button,
            glm::vec3(i * 0.1f, 0.0f, -2.0f),
            glm::vec2(0.1f, 0.1f));
    }
    
    // Messung der Rendering-Zeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Rendere 100 Frames
    for (int i = 0; i < 100; ++i) {
        panel.render();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 500); // Maximal 0.5 Sekunden für 100 Frames
}

TEST(VRControlPanelPerformanceTest, InteractionHandling) {
    VRControlPanel panel;
    
    // Erstelle viele Steuerelemente mit Callbacks
    for (int i = 0; i < 100; ++i) {
        auto controlId = panel.addControl("Test Control " + std::to_string(i),
            ControlType::Button,
            glm::vec3(i * 0.1f, 0.0f, -2.0f),
            glm::vec2(0.1f, 0.1f));
            
        panel.setControlCallback(controlId, []() {});
    }
    
    // Messung der Interaktions-Verarbeitungszeit
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simuliere 1000 Interaktionen
    for (int i = 0; i < 1000; ++i) {
        panel.handleInteraction("Test Control " + std::to_string(i % 100),
            InteractionType::Click);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 100); // Maximal 0.1 Sekunden für 1000 Interaktionen
}

class VRDAWPerformanceTest : public ::testing::Test {
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

// Audio-Performance Test
TEST_F(VRDAWPerformanceTest, AudioPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int blockCount = 0;
    int underruns = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Verarbeite Audio
        auto audioResult = daw->processAudioBlock();
        blockCount++;
        
        if (audioResult.hasUnderrun) {
            underruns++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getAudioMetrics();
        EXPECT_LT(metrics.cpuUsage, 50.0f); // Maximal 50% CPU
        EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB RAM
        EXPECT_LT(metrics.latency, 10.0f); // Maximal 10ms Latenz
    }
    
    // Überprüfe Gesamtperformance
    float underrunRate = (float)underruns / blockCount;
    EXPECT_LT(underrunRate, 0.01f); // Maximal 1% Underruns
}

// MIDI-Performance Test
TEST_F(VRDAWPerformanceTest, MIDIPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int eventCount = 0;
    int droppedEvents = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Verarbeite MIDI
        auto midiResult = daw->processMIDIEvents();
        eventCount++;
        
        if (midiResult.hasDroppedEvents) {
            droppedEvents++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getMIDIMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 256); // Maximal 256MB RAM
        EXPECT_LT(metrics.latency, 5.0f); // Maximal 5ms Latenz
    }
    
    // Überprüfe Gesamtperformance
    float dropRate = (float)droppedEvents / eventCount;
    EXPECT_LT(dropRate, 0.01f); // Maximal 1% Dropped Events
}

// Plugin-Performance Test
TEST_F(VRDAWPerformanceTest, PluginPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int pluginCount = 0;
    int failedPlugins = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Verarbeite Plugins
        auto pluginResult = daw->processPlugins();
        pluginCount++;
        
        if (pluginResult.hasFailedPlugins) {
            failedPlugins++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getPluginMetrics();
        EXPECT_LT(metrics.cpuUsage, 30.0f); // Maximal 30% CPU
        EXPECT_LT(metrics.memoryUsage, 512); // Maximal 512MB RAM
        EXPECT_LT(metrics.processingTime, 20.0f); // Maximal 20ms Verarbeitungszeit
    }
    
    // Überprüfe Gesamtperformance
    float failureRate = (float)failedPlugins / pluginCount;
    EXPECT_LT(failureRate, 0.01f); // Maximal 1% Fehlerrate
}

// VR-Performance Test
TEST_F(VRDAWPerformanceTest, VRPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int frameCount = 0;
    int droppedFrames = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Rendere VR-Frame
        auto frameResult = daw->renderVRFrame();
        frameCount++;
        
        if (frameResult.hasDroppedFrame) {
            droppedFrames++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getVRMetrics();
        EXPECT_LT(metrics.cpuUsage, 40.0f); // Maximal 40% CPU
        EXPECT_LT(metrics.gpuUsage, 80.0f); // Maximal 80% GPU
        EXPECT_LT(metrics.memoryUsage, 2048); // Maximal 2GB RAM
        EXPECT_LT(metrics.frameTime, 11.0f); // Maximal 11ms pro Frame (90 FPS)
    }
    
    // Überprüfe Gesamtperformance
    float frameRate = (float)frameCount / DURATION_SECONDS;
    EXPECT_GT(frameRate, 89.0f); // Mindestens 90 FPS
    
    float dropRate = (float)droppedFrames / frameCount;
    EXPECT_LT(dropRate, 0.01f); // Maximal 1% Dropped Frames
}

// Netzwerk-Performance Test
TEST_F(VRDAWPerformanceTest, NetworkPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int packetCount = 0;
    int lostPackets = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Verarbeite Netzwerk
        auto networkResult = daw->processNetwork();
        packetCount++;
        
        if (networkResult.hasLostPackets) {
            lostPackets++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getNetworkMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 256); // Maximal 256MB RAM
        EXPECT_LT(metrics.latency, 100.0f); // Maximal 100ms Latenz
    }
    
    // Überprüfe Gesamtperformance
    float lossRate = (float)lostPackets / packetCount;
    EXPECT_LT(lossRate, 0.01f); // Maximal 1% Paketverlust
}

// Speicher-Performance Test
TEST_F(VRDAWPerformanceTest, MemoryPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int allocationCount = 0;
    int failedAllocations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe Speicher-Operationen durch
        auto memoryResult = daw->performMemoryOperation();
        allocationCount++;
        
        if (memoryResult.hasFailedAllocation) {
            failedAllocations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getMemoryMetrics();
        EXPECT_LT(metrics.heapUsage, 4096); // Maximal 4GB Heap
        EXPECT_LT(metrics.stackUsage, 1024); // Maximal 1GB Stack
        EXPECT_LT(metrics.fragmentation, 0.1f); // Maximal 10% Fragmentierung
    }
    
    // Überprüfe Gesamtperformance
    float failureRate = (float)failedAllocations / allocationCount;
    EXPECT_LT(failureRate, 0.01f); // Maximal 1% Fehlerrate
}

// CPU-Performance Test
TEST_F(VRDAWPerformanceTest, CPUPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int operationCount = 0;
    int failedOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe CPU-Operationen durch
        auto cpuResult = daw->performCPUOperation();
        operationCount++;
        
        if (cpuResult.hasFailedOperation) {
            failedOperations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getCPUMetrics();
        EXPECT_LT(metrics.usage, 80.0f); // Maximal 80% CPU
        EXPECT_LT(metrics.temperature, 80.0f); // Maximal 80°C
        EXPECT_LT(metrics.powerUsage, 100.0f); // Maximal 100W
    }
    
    // Überprüfe Gesamtperformance
    float failureRate = (float)failedOperations / operationCount;
    EXPECT_LT(failureRate, 0.01f); // Maximal 1% Fehlerrate
}

// GPU-Performance Test
TEST_F(VRDAWPerformanceTest, GPUPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int operationCount = 0;
    int failedOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Führe GPU-Operationen durch
        auto gpuResult = daw->performGPUOperation();
        operationCount++;
        
        if (gpuResult.hasFailedOperation) {
            failedOperations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getGPUMetrics();
        EXPECT_LT(metrics.usage, 90.0f); // Maximal 90% GPU
        EXPECT_LT(metrics.temperature, 85.0f); // Maximal 85°C
        EXPECT_LT(metrics.powerUsage, 200.0f); // Maximal 200W
    }
    
    // Überprüfe Gesamtperformance
    float failureRate = (float)failedOperations / operationCount;
    EXPECT_LT(failureRate, 0.01f); // Maximal 1% Fehlerrate
}

} // namespace Tests
} // namespace VR_DAW 