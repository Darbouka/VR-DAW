#pragma once

#include <string>
#include <vector>
#include <memory>
#include <portaudio.h>
#include <sndfile.h>
#include "../audio/AudioProcessing.hpp"
#include "../audio/Automation.hpp"
#include "../audio/Mixer.hpp"
#include "../audio/Effects.hpp"

namespace VR_DAW {

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize();
    void shutdown();
    
    // Audio-Streaming
    bool startStream();
    void stopStream();
    
    // Projekt-Management
    bool loadProject(const std::string& path);
    bool saveProject(const std::string& path);
    
    // Audio-Verarbeitung
    void processBlock(float* input, float* output, unsigned long framesPerBuffer);
    void setSampleRate(double rate);
    void setBufferSize(int size);
    
    // Effekte und Automation
    void addEffect(std::shared_ptr<Effects> effect);
    void removeEffect(int index);
    void setAutomation(const std::string& parameter, float value);
    
    // Mixer-Kontrolle
    void setTrackVolume(int track, float volume);
    void setTrackPan(int track, float pan);
    void muteTrack(int track);
    void soloTrack(int track);

private:
    PaStream* stream;
    double sampleRate;
    int bufferSize;
    bool isInitialized;
    
    std::unique_ptr<AudioProcessing> audioProcessor;
    std::unique_ptr<Automation> automation;
    std::unique_ptr<Mixer> mixer;
    std::vector<std::shared_ptr<Effects>> effects;
    
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData);
};

} // namespace VR_DAW 