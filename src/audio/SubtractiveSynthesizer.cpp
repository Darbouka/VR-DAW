#include "SubtractiveSynthesizer.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VR_DAW {

SubtractiveSynthesizer::SubtractiveSynthesizer()
    : oscillatorType(OscillatorType::Sine)
    , oscillatorMix(1.0f)
    , oscillatorDetune(0.0f)
    , oscillatorPhase(0.0f)
    , filterType("lowpass")
    , filterDrive(1.0f)
    , filterEnvelopeAmount(0.0f)
    , filterEnvelopeAttack(0.1f)
    , filterEnvelopeDecay(0.1f)
    , lfoWaveform("sine")
    , lfoDestination("filter")
    , filterEnvelope(0.0f)
{
    filterState.fill(0.0f);
}

SubtractiveSynthesizer::~SubtractiveSynthesizer() {
}

void SubtractiveSynthesizer::setOscillatorType(OscillatorType type) {
    std::lock_guard<std::mutex> lock(mutex);
    oscillatorType = type;
}

void SubtractiveSynthesizer::setOscillatorMix(float mix) {
    std::lock_guard<std::mutex> lock(mutex);
    oscillatorMix = std::max(0.0f, std::min(1.0f, mix));
}

void SubtractiveSynthesizer::setOscillatorDetune(float detune) {
    std::lock_guard<std::mutex> lock(mutex);
    oscillatorDetune = std::max(-1.0f, std::min(1.0f, detune));
}

void SubtractiveSynthesizer::setOscillatorPhase(float phase) {
    std::lock_guard<std::mutex> lock(mutex);
    oscillatorPhase = std::max(0.0f, std::min(1.0f, phase));
}

void SubtractiveSynthesizer::setFilterType(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex);
    filterType = type;
}

void SubtractiveSynthesizer::setFilterDrive(float drive) {
    std::lock_guard<std::mutex> lock(mutex);
    filterDrive = std::max(1.0f, std::min(10.0f, drive));
}

void SubtractiveSynthesizer::setFilterEnvelopeAmount(float amount) {
    std::lock_guard<std::mutex> lock(mutex);
    filterEnvelopeAmount = std::max(0.0f, std::min(1.0f, amount));
}

void SubtractiveSynthesizer::setFilterEnvelopeAttack(float attack) {
    std::lock_guard<std::mutex> lock(mutex);
    filterEnvelopeAttack = std::max(0.001f, attack);
}

void SubtractiveSynthesizer::setFilterEnvelopeDecay(float decay) {
    std::lock_guard<std::mutex> lock(mutex);
    filterEnvelopeDecay = std::max(0.001f, decay);
}

void SubtractiveSynthesizer::setLFOWaveform(const std::string& waveform) {
    std::lock_guard<std::mutex> lock(mutex);
    lfoWaveform = waveform;
}

void SubtractiveSynthesizer::setLFODestination(const std::string& destination) {
    std::lock_guard<std::mutex> lock(mutex);
    lfoDestination = destination;
}

void SubtractiveSynthesizer::processBlock(float* output, size_t numSamples) {
    std::lock_guard<std::mutex> lock(mutex);

    for (size_t i = 0; i < numSamples; i += 2) {
        float leftSample = 0.0f;
        float rightSample = 0.0f;

        // Alle aktiven Stimmen verarbeiten
        for (auto& voice : voices) {
            if (voice.active) {
                float sample = generateSample(voice);
                updateEnvelope(voice);
                applyFilter(sample);
                applyLFO(sample);
                applyVolume(sample);

                // Stereopanning anwenden
                float leftGain = 1.0f - std::max(0.0f, pan);
                float rightGain = 1.0f + std::min(0.0f, pan);
                leftSample += sample * leftGain;
                rightSample += sample * rightGain;
            }
        }

        // Ausgabe schreiben
        output[i] = leftSample;
        output[i + 1] = rightSample;
    }
}

