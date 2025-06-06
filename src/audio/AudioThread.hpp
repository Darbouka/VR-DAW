#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include "AudioEvent.hpp"

namespace VR_DAW {

class AudioThread {
public:
    AudioThread();
    ~AudioThread();

    bool initialize(int sampleRate, int bufferSize, int numChannels);
    void shutdown();
    
    // Audio-Verarbeitung
    void processAudio(float* inputBuffer, float* outputBuffer, int numFrames);
    void handleAudioEvent(const AudioEvent& event);
    
    // Thread-Steuerung
    void start();
    void stop();
    bool isRunning() const;
    
    // Callbacks
    using AudioCallback = std::function<void(float* input, float* output, int numFrames)>;
    using EventCallback = std::function<void(const AudioEvent&)>;
    
    void setAudioCallback(AudioCallback callback);
    void setEventCallback(EventCallback callback);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    // Thread-Funktionen
    void audioThreadFunction();
    void processEvents();
    
    // Audio-System-spezifische Implementierung
    #ifdef USE_JACK
    void initializeJACK();
    #else
    void initializePortAudio();
    #endif
};

} // namespace VR_DAW 