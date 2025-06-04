#include "DynamicsProcessor.hpp"
#include <cmath>
#include <algorithm>

namespace VR_DAW {

DynamicsProcessor::DynamicsProcessor()
    : currentType(CompressorType::Standard)
    , sampleRate(44100.0)
    , blockSize(512)
{
    // Standard-Parameter initialisieren
    compressorParams = {
        -20.0f,  // threshold
        4.0f,    // ratio
        0.01f,   // attackTime
        0.1f,    // releaseTime
        6.0f,    // kneeWidth
        0.0f,    // makeupGain
        1.0f,    // mix
        false,   // bypass
        true,    // autoGain
        true,    // softKnee
        false    // lookahead
    };

    // Vintage-Parameter initialisieren
    vintageParams = {
        0.0f,    // inputGain
        0.0f,    // outputGain
        -20.0f,  // threshold
        4.0f,    // ratio
        0.01f,   // attackTime
        0.1f,    // releaseTime
        6.0f,    // kneeWidth
        0.5f,    // saturation
        0.3f,    // harmonicContent
        0.2f,    // transformerColor
        0.4f     // tubeWarmth
    };

    // Mastering-Parameter initialisieren
    masteringParams = {
        -20.0f,  // threshold
        2.0f,    // ratio
        0.005f,  // attackTime
        0.05f,   // releaseTime
        3.0f,    // kneeWidth
        0.0f,    // makeupGain
        1.0f,    // stereoWidth
        0.5f,    // midSideBalance
        0.2f,    // harmonicEnhancement
        0.8f     // stereoCoherence
    };

    // Limiter-Parameter initialisieren
    limiterParams = {
        0.0f,    // ceiling
        0.01f,   // releaseTime
        0.0f,    // lookahead
        0.0f,    // ditherAmount
        true,    // truePeak
        true     // oversampling
    };

    // Gate-Parameter initialisieren
    gateParams = {
        -50.0f,  // threshold
        20.0f,   // ratio
        0.001f,  // attackTime
        0.1f,    // releaseTime
        0.1f,    // holdTime
        80.0f,   // range
        false,   // sidechain
        -50.0f   // sidechainThreshold
    };

    // Expander-Parameter initialisieren
    expanderParams = {
        -50.0f,  // threshold
        2.0f,    // ratio
        0.001f,  // attackTime
        0.1f,    // releaseTime
        6.0f,    // kneeWidth
        40.0f,   // range
        false    // upward
    };
}

DynamicsProcessor::~DynamicsProcessor() {
    releaseResources();
}

void DynamicsProcessor::prepareToPlay(double newSampleRate, int newBlockSize) {
    sampleRate = newSampleRate;
    blockSize = newBlockSize;

    // Puffer initialisieren
    envelopeFollower.resize(blockSize);
    gainReduction.resize(blockSize);

    // Multiband-Filter initialisieren
    if (currentType == CompressorType::Multiband) {
        const size_t numBands = multibandConfig.crossoverFrequencies.size() + 1;
        bandBuffers.resize(numBands, std::vector<float>(blockSize));
        
        // Crossover-Filter erstellen
        crossoverFilters.clear();
        for (size_t i = 0; i < multibandConfig.crossoverFrequencies.size(); ++i) {
            juce::dsp::IIR::Coefficients<float>::Ptr coeffs = 
                juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 
                    multibandConfig.crossoverFrequencies[i]);
            crossoverFilters.push_back(juce::dsp::IIR::Filter<float>(coeffs));
        }
    }

    // Lookahead-Delays initialisieren
    if (compressorParams.lookahead) {
        const float maxLookahead = 10.0f; // ms
        const int maxDelaySamples = static_cast<int>(maxLookahead * sampleRate / 1000.0f);
        lookaheadDelays.resize(2, juce::dsp::Delay<float>(maxDelaySamples));
    }

    // Oversampling initialisieren
    if (limiterParams.oversampling) {
        oversampling.initProcessing(static_cast<size_t>(blockSize));
    }
}

void DynamicsProcessor::releaseResources() {
    envelopeFollower.clear();
    gainReduction.clear();
    bandBuffers.clear();
    crossoverFilters.clear();
    lookaheadDelays.clear();
}

