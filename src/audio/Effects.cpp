#include "Effects.hpp"
#include <cmath>
#include <algorithm>

namespace VR_DAW {

// Basis-Effekt-Klasse
Effects::Effects() : bypass(false), sampleRate(44100.0f) {}

// Reverb-Effekt
ReverbEffect::ReverbEffect()
    : mix(0.5f)
    , time(2.0f)
    , damping(0.5f)
    , writePos(0)
{
    delayBuffer.resize(static_cast<size_t>(sampleRate * time * 2));
}

void ReverbEffect::process(float* buffer, unsigned long framesPerBuffer) {
    if (bypass) return;

    float feedback = std::pow(0.001f, 1.0f / (time * sampleRate));
    
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        // Delay-Line
        size_t delayPos = (writePos + static_cast<size_t>(time * sampleRate)) % delayBuffer.size();
        float delayedLeft = delayBuffer[delayPos];
        float delayedRight = delayBuffer[delayPos + 1];
        
        // Feedback mit Dämpfung
        delayBuffer[writePos] = left + delayedLeft * feedback * (1.0f - damping);
        delayBuffer[writePos + 1] = right + delayedRight * feedback * (1.0f - damping);
        
        // Mix
        buffer[i * 2] = left * (1.0f - mix) + delayedLeft * mix;
        buffer[i * 2 + 1] = right * (1.0f - mix) + delayedRight * mix;
        
        writePos = (writePos + 2) % delayBuffer.size();
    }
}

void ReverbEffect::setParameter(const std::string& name, float value) {
    if (name == "mix") mix = std::max(0.0f, std::min(1.0f, value));
    else if (name == "time") time = std::max(0.1f, std::min(10.0f, value));
    else if (name == "damping") damping = std::max(0.0f, std::min(1.0f, value));
}

float ReverbEffect::getParameter(const std::string& name) const {
    if (name == "mix") return mix;
    if (name == "time") return time;
    if (name == "damping") return damping;
    return 0.0f;
}

std::vector<std::string> ReverbEffect::getParameterNames() const {
    return {"mix", "time", "damping"};
}

void ReverbEffect::reset() {
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    writePos = 0;
}

// Delay-Effekt
DelayEffect::DelayEffect()
    : time(0.5f)
    , feedback(0.3f)
    , mix(0.5f)
    , writePos(0)
{
    delayBuffer.resize(static_cast<size_t>(sampleRate * 2.0f * 2));
}

void DelayEffect::process(float* buffer, unsigned long framesPerBuffer) {
    if (bypass) return;

    size_t delaySamples = static_cast<size_t>(time * sampleRate);
    
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        // Delay-Line
        size_t delayPos = (writePos + delaySamples) % delayBuffer.size();
        float delayedLeft = delayBuffer[delayPos];
        float delayedRight = delayBuffer[delayPos + 1];
        
        // Feedback
        delayBuffer[writePos] = left + delayedLeft * feedback;
        delayBuffer[writePos + 1] = right + delayedRight * feedback;
        
        // Mix
        buffer[i * 2] = left * (1.0f - mix) + delayedLeft * mix;
        buffer[i * 2 + 1] = right * (1.0f - mix) + delayedRight * mix;
        
        writePos = (writePos + 2) % delayBuffer.size();
    }
}

void DelayEffect::setParameter(const std::string& name, float value) {
    if (name == "time") time = std::max(0.0f, std::min(2.0f, value));
    else if (name == "feedback") feedback = std::max(0.0f, std::min(0.99f, value));
    else if (name == "mix") mix = std::max(0.0f, std::min(1.0f, value));
}

float DelayEffect::getParameter(const std::string& name) const {
    if (name == "time") return time;
    if (name == "feedback") return feedback;
    if (name == "mix") return mix;
    return 0.0f;
}

std::vector<std::string> DelayEffect::getParameterNames() const {
    return {"time", "feedback", "mix"};
}

void DelayEffect::reset() {
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    writePos = 0;
}

// Compressor-Effekt
CompressorEffect::CompressorEffect()
    : threshold(-20.0f)
    , ratio(4.0f)
    , attack(0.003f)
    , release(0.25f)
    , envelope(0.0f)
{
}

void CompressorEffect::process(float* buffer, unsigned long framesPerBuffer) {
    if (bypass) return;

    float attackCoeff = std::exp(-1.0f / (attack * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (release * sampleRate));
    
    for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
        float input = std::abs(buffer[i]);
        
        // Hüllkurve
        if (input > envelope) {
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * input;
        } else {
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * input;
        }
        
        // Kompression
        if (envelope > threshold) {
            float gain = threshold + (envelope - threshold) / ratio;
            buffer[i] *= gain / envelope;
        }
    }
}

void CompressorEffect::setParameter(const std::string& name, float value) {
    if (name == "threshold") threshold = std::max(-60.0f, std::min(0.0f, value));
    else if (name == "ratio") ratio = std::max(1.0f, std::min(20.0f, value));
    else if (name == "attack") attack = std::max(0.001f, std::min(1.0f, value));
    else if (name == "release") release = std::max(0.001f, std::min(1.0f, value));
}

float CompressorEffect::getParameter(const std::string& name) const {
    if (name == "threshold") return threshold;
    if (name == "ratio") return ratio;
    if (name == "attack") return attack;
    if (name == "release") return release;
    return 0.0f;
}

std::vector<std::string> CompressorEffect::getParameterNames() const {
    return {"threshold", "ratio", "attack", "release"};
}

void CompressorEffect::reset() {
    envelope = 0.0f;
}

} // namespace VR_DAW 