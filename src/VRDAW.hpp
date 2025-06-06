#pragma once

#include "audio/AudioEngine.hpp"
#include "midi/MIDIEngine.hpp"
#include "vr/VRSystem.hpp"
#include "vr/VRUI.hpp"
#include <memory>
#include <vector>
#include <map>

namespace VR_DAW {

class VRDAW {
public:
    VRDAW();
    ~VRDAW();

    bool initialize();
    void shutdown();
    void update();
    void render();

    // Track-Management
    int createTrack(const std::string& name);
    void deleteTrack(int trackId);
    std::shared_ptr<AudioTrack> getTrack(int trackId);

    // Synthesizer-Management
    void setTrackSynthesizer(int trackId, const std::string& type);
    void updateSynthesizerUI(int trackId);
    void handleSynthesizerParameterChange(int trackId, const std::string& param, float value);

private:
    void loadConfiguration();
    void saveConfiguration();
    void applyConfiguration();
    void connectAudioUI();

    bool initialized;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<VRSystem> vrSystem;
    std::unique_ptr<VRUI> vrUI;
    std::map<int, std::shared_ptr<AudioTrack>> tracks;
    std::map<int, std::unique_ptr<SynthesizerController>> synthesizerControllers;
    int nextTrackId;

    void initializeAudio();
    void initializeMIDI();
    void initializeVR();
    void cleanup();
    void updateTracks();
    void renderTracks();
};

} // namespace VR_DAW 