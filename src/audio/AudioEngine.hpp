#pragma once

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace VR_DAW {

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    
    template<class F>
    void enqueue(F&& f);
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

class GPUAccelerator {
public:
    GPUAccelerator();
    ~GPUAccelerator();

    void initialize();
    void shutdown();
    void processAudioBlock(juce::AudioBuffer<float>& buffer);
    void setBufferSize(int size);
    void setProcessingThreshold(float threshold);
    bool isAvailable() const;

private:
    // CUDA/OpenCL Kontext
    struct GPUContext {
        void* device;
        void* context;
        void* commandQueue;
        void* program;
    } gpuContext;

    // GPU-Buffer
    struct GPUBuffers {
        void* inputBuffer;
        void* outputBuffer;
        void* processingBuffer;
    } gpuBuffers;

    // Verarbeitungsparameter
    int bufferSize = 2048;
    float processingThreshold = 0.8f;
    bool isInitialized = false;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Audio-Engine-Status
    enum class EngineState {
        Stopped,
        Playing,
        Paused,
        Recording
    };

    // Performance-Metriken
    struct PerformanceMetrics {
        float cpuUsage;
        float memoryUsage;
        float diskIO;
        float networkLatency;
        int activePlugins;
        int bufferUnderruns;
    };

    // Plugin-Management
    struct PluginInfo {
        std::string name;
        std::string manufacturer;
        std::string category;
        bool isVST3;
        bool isAU;
        bool isAAX;
        std::vector<std::string> parameters;
    };

    // Neue Performance-Optimierungen
    struct PerformanceOptimizations {
        bool gpuAcceleration = false;
        bool simdEnabled = true;
        bool parallelProcessing = true;
        int threadPoolSize = std::thread::hardware_concurrency();
        int bufferSize = 512;
        double sampleRate = 44100.0;
        bool adaptiveBuffering = true;
        bool predictiveLoading = true;
        bool memoryOptimization = true;
    };

    // Hauptfunktionen
    void initialize();
    void shutdown();
    void start();
    void stop();
    void pause();
    void resume();
    void record();

    // Performance-Optimierung
    void setBufferSize(int size);
    void setSampleRate(double rate);
    void setThreadCount(int count);
    void enableGPUAcceleration(bool enable);
    void optimizeMemoryUsage();

    // Plugin-Management
    void loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& pluginId);
    void scanPluginDirectory(const std::string& directory);
    std::vector<PluginInfo> getAvailablePlugins() const;

    // Audio-Verarbeitung
    void processBlock(juce::AudioBuffer<float>& buffer);
    void processMIDI(const juce::MidiMessage& message);
    void updateParameters();

    // Monitoring
    PerformanceMetrics getPerformanceMetrics() const;
    void setMonitoringCallback(std::function<void(const PerformanceMetrics&)> callback);

    // Audio-Stream-Verwaltung
    void startAudioStream();
    void stopAudioStream();
    
    // Plugin-Verwaltung
    void loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& pluginId);
    
    // Effekt-Verwaltung
    void addEffect(const std::string& effectType);
    void removeEffect(const std::string& effectId);
    
    // Dolby Atmos Integration
    void enableDolbyAtmos(bool enable);
    void configureDolbyAtmos(const std::string& config);
    
    // Recording
    void startRecording();
    void stopRecording();
    void saveRecording(const std::string& path);
    
    // Mixing & Mastering
    void setMasterVolume(float volume);
    void setChannelVolume(int channel, float volume);
    void setPan(int channel, float pan);
    
    // AI-Funktionen
    void applyAIMastering();
    void applyAIMixing();
    
    // Sample-Verwaltung
    void loadSample(const std::string& path);
    void updateSampleBank();
    
    // Bouncing
    void bounceToFile(const std::string& path, const std::string& format);

    // Neue fortschrittliche Funktionen
    void enableSpectralAnalysis(bool enable);
    void setSpectralAnalysisMode(const std::string& mode); // "Real-time", "Offline", "Hybrid"
    void enableAdaptiveProcessing(bool enable);
    void setAdaptiveProcessingMode(const std::string& mode); // "Dynamic", "Static", "Learning"
    
    // Erweiterte Plugin-Funktionen
    void enablePluginParallelProcessing(bool enable);
    void setPluginProcessingMode(const std::string& mode); // "Serial", "Parallel", "Hybrid"
    void enablePluginStateSaving(bool enable);
    
    // Erweiterte Mixing-Funktionen
    void enableAdvancedMixing(bool enable);
    void setMixingMode(const std::string& mode); // "Traditional", "3D", "Immersive"
    void enableAutoMixing(bool enable);
    
    // Erweiterte Mastering-Funktionen
    void enableAdvancedMastering(bool enable);
    void setMasteringMode(const std::string& mode); // "Standard", "AI", "Custom"
    void enableAutoMastering(bool enable);
    
    // Erweiterte Recording-Funktionen
    void enableAdvancedRecording(bool enable);
    void setRecordingMode(const std::string& mode); // "Standard", "Multi-track", "Immersive"
    void enableAutoRecording(bool enable);
    
    // Erweiterte Streaming-Funktionen
    void enableAdvancedStreaming(bool enable);
    void setStreamingMode(const std::string& mode); // "Standard", "Low-latency", "High-quality"
    void enableAutoStreaming(bool enable);
    
    // Erweiterte Analyse-Funktionen
    void enableAdvancedAnalysis(bool enable);
    void setAnalysisMode(const std::string& mode); // "Real-time", "Offline", "Hybrid"
    void enableAutoAnalysis(bool enable);
    
    // Erweiterte Verarbeitungs-Funktionen
    void enableAdvancedProcessing(bool enable);
    void setProcessingMode(const std::string& mode); // "Standard", "AI", "Custom"
    void enableAutoProcessing(bool enable);
    
    // Erweiterte Spatialization-Funktionen
    void enableAdvancedSpatialization(bool enable);
    void setSpatializationMode(const std::string& mode); // "Standard", "3D", "Immersive"
    void enableAutoSpatialization(bool enable);
    
    // Erweiterte Synthesis-Funktionen
    void enableAdvancedSynthesis(bool enable);
    void setSynthesisMode(const std::string& mode); // "Standard", "AI", "Custom"
    void enableAutoSynthesis(bool enable);
    
    // Erweiterte Playback-Funktionen
    void enableAdvancedPlayback(bool enable);
    void setPlaybackMode(const std::string& mode); // "Standard", "3D", "Immersive"
    void enableAutoPlayback(bool enable);
    
    // Erweiterte Effekt-Funktionen
    void enableAdvancedEffects(bool enable);
    void setEffectsMode(const std::string& mode); // "Standard", "AI", "Custom"
    void enableAutoEffects(bool enable);
    
    // Erweiterte Mixer-Funktionen
    void enableAdvancedMixer(bool enable);
    void setMixerMode(const std::string& mode); // "Standard", "3D", "Immersive"
    void enableAutoMixer(bool enable);

    // Performance-Optimierungen
    void enableSIMDOptimization(bool enable);
    void setThreadPoolSize(int size);
    
    // Audio-Buffering
    void setAudioBufferSize(int size);
    void setAudioBufferCount(int count);
    
    // Thread-Pool-Verwaltung
    void initializeThreadPool();
    void shutdownThreadPool();

    void setPerformanceOptimizations(const PerformanceOptimizations& optimizations);
    PerformanceOptimizations getPerformanceOptimizations() const;
    void optimizeForVR();
    void setAdaptiveBuffering(bool enable);
    void setPredictiveLoading(bool enable);

    // GPU-Beschleunigung
    void initializeGPUAcceleration();
    void shutdownGPUAcceleration();
    void processAudioWithGPU(juce::AudioBuffer<float>& buffer);
    void setGPUProcessingThreshold(float threshold);
    bool isGPUAccelerationAvailable() const;

