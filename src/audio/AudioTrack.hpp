#pragma once

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <map>
#include "../midi/MIDIEngine.hpp"
#include "Synthesizer.hpp"

namespace VR_DAW {

class AudioTrack {
public:
    AudioTrack();
    ~AudioTrack();

    // MIDI-Verarbeitung
    void processMIDINoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void processMIDINoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void processMIDIControlChange(uint8_t channel, uint8_t controller, uint8_t value);
    void processMIDIPitchBend(uint8_t channel, uint16_t value);
    void setMIDIEnabled(bool enabled);
    bool isMIDIEnabled() const;
    void setMIDIChannel(uint8_t channel);
    uint8_t getMIDIChannel() const;

    // Synthesizer-Verwaltung
    void setSynthesizer(std::shared_ptr<Synthesizer> synth);
    std::shared_ptr<Synthesizer> getSynthesizer() const;
    void setSynthesizerType(const std::string& type);
    std::string getSynthesizerType() const;
    void setSynthesizerParameter(const std::string& param, float value);
    float getSynthesizerParameter(const std::string& param) const;

    // Audio-Verarbeitung
    void processBlock(float* output, size_t numSamples);
    void setVolume(float volume);
    float getVolume() const;
    void setPan(float pan);
    float getPan() const;
    void setMute(bool mute);
    bool isMuted() const;
    void setSolo(bool solo);
    bool isSolo() const;

    // Status
    bool isActive() const;
    void setActive(bool active);
    const std::string& getName() const;
    void setName(const std::string& name);

private:
    // MIDI-bezogene Member
    std::atomic<bool> midiEnabled;
    uint8_t midiChannel;
    std::mutex midiMutex;
    std::map<uint8_t, uint8_t> activeNotes; // Note -> Velocity
    float pitchBend;
    std::map<uint8_t, uint8_t> controllers; // Controller -> Value

    // Synthesizer-bezogene Member
    std::shared_ptr<Synthesizer> synthesizer;
    std::string synthesizerType;
    std::map<std::string, float> synthesizerParameters;
    std::mutex synthMutex;

    // Audio-bezogene Member
    std::atomic<float> volume;
    std::atomic<float> pan;
    std::atomic<bool> muted;
    std::atomic<bool> solo;
    std::atomic<bool> active;
    std::string name;
    std::mutex audioMutex;

    // Hilfsfunktionen
    void updateSynthesizerParameters();
    void applyAudioProcessing(float* output, size_t numSamples);
};

} // namespace VR_DAW 