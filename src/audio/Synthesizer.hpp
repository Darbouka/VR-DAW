#pragma once

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <map>
#include <functional>

namespace VR_DAW {

class Synthesizer {
public:
    struct Voice {
        float frequency;
        float velocity;
        float phase;
        float amplitude;
        bool active;
        uint8_t note;
        uint8_t channel;
    };

    struct Envelope {
        float attack;
        float decay;
        float sustain;
        float release;
    };

    Synthesizer();
    virtual ~Synthesizer();

    // Synthesizer-Steuerung
    virtual void noteOn(uint8_t note, uint8_t velocity, uint8_t channel = 0);
    virtual void noteOff(uint8_t note, uint8_t velocity, uint8_t channel = 0);
    virtual void setController(uint8_t controller, uint8_t value, uint8_t channel = 0);
    virtual void setPitchBend(float value, uint8_t channel = 0);
    virtual void setModulation(float value, uint8_t channel = 0);
    virtual void setVolume(float volume);
    virtual void setPan(float pan);

    // Parameter-Einstellungen
    virtual void setEnvelope(const Envelope& envelope);
    virtual void setOscillatorType(const std::string& type);
    virtual void setFilterCutoff(float cutoff);
    virtual void setFilterResonance(float resonance);
    virtual void setLFORate(float rate);
    virtual void setLFODepth(float depth);

    // Audio-Verarbeitung
    virtual void processBlock(float* output, size_t numSamples) = 0;
    virtual void reset();

    // Status-Abfragen
    virtual bool isActive() const;
    virtual size_t getActiveVoiceCount() const;
    virtual float getCurrentVolume() const;
    virtual float getCurrentPan() const;

protected:
    // Synthesizer-Komponenten
    std::vector<Voice> voices;
    Envelope envelope;
    std::string oscillatorType;
    float filterCutoff;
    float filterResonance;
    float lfoRate;
    float lfoDepth;
    float volume;
    float pan;
    float sampleRate;
    std::atomic<bool> active;
    std::mutex mutex;

    // Hilfsfunktionen
    virtual float generateSample(const Voice& voice) = 0;
    virtual void updateEnvelope(Voice& voice);
    virtual void applyFilter(float& sample);
    virtual void applyLFO(float& sample);
    virtual void applyPan(float& leftSample, float& rightSample);
    virtual void applyVolume(float& sample);
};

} // namespace VR_DAW 