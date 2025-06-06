#pragma once

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <portaudio.h>
#include <jack/jack.h>
#include <atomic>
#include <functional>
#include <immintrin.h>
#include "../midi/MIDIEngine.hpp"
#include "AudioEvent.hpp"
#include "SynthesizerConfig.hpp"

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
    void processAudioBlock(float* buffer, int numSamples);
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
    static AudioEngine& getInstance();

    void initialize();
    void shutdown();
    void update();

    // Synthesizer-Management
    void createSynthesizer(int trackId, const SynthesizerConfig& config);
    void updateSynthesizer(int trackId, const SynthesizerConfig& config);
    void deleteSynthesizer(int trackId);

    // Audio-Verarbeitung
    void processAudio(float* buffer, size_t numFrames);
    void handleAudioEvent(const AudioEvent& event);

    // Callback-System
    using AudioCallback = std::function<void(const AudioEvent&)>;
    void setAudioCallback(AudioCallback callback);

    struct AudioTrack {
        int id;
        std::string name;
        std::vector<float> buffer;
        float volume;
        float pan;
        bool muted;
        bool soloed;
        std::vector<std::string> plugins;
        std::atomic<bool> isProcessing;
        std::mutex bufferMutex;
    };

    struct AudioPlugin {
        int id;
        std::string name;
        std::string type;
        std::map<std::string, float> parameters;
        std::atomic<bool> isProcessing;
        std::mutex parameterMutex;
    };

    struct AudioBuffer {
        float* data;
        size_t size;
        int channels;
        int sampleRate;
        std::atomic<bool> isLocked;
    };

    AudioEngine();
    ~AudioEngine();

    void process(float* input, float* output, unsigned long frameCount);

    AudioTrack* createTrack(const std::string& name);
    void deleteTrack(int trackId);
    void updateTrack(AudioTrack* track);
    
    AudioPlugin* loadPlugin(const std::string& name, const std::string& type);
    void unloadPlugin(int pluginId);
    void setPluginParameter(int pluginId, const std::string& paramName, float value);
    
    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    void setPlaybackPosition(double position);
    
    void setMasterVolume(float volume);
    void setSampleRate(int rate);
    void setBufferSize(int size);
    
    std::vector<float> getWaveform(int trackId, int channel);
    void updateWaveform(int trackId, const std::vector<float>& data);

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
    void start();
    void stop();
    void pause();
    void resume();
    void record();

    // Performance-Optimierung
    void optimizeMemoryUsage();

    // Plugin-Management
    void scanPluginDirectory(const std::string& directory);
    std::vector<PluginInfo> getAvailablePlugins() const;

    // Audio-Verarbeitung
    void processBlock(float* buffer, int numSamples);
    void processMIDI(const std::vector<unsigned char>& message);
    void updateParameters();

    // Monitoring
    PerformanceMetrics getPerformanceMetrics() const;
    void setMonitoringCallback(std::function<void(const PerformanceMetrics&)> callback);

    // Audio-Stream-Verwaltung
    void startAudioStream();
    void stopAudioStream();
    
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
    void processAudioWithGPU(float* buffer, int numSamples);
    void setGPUProcessingThreshold(float threshold);
    bool isGPUAccelerationAvailable() const;

    // Neue Methoden für optimierte Verarbeitung
    void processAudioBuffers();
    void optimizeBufferSize();
    void setThreadCount(int count);
    void enableSIMD(bool enable);
    void setProcessingMode(ProcessingMode mode);

    // MIDI-Integration
    void setMIDIEngine(std::shared_ptr<MIDIEngine> midiEngine);
    void processMIDIInput();
    void handleMIDIMessage(const MIDIEngine::MIDIMessage& message);
    void startMIDIRecording();
    void stopMIDIRecording();
    bool isMIDIRecording() const;
    std::vector<MIDIEngine::MIDIMessage> getRecordedMIDI() const;
    void clearMIDIRecording();
    void setMIDICallback(std::function<void(const MIDIEngine::MIDIMessage&)> callback);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    bool initialized;
    int sampleRate;
    int bufferSize;
    float masterVolume;
    bool isPlaying;
    double playbackPosition;
    
    std::vector<AudioTrack> tracks;
    std::vector<AudioPlugin> plugins;
    
    PaStream* stream;
    jack_client_t* jackClient;

    // Neue Member für optimierte Verarbeitung
    std::vector<std::thread> processingThreads;
    std::queue<AudioBuffer> bufferQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldProcess;
    int threadCount;
    bool simdEnabled;
    ProcessingMode processingMode;

    enum class ProcessingMode {
        RealTime,
        Offline
    };
    
    void initializePortAudio();
    void initializeJack();
    void processAudio(float* input, float* output, unsigned long frameCount);
    void applyEffects(AudioTrack& track, float* buffer, unsigned long frameCount);
    void processBuffer(AudioBuffer& buffer);
    void optimizeProcessing();
    void initializeThreads();
    void cleanupThreads();

    // MIDI-bezogene Member
    std::shared_ptr<MIDIEngine> midiEngine;
    std::atomic<bool> midiRecordingActive;
    std::vector<MIDIEngine::MIDIMessage> recordedMIDI;
    std::function<void(const MIDIEngine::MIDIMessage&)> midiCallback;
    std::mutex midiMutex;
};

} // namespace VR_DAW 