private:
    // Audio-Engine-Komponenten
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<juce::AudioProcessorPlayer> processorPlayer;
    std::vector<std::unique_ptr<juce::AudioProcessor>> plugins;
    std::unique_ptr<juce::MidiMessageCollector> midiCollector;

    // Performance-Optimierung
    juce::dsp::ProcessSpec processSpec;
    std::vector<juce::dsp::ProcessorDuplicator> processorDuplicators;
    std::vector<std::thread> processingThreads;
    std::atomic<bool> isProcessing;

    // Plugin-Management
    std::map<std::string, PluginInfo> pluginRegistry;
    std::vector<std::string> pluginSearchPaths;
    juce::KnownPluginList knownPluginList;

    // Monitoring
    PerformanceMetrics currentMetrics;
    std::function<void(const PerformanceMetrics&)> monitoringCallback;
    juce::Timer metricsTimer;

    // Hilfsfunktionen
    void initializeAudioDevice();
    void initializePlugins();
    void optimizeProcessingChain();
    void updatePerformanceMetrics();
    void handlePluginScanResults(const juce::KnownPluginList::PluginTree& tree);

    // JUCE Audio-Komponenten
    std::vector<std::unique_ptr<juce::AudioPluginInstance>> loadedPlugins;
    
    // Effekt-Kette
    std::vector<std::unique_ptr<juce::AudioProcessor>> effects;
    
    // Dolby Atmos
    bool dolbyAtmosEnabled = false;
    std::unique_ptr<class DolbyAtmosProcessor> atmosProcessor;
    
    // Recording
    std::unique_ptr<juce::AudioFormatWriter> recordingWriter;
    std::unique_ptr<juce::FileOutputStream> recordingStream;
    
    // Sample-Bank
    std::vector<std::unique_ptr<juce::AudioFormatReader>> sampleBank;
    
    // AI-Komponenten
    std::unique_ptr<class AIMasteringProcessor> aiMastering;
    std::unique_ptr<class AIMixingProcessor> aiMixing;
    
    // Neue Komponenten
    std::unique_ptr<class SpectralAnalyzer> spectralAnalyzer;
    std::unique_ptr<class AdaptiveProcessor> adaptiveProcessor;
    std::unique_ptr<class ThreadPool> pluginThreadPool;
    std::unique_ptr<class PluginStateManager> pluginStateManager;
    std::unique_ptr<class AdvancedMixer> advancedMixer;
    std::unique_ptr<class AdvancedMastering> advancedMastering;
    std::unique_ptr<class AdvancedRecording> advancedRecording;
    std::unique_ptr<class AdvancedStreaming> advancedStreaming;
    std::unique_ptr<class AdvancedAnalysis> advancedAnalysis;
    std::unique_ptr<class AdvancedProcessing> advancedProcessing;
    std::unique_ptr<class AdvancedSpatialization> advancedSpatialization;
    std::unique_ptr<class AdvancedSynthesis> advancedSynthesis;
    std::unique_ptr<class AdvancedPlayback> advancedPlayback;
    std::unique_ptr<class AdvancedEffects> advancedEffects;
    
    // Zustandsvariablen
    bool pluginParallelProcessing = false;
    enum class PluginProcessingMode { Serial, Parallel, Hybrid } pluginProcessingMode = PluginProcessingMode::Serial;
    bool pluginStateSaving = false;

    // Performance-Optimierungen
    bool simdEnabled = false;
    int bufferSize = 512;
    int threadPoolSize = std::thread::hardware_concurrency();
    std::unique_ptr<ThreadPool> threadPool;
    
    // Audio-Buffering
    int audioBufferSize = 1024;
    int audioBufferCount = 3;
    std::vector<juce::AudioBuffer<float>> audioBuffers;
    std::atomic<int> currentBufferIndex{0};

    PerformanceOptimizations currentOptimizations;
    std::unique_ptr<GPUAccelerator> gpuAccelerator;
    bool gpuAccelerationEnabled = false;
    float gpuProcessingThreshold = 0.8f;
    std::unique_ptr<class AdaptiveBuffer> adaptiveBuffer;
    std::unique_ptr<class PredictiveLoader> predictiveLoader;
    std::unique_ptr<class MemoryOptimizer> memoryOptimizer;
}; 