void DynamicsProcessor::processBlock(juce::AudioBuffer<float>& buffer) {
    if (compressorParams.bypass) return;

    switch (currentType) {
        case CompressorType::Standard:
            processStandardCompressor(buffer);
            break;
        case CompressorType::Multiband:
            processMultibandCompressor(buffer);
            break;
        case CompressorType::Sidechain:
            processSidechainCompressor(buffer);
            break;
        case CompressorType::Parallel:
            processParallelCompressor(buffer);
            break;
        case CompressorType::Vintage:
            processVintageCompressor(buffer);
            break;
        case CompressorType::Modern:
            processModernCompressor(buffer);
            break;
        case CompressorType::Mastering:
            processMasteringCompressor(buffer);
            break;
        case CompressorType::Limiter:
            processLimiter(buffer);
            break;
        case CompressorType::Gate:
            processGate(buffer);
            break;
        case CompressorType::Expander:
            processExpander(buffer);
            break;
    }
}

void DynamicsProcessor::processStandardCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Envelope-Follower berechnen
    for (int i = 0; i < numSamples; ++i) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxLevel = std::max(maxLevel, std::abs(buffer.getSample(ch, i)));
        }
        envelopeFollower[i] = maxLevel;
    }

    // Gain-Reduction berechnen
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateGainReduction(envelopeFollower[i], compressorParams);
    }

    // Gain-Reduction anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] *= gainReduction[i];
        }
    }

    // Makeup-Gain anwenden
    if (compressorParams.autoGain) {
        const float makeupGain = std::pow(10.0f, compressorParams.makeupGain / 20.0f);
        buffer.applyGain(makeupGain);
    }
}

void DynamicsProcessor::processMultibandCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const size_t numBands = multibandConfig.crossoverFrequencies.size() + 1;

    // Signal in Bänder aufteilen
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getReadPointer(ch);
        
        // Band 1 (Low)
        std::copy(channelData, channelData + numSamples, bandBuffers[0].begin());
        crossoverFilters[0].process(juce::dsp::ProcessContextReplacing<float>(
            juce::dsp::AudioBlock<float>(bandBuffers[0].data(), 1, numSamples)));

        // Mittlere Bänder
        for (size_t band = 1; band < numBands - 1; ++band) {
            std::copy(channelData, channelData + numSamples, bandBuffers[band].begin());
            
            // Low-Pass
            crossoverFilters[band].process(juce::dsp::ProcessContextReplacing<float>(
                juce::dsp::AudioBlock<float>(bandBuffers[band].data(), 1, numSamples)));
            
            // Band-Pass durch Subtraktion
            for (int i = 0; i < numSamples; ++i) {
                bandBuffers[band][i] -= bandBuffers[band - 1][i];
            }
        }

        // Band N (High)
        std::copy(channelData, channelData + numSamples, bandBuffers[numBands - 1].begin());
        for (size_t band = 0; band < numBands - 1; ++band) {
            for (int i = 0; i < numSamples; ++i) {
                bandBuffers[numBands - 1][i] -= bandBuffers[band][i];
            }
        }
    }

    // Jedes Band komprimieren
    for (size_t band = 0; band < numBands; ++band) {
        // Envelope-Follower für dieses Band
        for (int i = 0; i < numSamples; ++i) {
            float maxLevel = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                maxLevel = std::max(maxLevel, std::abs(bandBuffers[band][i]));
            }
            envelopeFollower[i] = maxLevel;
        }

        // Gain-Reduction für dieses Band
        CompressorParameters bandParams = compressorParams;
        bandParams.threshold = multibandConfig.bandThresholds[band];
        bandParams.ratio = multibandConfig.bandRatios[band];
        bandParams.attackTime = multibandConfig.bandAttackTimes[band];
        bandParams.releaseTime = multibandConfig.bandReleaseTimes[band];

        for (int i = 0; i < numSamples; ++i) {
            gainReduction[i] = calculateGainReduction(envelopeFollower[i], bandParams);
        }

        // Gain-Reduction und Band-Gain anwenden
        for (int i = 0; i < numSamples; ++i) {
            bandBuffers[band][i] *= gainReduction[i] * multibandConfig.bandGains[band];
        }
    }

    // Bänder wieder zusammenführen
    buffer.clear();
    for (size_t band = 0; band < numBands; ++band) {
        for (int ch = 0; ch < numChannels; ++ch) {
            float* channelData = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i) {
                channelData[i] += bandBuffers[band][i];
            }
        }
    }
}

