#pragma once

#include "Synthesizer.hpp"
#include <array>

namespace VR_DAW {

class SubtractiveSynthesizer : public Synthesizer {
public:
    enum class OscillatorType {
        Sine,
        Square,
        Saw,
        Triangle,
        Noise
    };

    SubtractiveSynthesizer();
    ~SubtractiveSynthesizer() override;

    // Oszillator-Einstellungen
    void setOscillatorType(OscillatorType type);
    void setOscillatorMix(float mix);
    void setOscillatorDetune(float detune);
    void setOscillatorPhase(float phase);

    // Filter-Einstellungen
    void setFilterType(const std::string& type);
    void setFilterDrive(float drive);
    void setFilterEnvelopeAmount(float amount);
    void setFilterEnvelopeAttack(float attack);
    void setFilterEnvelopeDecay(float decay);

    // LFO-Einstellungen
    void setLFOWaveform(const std::string& waveform);
    void setLFODestination(const std::string& destination);

    // Audio-Verarbeitung
    void processBlock(float* output, size_t numSamples) override;

protected:
    float generateSample(const Voice& voice) override;
    void updateEnvelope(Voice& voice) override;
    void applyFilter(float& sample) override;
    void applyLFO(float& sample) override;

private:
    // Oszillator-Parameter
    OscillatorType oscillatorType;
    float oscillatorMix;
    float oscillatorDetune;
    float oscillatorPhase;

    // Filter-Parameter
    std::string filterType;
    float filterDrive;
    float filterEnvelopeAmount;
    float filterEnvelopeAttack;
    float filterEnvelopeDecay;

    // LFO-Parameter
    std::string lfoWaveform;
    std::string lfoDestination;

    // Filter-Status
    std::array<float, 4> filterState;
    float filterEnvelope;

    // Hilfsfunktionen
    float generateSineWave(float phase);
    float generateSquareWave(float phase);
    float generateSawWave(float phase);
    float generateTriangleWave(float phase);
    float generateNoise();
    float generateLFO(float phase);
    void updateFilterEnvelope();
};

} // namespace VR_DAW 