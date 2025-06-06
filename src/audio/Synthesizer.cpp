#include "Synthesizer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace VR_DAW {

Synthesizer::Synthesizer()
    : filterCutoff(1000.0f)
    , filterResonance(0.7f)
    , lfoRate(5.0f)
    , lfoDepth(0.1f)
    , volume(1.0f)
    , pan(0.0f)
    , sampleRate(44100.0f)
    , active(false)
{
    // Standard-Envelope
    envelope.attack = 0.1f;
    envelope.decay = 0.1f;
    envelope.sustain = 0.7f;
    envelope.release = 0.2f;

    // 16 Stimmen initialisieren
    voices.resize(16);
    for (auto& voice : voices) {
        voice.frequency = 0.0f;
        voice.velocity = 0.0f;
        voice.phase = 0.0f;
        voice.amplitude = 0.0f;
        voice.active = false;
        voice.note = 0;
        voice.channel = 0;
    }
}

Synthesizer::~Synthesizer() {
}

void Synthesizer::noteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    std::lock_guard<std::mutex> lock(mutex);

    // Freie Stimme suchen
    auto it = std::find_if(voices.begin(), voices.end(),
        [](const Voice& v) { return !v.active; });

    if (it != voices.end()) {
        // MIDI-Note in Frequenz umrechnen
        float frequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);

        it->frequency = frequency;
        it->velocity = velocity / 127.0f;
        it->phase = 0.0f;
        it->amplitude = 0.0f;
        it->active = true;
        it->note = note;
        it->channel = channel;

        active = true;
    }
}

void Synthesizer::noteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    std::lock_guard<std::mutex> lock(mutex);

    // Stimme mit entsprechender Note finden
    auto it = std::find_if(voices.begin(), voices.end(),
        [note, channel](const Voice& v) {
            return v.active && v.note == note && v.channel == channel;
        });

    if (it != voices.end()) {
        it->active = false;
    }

    // Prüfen, ob noch aktive Stimmen vorhanden sind
    active = std::any_of(voices.begin(), voices.end(),
        [](const Voice& v) { return v.active; });
}

void Synthesizer::setController(uint8_t controller, uint8_t value, uint8_t channel) {
    std::lock_guard<std::mutex> lock(mutex);

    switch (controller) {
        case 1: // Modulation
            setModulation(value / 127.0f, channel);
            break;
        case 7: // Volume
            setVolume(value / 127.0f);
            break;
        case 10: // Pan
            setPan((value / 127.0f) * 2.0f - 1.0f);
            break;
        case 71: // Filter Cutoff
            setFilterCutoff(value * 20.0f);
            break;
        case 74: // Filter Resonance
            setFilterResonance(value / 127.0f);
            break;
    }
}

void Synthesizer::setPitchBend(float value, uint8_t channel) {
    std::lock_guard<std::mutex> lock(mutex);

    // Pitch Bend auf alle Stimmen des Kanals anwenden
    for (auto& voice : voices) {
        if (voice.active && voice.channel == channel) {
            voice.frequency *= std::pow(2.0f, value);
        }
    }
}

void Synthesizer::setModulation(float value, uint8_t channel) {
    std::lock_guard<std::mutex> lock(mutex);
    lfoDepth = value;
}

void Synthesizer::setVolume(float vol) {
    std::lock_guard<std::mutex> lock(mutex);
    volume = std::max(0.0f, std::min(1.0f, vol));
}

void Synthesizer::setPan(float p) {
    std::lock_guard<std::mutex> lock(mutex);
    pan = std::max(-1.0f, std::min(1.0f, p));
}

void Synthesizer::setEnvelope(const Envelope& env) {
    std::lock_guard<std::mutex> lock(mutex);
    envelope = env;
}

void Synthesizer::setOscillatorType(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex);
    oscillatorType = type;
}

void Synthesizer::setFilterCutoff(float cutoff) {
    std::lock_guard<std::mutex> lock(mutex);
    filterCutoff = std::max(20.0f, std::min(20000.0f, cutoff));
}

void Synthesizer::setFilterResonance(float resonance) {
    std::lock_guard<std::mutex> lock(mutex);
    filterResonance = std::max(0.0f, std::min(1.0f, resonance));
}

void Synthesizer::setLFORate(float rate) {
    std::lock_guard<std::mutex> lock(mutex);
    lfoRate = std::max(0.1f, std::min(20.0f, rate));
}

void Synthesizer::setLFODepth(float depth) {
    std::lock_guard<std::mutex> lock(mutex);
    lfoDepth = std::max(0.0f, std::min(1.0f, depth));
}

void Synthesizer::reset() {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto& voice : voices) {
        voice.active = false;
        voice.amplitude = 0.0f;
        voice.phase = 0.0f;
    }
    
    active = false;
}

bool Synthesizer::isActive() const {
    return active;
}

size_t Synthesizer::getActiveVoiceCount() const {
    return std::count_if(voices.begin(), voices.end(),
        [](const Voice& v) { return v.active; });
}

float Synthesizer::getCurrentVolume() const {
    return volume;
}

float Synthesizer::getCurrentPan() const {
    return pan;
}

void Synthesizer::updateEnvelope(Voice& voice) {
    if (!voice.active) {
        voice.amplitude *= (1.0f - envelope.release);
        if (voice.amplitude < 0.001f) {
            voice.amplitude = 0.0f;
        }
    } else {
        if (voice.amplitude < voice.velocity) {
            voice.amplitude += envelope.attack;
        } else if (voice.amplitude > voice.velocity * envelope.sustain) {
            voice.amplitude -= envelope.decay;
        }
    }
}

void Synthesizer::applyFilter(float& sample) {
    // Einfacher Tiefpassfilter
    static float lastSample = 0.0f;
    float alpha = filterCutoff / sampleRate;
    sample = lastSample + alpha * (sample - lastSample);
    lastSample = sample;
}

void Synthesizer::applyLFO(float& sample) {
    static float phase = 0.0f;
    float lfo = std::sin(2.0f * M_PI * phase);
    sample *= (1.0f + lfo * lfoDepth);
    phase += lfoRate / sampleRate;
    if (phase >= 1.0f) phase -= 1.0f;
}

void Synthesizer::applyPan(float& leftSample, float& rightSample) {
    float leftGain = 1.0f - std::max(0.0f, pan);
    float rightGain = 1.0f + std::min(0.0f, pan);
    leftSample *= leftGain;
    rightSample *= rightGain;
}

void Synthesizer::applyVolume(float& sample) {
    sample *= volume;
}

} // namespace VR_DAW 