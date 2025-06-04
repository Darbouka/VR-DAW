#include <gtest/gtest.h>
#include <chrono>
#include "../src/audio/AudioEngine.hpp"

namespace VR_DAW {
namespace Tests {

class AudioEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<AudioEngine>();
        engine->initialize();
    }
    
    void TearDown() override {
        engine->shutdown();
    }
    
    std::unique_ptr<AudioEngine> engine;
};

// Plugin-Verarbeitung Tests
TEST_F(AudioEngineTest, PluginLoading) {
    // Teste Plugin-Laden
    EXPECT_TRUE(engine->loadPlugin("test_plugin.vst3"));
    EXPECT_TRUE(engine->isPluginLoaded("test_plugin.vst3"));
    
    // Teste Plugin-Entladen
    engine->unloadPlugin("test_plugin.vst3");
    EXPECT_FALSE(engine->isPluginLoaded("test_plugin.vst3"));
}

TEST_F(AudioEngineTest, PluginProcessing) {
    // Lade mehrere Plugins
    for (int i = 0; i < 10; ++i) {
        engine->loadPlugin("test_plugin_" + std::to_string(i) + ".vst3");
    }
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    // Verarbeite Audio
    engine->processBlock(buffer, midiMessages);
    
    // Überprüfe Buffer
    EXPECT_EQ(buffer.getNumChannels(), 2);
    EXPECT_EQ(buffer.getNumSamples(), 512);
}

// Performance Tests
TEST_F(AudioEngineTest, BufferSizePerformance) {
    std::vector<int> bufferSizes = {64, 128, 256, 512, 1024, 2048};
    
    for (int size : bufferSizes) {
        engine->setBufferSize(size);
        
        juce::AudioBuffer<float> buffer(2, size);
        juce::MidiBuffer midiMessages;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Verarbeite 1000 Blöcke
        for (int i = 0; i < 1000; ++i) {
            engine->processBlock(buffer, midiMessages);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Überprüfe Performance
        EXPECT_LT(duration.count(), 1000) << "Buffer size " << size << " zu langsam";
    }
}

TEST_F(AudioEngineTest, SampleRatePerformance) {
    std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0};
    
    for (double rate : sampleRates) {
        engine->setSampleRate(rate);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiMessages;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Verarbeite 1000 Blöcke
        for (int i = 0; i < 1000; ++i) {
            engine->processBlock(buffer, midiMessages);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Überprüfe Performance
        EXPECT_LT(duration.count(), 1000) << "Sample rate " << rate << " zu langsam";
    }
}

// DSP Tests
TEST_F(AudioEngineTest, DSPOperations) {
    // Teste verschiedene DSP-Operationen
    engine->enableSIMDOptimization(true);
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    // Fülle Buffer mit Test-Signal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            buffer.setSample(channel, sample, 0.5f * sin(2.0f * M_PI * 440.0f * sample / 44100.0f));
        }
    }
    
    // Verarbeite Audio
    engine->processBlock(buffer, midiMessages);
    
    // Überprüfe DSP-Ergebnisse
    float maxSample = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            maxSample = std::max(maxSample, std::abs(buffer.getSample(channel, sample)));
        }
    }
    EXPECT_LE(maxSample, 1.0f);
}

// Speichermanagement Tests
TEST_F(AudioEngineTest, MemoryManagement) {
    // Teste Speichermanagement
    engine->optimizeMemoryUsage();
    
    // Lade viele Plugins
    for (int i = 0; i < 50; ++i) {
        engine->loadPlugin("test_plugin_" + std::to_string(i) + ".vst3");
    }
    
    // Überprüfe Speichernutzung
    auto metrics = engine->getPerformanceMetrics();
    EXPECT_LT(metrics.memoryUsage, 1024); // Maximal 1GB Speichernutzung
}

// Multithreading Tests
TEST_F(AudioEngineTest, Multithreading) {
    std::vector<int> threadCounts = {1, 2, 4, 8};
    
    for (int count : threadCounts) {
        engine->setThreadCount(count);
        
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiMessages;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Verarbeite 1000 Blöcke
        for (int i = 0; i < 1000; ++i) {
            engine->processBlock(buffer, midiMessages);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Überprüfe Performance
        EXPECT_LT(duration.count(), 1000) << "Thread count " << count << " zu langsam";
    }
}

// GPU-Beschleunigung Tests
TEST_F(AudioEngineTest, GPUAcceleration) {
    engine->enableGPUAcceleration(true);
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Verarbeite 1000 Blöcke
    for (int i = 0; i < 1000; ++i) {
        engine->processBlock(buffer, midiMessages);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 500); // Maximal 0.5 Sekunden mit GPU-Beschleunigung
}

// Plugin-Parallelverarbeitung Tests
TEST_F(AudioEngineTest, PluginParallelProcessing) {
    engine->enablePluginParallelProcessing(true);
    
    // Lade viele Plugins
    for (int i = 0; i < 20; ++i) {
        engine->loadPlugin("test_plugin_" + std::to_string(i) + ".vst3");
    }
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Verarbeite 1000 Blöcke
    for (int i = 0; i < 1000; ++i) {
        engine->processBlock(buffer, midiMessages);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde mit Parallelverarbeitung
}

// Fehlerbehandlung Tests
TEST_F(AudioEngineTest, ErrorHandling) {
    // Teste ungültige Plugin-Pfade
    EXPECT_FALSE(engine->loadPlugin("invalid_plugin.vst3"));
    
    // Teste ungültige Buffer-Größen
    EXPECT_FALSE(engine->setBufferSize(-1));
    EXPECT_FALSE(engine->setBufferSize(0));
    
    // Teste ungültige Sample-Rates
    EXPECT_FALSE(engine->setSampleRate(-1.0));
    EXPECT_FALSE(engine->setSampleRate(0.0));
    
    // Teste ungültige Thread-Anzahl
    EXPECT_FALSE(engine->setThreadCount(-1));
    EXPECT_FALSE(engine->setThreadCount(0));
}

} // namespace Tests
} // namespace VR_DAW 