void DynamicsProcessor::processVintageCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Input-Gain anwenden
    const float inputGain = std::pow(10.0f, vintageParams.inputGain / 20.0f);
    buffer.applyGain(inputGain);

    // Envelope-Follower berechnen
    for (int i = 0; i < numSamples; ++i) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxLevel = std::max(maxLevel, std::abs(buffer.getSample(ch, i)));
        }
        envelopeFollower[i] = maxLevel;
    }

    // Gain-Reduction berechnen
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateVintageGainReduction(envelopeFollower[i], vintageParams);
    }

    // Gain-Reduction und Sättigung anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            // Gain-Reduction
            channelData[i] *= gainReduction[i];

            // Sättigung
            const float saturation = vintageParams.saturation;
            channelData[i] = std::tanh(channelData[i] * (1.0f + saturation)) / (1.0f + saturation);

            // Röhren-Wärme
            const float tubeWarmth = vintageParams.tubeWarmth;
            channelData[i] = channelData[i] * (1.0f + tubeWarmth * std::abs(channelData[i]));

            // Transformer-Färbung
            const float transformerColor = vintageParams.transformerColor;
            channelData[i] = channelData[i] + transformerColor * std::pow(channelData[i], 2);
        }
    }

    // Output-Gain anwenden
    const float outputGain = std::pow(10.0f, vintageParams.outputGain / 20.0f);
    buffer.applyGain(outputGain);
}

void DynamicsProcessor::processMasteringCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Mid-Side-Transformation
    std::vector<float> midBuffer(numSamples);
    std::vector<float> sideBuffer(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        const float left = buffer.getSample(0, i);
        const float right = buffer.getSample(1, i);
        midBuffer[i] = (left + right) * 0.5f;
        sideBuffer[i] = (left - right) * 0.5f;
    }

    // Mid-Kanal komprimieren
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateMasteringGainReduction(std::abs(midBuffer[i]), masteringParams);
        midBuffer[i] *= gainReduction[i];
    }

    // Side-Kanal komprimieren
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateMasteringGainReduction(std::abs(sideBuffer[i]), masteringParams);
        sideBuffer[i] *= gainReduction[i];
    }

    // Mid-Side-Balance anwenden
    const float midGain = masteringParams.midSideBalance;
    const float sideGain = 1.0f - masteringParams.midSideBalance;
    for (int i = 0; i < numSamples; ++i) {
        midBuffer[i] *= midGain;
        sideBuffer[i] *= sideGain;
    }

    // Zurück zu Stereo
    for (int i = 0; i < numSamples; ++i) {
        buffer.setSample(0, i, midBuffer[i] + sideBuffer[i]);
        buffer.setSample(1, i, midBuffer[i] - sideBuffer[i]);
    }

    // Stereo-Breite anwenden
    const float width = masteringParams.stereoWidth;
    for (int i = 0; i < numSamples; ++i) {
        const float left = buffer.getSample(0, i);
        const float right = buffer.getSample(1, i);
        const float mid = (left + right) * 0.5f;
        const float side = (left - right) * 0.5f;
        buffer.setSample(0, i, mid + side * width);
        buffer.setSample(1, i, mid - side * width);
    }

    // Harmonische Verstärkung
    const float harmonicEnhancement = masteringParams.harmonicEnhancement;
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] += harmonicEnhancement * std::pow(channelData[i], 2);
        }
    }

    // Makeup-Gain anwenden
    const float makeupGain = std::pow(10.0f, masteringParams.makeupGain / 20.0f);
    buffer.applyGain(makeupGain);
}

void DynamicsProcessor::processLimiter(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Oversampling aktivieren
    if (limiterParams.oversampling) {
        juce::dsp::AudioBlock<float> block(buffer);
        auto oversampledBlock = oversampling.processSamplesUp(block);
        
        // Limiter auf oversampled Daten anwenden
        for (int ch = 0; ch < numChannels; ++ch) {
            float* channelData = oversampledBlock.getChannelPointer(ch);
            for (int i = 0; i < oversampledBlock.getNumSamples(); ++i) {
                const float inputLevel = std::abs(channelData[i]);
                const float gainReduction = calculateLimiterGainReduction(inputLevel, limiterParams);
                channelData[i] *= gainReduction;
            }
        }
        
        // Zurück zu Original-Sample-Rate
        oversampling.processSamplesDown(block);
    } else {
        // Limiter auf Original-Daten anwenden
        for (int ch = 0; ch < numChannels; ++ch) {
            float* channelData = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i) {
                const float inputLevel = std::abs(channelData[i]);
                const float gainReduction = calculateLimiterGainReduction(inputLevel, limiterParams);
                channelData[i] *= gainReduction;
            }
        }
    }

    // Dither hinzufügen
    if (limiterParams.ditherAmount > 0.0f) {
        for (int ch = 0; ch < numChannels; ++ch) {
            float* channelData = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i) {
                const float dither = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
                channelData[i] += dither * limiterParams.ditherAmount;
            }
        }
    }
}

