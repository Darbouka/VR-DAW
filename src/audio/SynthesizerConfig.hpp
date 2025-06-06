#pragma once

#include <string>
#include <vector>
#include <map>

namespace VR_DAW {

struct SynthesizerConfig {
    // Allgemeine Einstellungen
    std::string defaultType;  // z.B. "subtractive", "fm", "wavetable"
    float defaultVolume;
    float defaultPan;
    bool enableEffects;
    int maxVoices;

    // Oszillator-Einstellungen
    struct OscillatorConfig {
        std::string type;  // "sine", "square", "saw", "triangle", "noise"
        float mix;
        float detune;
        float phase;
        float level;
    };
    std::vector<OscillatorConfig> oscillators;

    // Filter-Einstellungen
    struct FilterConfig {
        std::string type;  // "lowpass", "highpass", "bandpass"
        float cutoff;
        float resonance;
        float drive;
        float envelopeAmount;
        float envelopeAttack;
        float envelopeDecay;
    };
    FilterConfig filter;

    // LFO-Einstellungen
    struct LFOConfig {
        std::string waveform;  // "sine", "square", "saw", "triangle"
        float rate;
        float depth;
        std::string destination;  // "pitch", "filter", "amplitude"
    };
    std::vector<LFOConfig> lfos;

    // Hüllkurven-Einstellungen
    struct EnvelopeConfig {
        float attack;
        float decay;
        float sustain;
        float release;
    };
    EnvelopeConfig amplitudeEnvelope;
    EnvelopeConfig filterEnvelope;

    // Effekt-Einstellungen
    struct EffectConfig {
        std::string type;  // "reverb", "delay", "chorus", "distortion"
        bool enabled;
        std::map<std::string, float> parameters;
    };
    std::vector<EffectConfig> effects;

    // Standardkonstruktor mit sinnvollen Standardwerten
    SynthesizerConfig() :
        defaultType("subtractive"),
        defaultVolume(0.7f),
        defaultPan(0.0f),
        enableEffects(true),
        maxVoices(16)
    {
        // Standard-Oszillator
        OscillatorConfig osc;
        osc.type = "sine";
        osc.mix = 1.0f;
        osc.detune = 0.0f;
        osc.phase = 0.0f;
        osc.level = 1.0f;
        oscillators.push_back(osc);

        // Standard-Filter
        filter.type = "lowpass";
        filter.cutoff = 1000.0f;
        filter.resonance = 0.7f;
        filter.drive = 1.0f;
        filter.envelopeAmount = 0.5f;
        filter.envelopeAttack = 0.1f;
        filter.envelopeDecay = 0.3f;

        // Standard-LFO
        LFOConfig lfo;
        lfo.waveform = "sine";
        lfo.rate = 5.0f;
        lfo.depth = 0.1f;
        lfo.destination = "pitch";
        lfos.push_back(lfo);

        // Standard-Hüllkurven
        amplitudeEnvelope.attack = 0.01f;
        amplitudeEnvelope.decay = 0.1f;
        amplitudeEnvelope.sustain = 0.7f;
        amplitudeEnvelope.release = 0.3f;

        filterEnvelope.attack = 0.1f;
        filterEnvelope.decay = 0.2f;
        filterEnvelope.sustain = 0.5f;
        filterEnvelope.release = 0.4f;

        // Standard-Effekte
        EffectConfig reverb;
        reverb.type = "reverb";
        reverb.enabled = true;
        reverb.parameters["mix"] = 0.3f;
        reverb.parameters["time"] = 2.0f;
        reverb.parameters["damping"] = 0.5f;
        effects.push_back(reverb);
    }
};

} // namespace VR_DAW 