float SubtractiveSynthesizer::generateSample(const Voice& voice) {
    float sample = 0.0f;
    float phase = voice.phase;

    // Oszillator-Typ auswählen
    switch (oscillatorType) {
        case OscillatorType::Sine:
            sample = generateSineWave(phase);
            break;
        case OscillatorType::Square:
            sample = generateSquareWave(phase);
            break;
        case OscillatorType::Saw:
            sample = generateSawWave(phase);
            break;
        case OscillatorType::Triangle:
            sample = generateTriangleWave(phase);
            break;
        case OscillatorType::Noise:
            sample = generateNoise();
            break;
    }

    // Hüllkurve anwenden
    sample *= voice.amplitude;

    return sample;
}

void SubtractiveSynthesizer::updateEnvelope(Voice& voice) {
    Synthesizer::updateEnvelope(voice);
    updateFilterEnvelope();
}

void SubtractiveSynthesizer::applyFilter(float& sample) {
    // Filter-Typ auswählen
    if (filterType == "lowpass") {
        // Tiefpassfilter
        float cutoff = filterCutoff * (1.0f + filterEnvelope * filterEnvelopeAmount);
        float resonance = filterResonance;
        float drive = filterDrive;

        // Moog-ähnlicher Tiefpassfilter
        float input = sample * drive;
        float feedback = filterState[3] * resonance;

        filterState[0] = input - feedback;
        filterState[0] = std::tanh(filterState[0]);
        filterState[1] = filterState[0] + filterState[1];
        filterState[1] = std::tanh(filterState[1]);
        filterState[2] = filterState[1] + filterState[2];
        filterState[2] = std::tanh(filterState[2]);
        filterState[3] = filterState[2] + filterState[3];
        filterState[3] = std::tanh(filterState[3]);

        sample = filterState[3];
    } else if (filterType == "highpass") {
        // Hochpassfilter
        float cutoff = filterCutoff * (1.0f + filterEnvelope * filterEnvelopeAmount);
        float resonance = filterResonance;

        float input = sample;
        float feedback = filterState[3] * resonance;

        filterState[0] = input - feedback;
        filterState[1] = filterState[0] - filterState[1];
        filterState[2] = filterState[1] - filterState[2];
        filterState[3] = filterState[2] - filterState[3];

        sample = filterState[3];
    }
}

void SubtractiveSynthesizer::applyLFO(float& sample) {
    float lfoValue = generateLFO(0.0f); // Phase wird intern verwaltet

    if (lfoDestination == "filter") {
        filterCutoff *= (1.0f + lfoValue * lfoDepth);
    } else if (lfoDestination == "amplitude") {
        sample *= (1.0f + lfoValue * lfoDepth);
    } else if (lfoDestination == "pitch") {
        sample *= std::pow(2.0f, lfoValue * lfoDepth);
    }
}

float SubtractiveSynthesizer::generateSineWave(float phase) {
    return std::sin(2.0f * M_PI * phase);
}

float SubtractiveSynthesizer::generateSquareWave(float phase) {
    return phase < 0.5f ? 1.0f : -1.0f;
}

float SubtractiveSynthesizer::generateSawWave(float phase) {
    return 2.0f * phase - 1.0f;
}

float SubtractiveSynthesizer::generateTriangleWave(float phase) {
    if (phase < 0.5f) {
        return 4.0f * phase - 1.0f;
    } else {
        return 3.0f - 4.0f * phase;
    }
}

float SubtractiveSynthesizer::generateNoise() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(gen);
}

float SubtractiveSynthesizer::generateLFO(float phase) {
    if (lfoWaveform == "sine") {
        return std::sin(2.0f * M_PI * phase);
    } else if (lfoWaveform == "square") {
        return phase < 0.5f ? 1.0f : -1.0f;
    } else if (lfoWaveform == "saw") {
        return 2.0f * phase - 1.0f;
    } else if (lfoWaveform == "triangle") {
        if (phase < 0.5f) {
            return 4.0f * phase - 1.0f;
        } else {
            return 3.0f - 4.0f * phase;
        }
    }
    return 0.0f;
}

void SubtractiveSynthesizer::updateFilterEnvelope() {
    // Filter-Hüllkurve aktualisieren
    if (filterEnvelope < 1.0f) {
        filterEnvelope += filterEnvelopeAttack;
    } else {
        filterEnvelope -= filterEnvelopeDecay;
    }
    filterEnvelope = std::max(0.0f, std::min(1.0f, filterEnvelope));
}

} // namespace VR_DAW 