float DynamicsProcessor::calculateGainReduction(float inputLevel, const CompressorParameters& params) {
    const float threshold = std::pow(10.0f, params.threshold / 20.0f);
    const float ratio = params.ratio;
    
    if (inputLevel <= threshold) return 1.0f;
    
    float gainReduction;
    if (params.softKnee) {
        const float kneeWidth = params.kneeWidth;
        const float kneeStart = threshold - kneeWidth / 2.0f;
        const float kneeEnd = threshold + kneeWidth / 2.0f;
        
        if (inputLevel <= kneeStart) {
            gainReduction = 1.0f;
        } else if (inputLevel >= kneeEnd) {
            gainReduction = std::pow(inputLevel / threshold, 1.0f / ratio - 1.0f);
        } else {
            const float kneeFactor = (inputLevel - kneeStart) / kneeWidth;
            const float softRatio = 1.0f + (ratio - 1.0f) * kneeFactor;
            gainReduction = std::pow(inputLevel / threshold, 1.0f / softRatio - 1.0f);
        }
    } else {
        gainReduction = std::pow(inputLevel / threshold, 1.0f / ratio - 1.0f);
    }
    
    return std::max(0.0f, std::min(1.0f, gainReduction));
}

float DynamicsProcessor::calculateVintageGainReduction(float inputLevel, const VintageParameters& params) {
    const float threshold = std::pow(10.0f, params.threshold / 20.0f);
    const float ratio = params.ratio;
    
    if (inputLevel <= threshold) return 1.0f;
    
    // Vintage-Charakteristik
    float gainReduction = std::pow(inputLevel / threshold, 1.0f / ratio - 1.0f);
    
    // Harmonische Verzerrung
    const float harmonicContent = params.harmonicContent;
    gainReduction = gainReduction * (1.0f + harmonicContent * std::abs(gainReduction));
    
    return std::max(0.0f, std::min(1.0f, gainReduction));
}

float DynamicsProcessor::calculateMasteringGainReduction(float inputLevel, const MasteringParameters& params) {
    const float threshold = std::pow(10.0f, params.threshold / 20.0f);
    const float ratio = params.ratio;
    
    if (inputLevel <= threshold) return 1.0f;
    
    // Mastering-Charakteristik
    float gainReduction = std::pow(inputLevel / threshold, 1.0f / ratio - 1.0f);
    
    // Weichere Übergänge für Mastering
    gainReduction = std::pow(gainReduction, 0.5f);
    
    return std::max(0.0f, std::min(1.0f, gainReduction));
}

float DynamicsProcessor::calculateLimiterGainReduction(float inputLevel, const LimiterParameters& params) {
    const float ceiling = std::pow(10.0f, params.ceiling / 20.0f);
    
    if (inputLevel <= ceiling) return 1.0f;
    
    return ceiling / inputLevel;
}

float DynamicsProcessor::calculateGateGainReduction(float inputLevel, const GateParameters& params) {
    const float threshold = std::pow(10.0f, params.threshold / 20.0f);
    const float ratio = params.ratio;
    
    if (inputLevel >= threshold) return 1.0f;
    
    return std::pow(inputLevel / threshold, ratio - 1.0f);
}

float DynamicsProcessor::calculateExpanderGainReduction(float inputLevel, const ExpanderParameters& params) {
    const float threshold = std::pow(10.0f, params.threshold / 20.0f);
    const float ratio = params.ratio;
    
    if (params.upward) {
        if (inputLevel >= threshold) return 1.0f;
        return std::pow(inputLevel / threshold, 1.0f - ratio);
    } else {
        if (inputLevel <= threshold) return 1.0f;
        return std::pow(inputLevel / threshold, ratio - 1.0f);
    }
}

