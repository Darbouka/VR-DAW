#pragma once

#include <vector>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>

namespace VR_DAW {

class DynamicsProcessor {
public:
    DynamicsProcessor();
    ~DynamicsProcessor();

    // Kompressor-Typen
    enum class CompressorType {
        Standard,
        Multiband,
        Sidechain,
        Parallel,
        Vintage,
        Modern,
        Mastering,
        Limiter,
        Gate,
        Expander
    };

    // Multiband-Konfiguration
    struct MultibandConfig {
        std::vector<float> crossoverFrequencies;
        std::vector<float> bandGains;
        std::vector<float> bandThresholds;
        std::vector<float> bandRatios;
        std::vector<float> bandAttackTimes;
        std::vector<float> bandReleaseTimes;
    };

    // Kompressor-Parameter
    struct CompressorParameters {
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float kneeWidth;
        float makeupGain;
        float mix;
        bool bypass;
        bool autoGain;
        bool softKnee;
        bool lookahead;
    };

    // Vintage-Kompressor-Parameter
    struct VintageParameters {
        float inputGain;
        float outputGain;
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float kneeWidth;
        float saturation;
        float harmonicContent;
        float transformerColor;
        float tubeWarmth;
    };

    // Mastering-Kompressor-Parameter
    struct MasteringParameters {
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float kneeWidth;
        float makeupGain;
        float stereoWidth;
        float midSideBalance;
        float harmonicEnhancement;
        float stereoCoherence;
    };

    // Limiter-Parameter
    struct LimiterParameters {
        float ceiling;
        float releaseTime;
        float lookahead;
        float ditherAmount;
        bool truePeak;
        bool oversampling;
    };

    // Gate-Parameter
    struct GateParameters {
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float holdTime;
        float range;
        bool sidechain;
        float sidechainThreshold;
    };

    // Expander-Parameter
    struct ExpanderParameters {
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float kneeWidth;
        float range;
        bool upward;
    };

    // Hauptfunktionen
    void processBlock(juce::AudioBuffer<float>& buffer);
    void setCompressorType(CompressorType type);
    void setMultibandConfig(const MultibandConfig& config);
    void setCompressorParameters(const CompressorParameters& params);
    void setVintageParameters(const VintageParameters& params);
    void setMasteringParameters(const MasteringParameters& params);
    void setLimiterParameters(const LimiterParameters& params);
    void setGateParameters(const GateParameters& params);
    void setExpanderParameters(const ExpanderParameters& params);

    // Hilfsfunktionen
    void reset();
    void prepareToPlay(double sampleRate, int blockSize);
    void releaseResources();

private:
    // Kompressor-Implementierungen
    void processStandardCompressor(juce::AudioBuffer<float>& buffer);
    void processMultibandCompressor(juce::AudioBuffer<float>& buffer);
    void processSidechainCompressor(juce::AudioBuffer<float>& buffer);
    void processParallelCompressor(juce::AudioBuffer<float>& buffer);
    void processVintageCompressor(juce::AudioBuffer<float>& buffer);
    void processModernCompressor(juce::AudioBuffer<float>& buffer);
    void processMasteringCompressor(juce::AudioBuffer<float>& buffer);
    void processLimiter(juce::AudioBuffer<float>& buffer);
    void processGate(juce::AudioBuffer<float>& buffer);
    void processExpander(juce::AudioBuffer<float>& buffer);

    // Hilfsfunktionen
    float calculateGainReduction(float inputLevel, const CompressorParameters& params);
    float calculateVintageGainReduction(float inputLevel, const VintageParameters& params);
    float calculateMasteringGainReduction(float inputLevel, const MasteringParameters& params);
    float calculateLimiterGainReduction(float inputLevel, const LimiterParameters& params);
    float calculateGateGainReduction(float inputLevel, const GateParameters& params);
    float calculateExpanderGainReduction(float inputLevel, const ExpanderParameters& params);

    // Member-Variablen
    CompressorType currentType;
    MultibandConfig multibandConfig;
    CompressorParameters compressorParams;
    VintageParameters vintageParams;
    MasteringParameters masteringParams;
    LimiterParameters limiterParams;
    GateParameters gateParams;
    ExpanderParameters expanderParams;

    // Audio-Verarbeitung
    double sampleRate;
    int blockSize;
    std::vector<float> envelopeFollower;
    std::vector<float> gainReduction;
    std::vector<std::vector<float>> bandBuffers;
    std::vector<juce::dsp::IIR::Filter<float>> crossoverFilters;
    std::vector<juce::dsp::Delay<float>> lookaheadDelays;
    juce::dsp::Oversampling<float> oversampling;
};

} // namespace VR_DAW 