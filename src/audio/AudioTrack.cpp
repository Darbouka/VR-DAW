#include "AudioTrack.hpp"
#include "SubtractiveSynthesizer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace VR_DAW {

AudioTrack::AudioTrack()
    : midiEnabled(false)
    , midiChannel(0)
    , pitchBend(0.0f)
    , volume(1.0f)
    , pan(0.0f)
    , muted(false)
    , solo(false)
    , active(true)
    , name("Unnamed Track")
{
    // Standard-Synthesizer-Parameter
    synthesizerParameters["oscillator_type"] = 0.0f; // Sine
    synthesizerParameters["filter_cutoff"] = 1000.0f;
    synthesizerParameters["filter_resonance"] = 0.7f;
    synthesizerParameters["envelope_attack"] = 0.1f;
    synthesizerParameters["envelope_decay"] = 0.1f;
    synthesizerParameters["envelope_sustain"] = 0.7f;
    synthesizerParameters["envelope_release"] = 0.2f;
    synthesizerParameters["lfo_rate"] = 5.0f;
    synthesizerParameters["lfo_depth"] = 0.1f;
}

AudioTrack::~AudioTrack() {
}

void AudioTrack::processMIDINoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!midiEnabled || channel != midiChannel) return;

    std::lock_guard<std::mutex> lock(midiMutex);
    activeNotes[note] = velocity;

    if (synthesizer) {
        synthesizer->noteOn(note, velocity, channel);
    }
}

void AudioTrack::processMIDINoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!midiEnabled || channel != midiChannel) return;

    std::lock_guard<std::mutex> lock(midiMutex);
    activeNotes.erase(note);

    if (synthesizer) {
        synthesizer->noteOff(note, velocity, channel);
    }
}

void AudioTrack::processMIDIControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    if (!midiEnabled || channel != midiChannel) return;

    std::lock_guard<std::mutex> lock(midiMutex);
    controllers[controller] = value;

    if (synthesizer) {
        synthesizer->setController(controller, value, channel);
    }
}

void AudioTrack::processMIDIPitchBend(uint8_t channel, uint16_t value) {
    if (!midiEnabled || channel != midiChannel) return;

    std::lock_guard<std::mutex> lock(midiMutex);
    pitchBend = (value - 8192) / 8192.0f;

    if (synthesizer) {
        synthesizer->setPitchBend(pitchBend, channel);
    }
}

void AudioTrack::setMIDIEnabled(bool enabled) {
    midiEnabled = enabled;
}

bool AudioTrack::isMIDIEnabled() const {
    return midiEnabled;
}

void AudioTrack::setMIDIChannel(uint8_t channel) {
    midiChannel = channel;
}

uint8_t AudioTrack::getMIDIChannel() const {
    return midiChannel;
}

void AudioTrack::setSynthesizer(std::shared_ptr<Synthesizer> synth) {
    std::lock_guard<std::mutex> lock(synthMutex);
    synthesizer = synth;
    updateSynthesizerParameters();
}

std::shared_ptr<Synthesizer> AudioTrack::getSynthesizer() const {
    return synthesizer;
}

void AudioTrack::setSynthesizerType(const std::string& type) {
    std::lock_guard<std::mutex> lock(synthMutex);
    synthesizerType = type;

    // Synthesizer-Typ erstellen
    if (type == "subtractive") {
        synthesizer = std::make_shared<SubtractiveSynthesizer>();
        updateSynthesizerParameters();
    }
    // Weitere Synthesizer-Typen hier...
}

std::string AudioTrack::getSynthesizerType() const {
    return synthesizerType;
}

void AudioTrack::setSynthesizerParameter(const std::string& param, float value) {
    std::lock_guard<std::mutex> lock(synthMutex);
    synthesizerParameters[param] = value;

    if (synthesizer) {
        if (param == "oscillator_type") {
            if (auto* subSynth = dynamic_cast<SubtractiveSynthesizer*>(synthesizer.get())) {
                subSynth->setOscillatorType(static_cast<SubtractiveSynthesizer::OscillatorType>(
                    static_cast<int>(value)));
            }
        } else if (param == "filter_cutoff") {
            synthesizer->setFilterCutoff(value);
        } else if (param == "filter_resonance") {
            synthesizer->setFilterResonance(value);
        } else if (param == "envelope_attack") {
            Synthesizer::Envelope env = synthesizer->getEnvelope();
            env.attack = value;
            synthesizer->setEnvelope(env);
        } else if (param == "envelope_decay") {
            Synthesizer::Envelope env = synthesizer->getEnvelope();
            env.decay = value;
            synthesizer->setEnvelope(env);
        } else if (param == "envelope_sustain") {
            Synthesizer::Envelope env = synthesizer->getEnvelope();
            env.sustain = value;
            synthesizer->setEnvelope(env);
        } else if (param == "envelope_release") {
            Synthesizer::Envelope env = synthesizer->getEnvelope();
            env.release = value;
            synthesizer->setEnvelope(env);
        } else if (param == "lfo_rate") {
            synthesizer->setLFORate(value);
        } else if (param == "lfo_depth") {
            synthesizer->setLFODepth(value);
        }
    }
}

float AudioTrack::getSynthesizerParameter(const std::string& param) const {
    auto it = synthesizerParameters.find(param);
    return it != synthesizerParameters.end() ? it->second : 0.0f;
}

void AudioTrack::processBlock(float* output, size_t numSamples) {
    if (!active || muted) return;

    std::lock_guard<std::mutex> lock(audioMutex);

    // Synthesizer-Audio verarbeiten
    if (synthesizer) {
        synthesizer->processBlock(output, numSamples);
    }

    // Audio-Verarbeitung anwenden
    applyAudioProcessing(output, numSamples);
}

void AudioTrack::setVolume(float vol) {
    volume = std::max(0.0f, std::min(1.0f, vol));
}

float AudioTrack::getVolume() const {
    return volume;
}

void AudioTrack::setPan(float p) {
    pan = std::max(-1.0f, std::min(1.0f, p));
}

float AudioTrack::getPan() const {
    return pan;
}

void AudioTrack::setMute(bool mute) {
    muted = mute;
}

bool AudioTrack::isMuted() const {
    return muted;
}

void AudioTrack::setSolo(bool s) {
    solo = s;
}

bool AudioTrack::isSolo() const {
    return solo;
}

bool AudioTrack::isActive() const {
    return active;
}

void AudioTrack::setActive(bool act) {
    active = act;
}

const std::string& AudioTrack::getName() const {
    return name;
}

void AudioTrack::setName(const std::string& n) {
    name = n;
}

void AudioTrack::updateSynthesizerParameters() {
    if (!synthesizer) return;

    for (const auto& [param, value] : synthesizerParameters) {
        setSynthesizerParameter(param, value);
    }
}

void AudioTrack::applyAudioProcessing(float* output, size_t numSamples) {
    // Volume und Pan anwenden
    for (size_t i = 0; i < numSamples; i += 2) {
        float leftSample = output[i];
        float rightSample = output[i + 1];

        // Volume
        leftSample *= volume;
        rightSample *= volume;

        // Pan
        float leftGain = 1.0f - std::max(0.0f, pan);
        float rightGain = 1.0f + std::min(0.0f, pan);
        leftSample *= leftGain;
        rightSample *= rightGain;

        output[i] = leftSample;
        output[i + 1] = rightSample;
    }
}

} // namespace VR_DAW 