void DynamicsProcessor::processSidechainCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Sidechain-Signal verarbeiten (Inspiriert von Waves C1)
    for (int i = 0; i < numSamples; ++i) {
        float sidechainLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            sidechainLevel = std::max(sidechainLevel, std::abs(sidechainBuffer.getSample(ch, i)));
        }
        envelopeFollower[i] = sidechainLevel;
    }

    // Gain-Reduction mit Ducking (Inspiriert von FabFilter Pro-C2)
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateGainReduction(envelopeFollower[i], compressorParams);
        
        // Ducking-Kurve (Inspiriert von SSL G-Series)
        const float duckingCurve = std::pow(gainReduction[i], 1.5f);
        gainReduction[i] = 1.0f - (1.0f - duckingCurve) * compressorParams.mix;
    }

    // Gain-Reduction anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] *= gainReduction[i];
        }
    }
}

void DynamicsProcessor::processParallelCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Original-Signal speichern (Inspiriert von UAD 1176)
    juce::AudioBuffer<float> originalBuffer;
    originalBuffer.makeCopyOf(buffer);

    // Komprimiertes Signal erstellen
    processStandardCompressor(buffer);

    // Parallel-Mix (Inspiriert von Empirical Labs Distressor)
    const float parallelMix = compressorParams.mix;
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        const float* originalData = originalBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] = originalData[i] * (1.0f - parallelMix) + 
                            channelData[i] * parallelMix;
        }
    }
}

void DynamicsProcessor::processModernCompressor(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Envelope-Follower mit moderner Charakteristik (Inspiriert von Cytomic The Glue)
    for (int i = 0; i < numSamples; ++i) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxLevel = std::max(maxLevel, std::abs(buffer.getSample(ch, i)));
        }
        
        // RMS-basierte Hüllkurve
        envelopeFollower[i] = std::sqrt(envelopeFollower[i] * envelopeFollower[i] * 0.95f + 
                                      maxLevel * maxLevel * 0.05f);
    }

    // Gain-Reduction mit moderner Charakteristik (Inspiriert von Softube FET)
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateGainReduction(envelopeFollower[i], compressorParams);
        
        // Moderne Kompressionskurve
        const float modernCurve = std::pow(gainReduction[i], 0.7f);
        gainReduction[i] = 1.0f - (1.0f - modernCurve) * compressorParams.mix;
    }

    // Gain-Reduction anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] *= gainReduction[i];
        }
    }
}

void DynamicsProcessor::processGate(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Envelope-Follower (Inspiriert von Drawmer DS201)
    for (int i = 0; i < numSamples; ++i) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxLevel = std::max(maxLevel, std::abs(buffer.getSample(ch, i)));
        }
        envelopeFollower[i] = maxLevel;
    }

    // Gate-Logik (Inspiriert von SSL G-Series)
    float holdCounter = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateGateGainReduction(envelopeFollower[i], gateParams);
        
        // Hold-Time implementieren
        if (gainReduction[i] < 1.0f) {
            holdCounter = gateParams.holdTime * sampleRate;
        } else if (holdCounter > 0.0f) {
            holdCounter -= 1.0f;
            gainReduction[i] = 1.0f;
        }
    }

    // Gain-Reduction anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] *= gainReduction[i];
        }
    }
}

void DynamicsProcessor::processExpander(juce::AudioBuffer<float>& buffer) {
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Envelope-Follower (Inspiriert von SPL Transient Designer)
    for (int i = 0; i < numSamples; ++i) {
        float maxLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            maxLevel = std::max(maxLevel, std::abs(buffer.getSample(ch, i)));
        }
        envelopeFollower[i] = maxLevel;
    }

    // Expansion-Logik (Inspiriert von BSS DPR-901)
    for (int i = 0; i < numSamples; ++i) {
        gainReduction[i] = calculateExpanderGainReduction(envelopeFollower[i], expanderParams);
        
        // Range-Kontrolle
        const float range = expanderParams.range;
        gainReduction[i] = 1.0f + (gainReduction[i] - 1.0f) * range;
    }

    // Gain-Reduction anwenden
    for (int ch = 0; ch < numChannels; ++ch) {
        float* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            channelData[i] *= gainReduction[i];
        }
    }
}

} // namespace VR_DAW 