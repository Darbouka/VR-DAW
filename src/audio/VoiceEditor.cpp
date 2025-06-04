#include "VoiceEditor.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace VR_DAW {

VoiceEditor::VoiceEditor() {
    initialize();
}

VoiceEditor::~VoiceEditor() {
    shutdown();
}

void VoiceEditor::initialize() {
    // Audio-Prozessoren initialisieren
    noiseReducer = std::make_unique<NoiseReducer>();
    dynamicCompressor = std::make_unique<DynamicCompressor>();
    spectralEnhancer = std::make_unique<SpectralEnhancer>();
    
    // Standard-Parameter setzen
    currentParams = {
        true,   // noiseReduction
        true,   // dynamicCompression
        true,   // adaptiveEQ
        true,   // spectralEnhancement
        true,   // phaseCorrection
        true,   // stereoEnhancement
        true,   // transientEnhancement
        true    // harmonicEnhancement
    };
}

void VoiceEditor::shutdown() {
    noiseReducer.reset();
    dynamicCompressor.reset();
    spectralEnhancer.reset();
}

void VoiceEditor::process(juce::AudioBuffer<float>& buffer) {
    // Rauschunterdrückung
    if (currentParams.noiseReduction && noiseReducer) {
        noiseReducer->process(buffer);
    }
    
    // Dynamische Kompression
    if (currentParams.dynamicCompression && dynamicCompressor) {
        dynamicCompressor->process(buffer);
    }
    
    // Spektrale Verbesserung
    if (currentParams.spectralEnhancement && spectralEnhancer) {
        spectralEnhancer->process(buffer);
    }
    
    // Phasenkorrektur
    if (currentParams.phaseCorrection) {
        applyPhaseCorrection(buffer);
    }
    
    // Stereo-Verbesserung
    if (currentParams.stereoEnhancement) {
        applyStereoEnhancement(buffer);
    }
    
    // Transienten-Verbesserung
    if (currentParams.transientEnhancement) {
        applyTransientEnhancement(buffer);
    }
    
    // Harmonische Verbesserung
    if (currentParams.harmonicEnhancement) {
        applyHarmonicEnhancement(buffer);
    }
}

void VoiceEditor::setAudioProcessingOptimizations(const AudioProcessingOptimizations& optimizations) {
    currentParams = optimizations;
    
    // Aktualisiere Prozessoren
    if (noiseReducer) {
        noiseReducer->setThreshold(optimizations.noiseReductionThreshold);
        noiseReducer->setBands(optimizations.noiseReductionBands);
        noiseReducer->setAdaptiveProcessing(optimizations.adaptiveNoiseReduction);
    }
    
    if (dynamicCompressor) {
        dynamicCompressor->setThreshold(optimizations.compressionThreshold);
        dynamicCompressor->setRatio(optimizations.compressionRatio);
        dynamicCompressor->setAttack(optimizations.compressionAttack);
        dynamicCompressor->setRelease(optimizations.compressionRelease);
    }
    
    if (spectralEnhancer) {
        spectralEnhancer->setEnhancementAmount(optimizations.spectralEnhancementAmount);
        spectralEnhancer->setFFTSize(optimizations.spectralEnhancementFFTSize);
        spectralEnhancer->setAdaptiveProcessing(optimizations.adaptiveSpectralEnhancement);
    }
}

void VoiceEditor::setNoiseReduction(bool enable) {
    currentParams.noiseReduction = enable;
    if (noiseReducer) {
        noiseReducer->setEnabled(enable);
    }
}

void VoiceEditor::setDynamicCompression(bool enable) {
    currentParams.dynamicCompression = enable;
    if (dynamicCompressor) {
        dynamicCompressor->setEnabled(enable);
    }
}

void VoiceEditor::setSpectralEnhancement(bool enable) {
    currentParams.spectralEnhancement = enable;
    if (spectralEnhancer) {
        spectralEnhancer->setEnabled(enable);
    }
}

void VoiceEditor::setPhaseCorrection(bool enable) {
    currentParams.phaseCorrection = enable;
}

void VoiceEditor::setStereoEnhancement(bool enable) {
    currentParams.stereoEnhancement = enable;
}

void VoiceEditor::setTransientEnhancement(bool enable) {
    currentParams.transientEnhancement = enable;
}

void VoiceEditor::setHarmonicEnhancement(bool enable) {
    currentParams.harmonicEnhancement = enable;
}

void VoiceEditor::applyPhaseCorrection(juce::AudioBuffer<float>& buffer) {
    // FFT für Phasenanalyse
    juce::dsp::FFT fft(11);  // 2048 Punkte
    std::vector<float> fftData(2048);
    
    // Für jeden Kanal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        // FFT durchführen
        for (int i = 0; i < 2048; ++i) {
            fftData[i] = buffer.getSample(channel, i);
        }
        fft.performRealOnlyForwardTransform(fftData.data());
        
        // Phasenkorrektur
        for (int i = 0; i < 1024; ++i) {
            float magnitude = std::abs(fftData[i]);
            float phase = std::arg(fftData[i]);
            
            // Phasenkorrektur anwenden
            phase = std::atan2(std::sin(phase), std::cos(phase));
            
            // Magnitude beibehalten
            fftData[i] = magnitude * std::complex<float>(std::cos(phase), std::sin(phase));
        }
        
        // Inverse FFT
        fft.performRealOnlyInverseTransform(fftData.data());
        
        // Ergebnisse zurück in den Buffer schreiben
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            buffer.setSample(channel, i, fftData[i]);
        }
    }
}

void VoiceEditor::applyStereoEnhancement(juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 2) return;
    
    // Stereo-Verbesserung
    float width = 1.5f;  // Stereo-Breite
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float left = buffer.getSample(0, sample);
        float right = buffer.getSample(1, sample);
        
        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;
        
        // Side-Signal verstärken
        side *= width;
        
        // Zurück zu Stereo
        buffer.setSample(0, sample, mid + side);
        buffer.setSample(1, sample, mid - side);
    }
}

void VoiceEditor::applyTransientEnhancement(juce::AudioBuffer<float>& buffer) {
    // Transienten-Erkennung und -Verstärkung
    float attackThreshold = 0.1f;
    float releaseTime = 0.01f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float envelope = 0.0f;
        float previousSample = 0.0f;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float currentSample = buffer.getSample(channel, sample);
            float delta = currentSample - previousSample;
            
            // Transienten-Erkennung
            if (std::abs(delta) > attackThreshold) {
                envelope = 1.0f;
            } else {
                envelope = envelope * (1.0f - releaseTime);
            }
            
            // Transienten-Verstärkung
            float gain = 1.0f + envelope * 0.5f;
            buffer.setSample(channel, sample, currentSample * gain);
            
            previousSample = currentSample;
        }
    }
}

void VoiceEditor::applyHarmonicEnhancement(juce::AudioBuffer<float>& buffer) {
    // FFT für harmonische Analyse
    juce::dsp::FFT fft(11);  // 2048 Punkte
    std::vector<float> fftData(2048);
    
    // Für jeden Kanal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        // FFT durchführen
        for (int i = 0; i < 2048; ++i) {
            fftData[i] = buffer.getSample(channel, i);
        }
        fft.performRealOnlyForwardTransform(fftData.data());
        
        // Harmonische Verbesserung
        for (int i = 0; i < 1024; ++i) {
            float magnitude = std::abs(fftData[i]);
            float phase = std::arg(fftData[i]);
            
            // Harmonische Verstärkung
            float harmonicGain = 1.0f + 0.2f * std::sin(2.0f * M_PI * i / 1024.0f);
            magnitude *= harmonicGain;
            
            // Phase beibehalten
            fftData[i] = magnitude * std::complex<float>(std::cos(phase), std::sin(phase));
        }
        
        // Inverse FFT
        fft.performRealOnlyInverseTransform(fftData.data());
        
        // Ergebnisse zurück in den Buffer schreiben
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            buffer.setSample(channel, i, fftData[i]);
        }
    }
}

void VoiceEditor::processBlock(juce::AudioBuffer<float>& buffer) {
    // DSP-Verarbeitung
    if (dspProcessing.adaptiveProcessing && adaptiveProcessor) {
        adaptiveProcessor->process(buffer);
    }
    
    if (dspProcessing.spectralEnhancement && spectralEnhancer) {
        spectralEnhancer->process(buffer);
    }
    
    if (dspProcessing.phaseCorrection && phaseCorrector) {
        phaseCorrector->process(buffer);
    }
    
    if (dspProcessing.stereoEnhancement && stereoEnhancer) {
        stereoEnhancer->process(buffer);
    }
    
    if (dspProcessing.transientEnhancement && transientEnhancer) {
        transientEnhancer->process(buffer);
    }
    
    if (dspProcessing.harmonicEnhancement && harmonicEnhancer) {
        harmonicEnhancer->process(buffer);
    }
    
    if (dspProcessing.noiseReduction && noiseReducer) {
        noiseReducer->process(buffer);
    }
    
    if (dspProcessing.dynamicCompression && dynamicCompressor) {
        dynamicCompressor->process(buffer);
    }
    
    // KI-Verarbeitung
    if (aiProcessing.autoTune || aiProcessing.pitchCorrection) {
        if (aiVoiceProcessor) {
            aiVoiceProcessor->process(buffer);
        }
    }
    
    if (aiProcessing.styleTransfer && aiStyleTransfer) {
        aiStyleTransfer->process(buffer);
    }
    
    if (aiProcessing.emotionAnalysis && aiEmotionAnalyzer) {
        aiEmotionAnalyzer->analyze(buffer);
    }
    
    if (aiProcessing.genreAdaptation && aiGenreAdapter) {
        aiGenreAdapter->process(buffer);
    }
    
    // Plugin-Verarbeitung
    if (pluginManager) {
        pluginManager->processBlock(buffer);
    }
    
    // Bestehende Verarbeitung
    if (parameters.vocoderEnabled) {
        processVocoder(buffer);
    }
    
    if (parameters.reverbEnabled) {
        processReverb(buffer);
    }
    
    if (parameters.delayEnabled) {
        processDelay(buffer);
    }
    
    if (parameters.chorusEnabled) {
        processChorus(buffer);
    }
    
    if (parameters.compressorEnabled) {
        processCompressor(buffer);
    }
    
    if (parameters.pitchShiftEnabled) {
        processPitchShift(buffer);
    }
    
    if (parameters.formantShiftEnabled) {
        processFormantShift(buffer);
    }
    
    if (parameters.harmonizationEnabled) {
        processHarmonization(buffer);
    }
    
    // Analyse aktualisieren
    updateAnalysisResults(buffer);
}

void VoiceEditor::processFile(const std::string& inputPath, const std::string& outputPath) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(juce::File(inputPath)));
    if (!reader) return;
    
    juce::AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
    
    processBlock(buffer);
    
    std::unique_ptr<juce::AudioFormatWriter> writer(formatManager.createWriterFor(
        juce::File(outputPath),
        reader->sampleRate,
        reader->numChannels,
        16,
        {},
        0));
    
    if (writer) {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
}

void VoiceEditor::processVocoder(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    vocoder->setModulationRate(parameters.vocoderModulation);
    vocoder->setPitch(parameters.vocoderPitch);
    vocoder->setFormant(parameters.vocoderFormant);
    
    vocoder->process(context);
}

void VoiceEditor::processReverb(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    reverb->setParameters({
        parameters.reverbSize,
        parameters.reverbDamping,
        parameters.reverbAmount,
        0.0f,
        0.0f,
        0.0f,
        0.0f
    });
    
    reverb->process(context);
}

void VoiceEditor::processDelay(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    delay->setDelay(parameters.delayTime);
    delay->setFeedback(parameters.delayFeedback);
    
    delay->process(context);
}

void VoiceEditor::processChorus(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    chorus->setRate(parameters.chorusRate);
    chorus->setDepth(parameters.chorusDepth);
    chorus->setMix(parameters.chorusMix);
    
    chorus->process(context);
}

void VoiceEditor::processCompressor(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    compressor->setThreshold(parameters.compressorThreshold);
    compressor->setRatio(parameters.compressorRatio);
    compressor->setAttack(parameters.compressorAttack);
    compressor->setRelease(parameters.compressorRelease);
    
    compressor->process(context);
}

void VoiceEditor::processPitchShift(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    pitchShifter->setPitchShift(parameters.pitchShiftAmount);
    pitchShifter->setFormantPreservation(parameters.pitchShiftFormantPreservation);
    
    pitchShifter->process(context);
}

void VoiceEditor::processFormantShift(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    formantShifter->setFormantShift(parameters.formantShiftAmount);
    formantShifter->setFormantScale(parameters.formantScale);
    
    formantShifter->process(context);
}

void VoiceEditor::processHarmonization(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    harmonizer->setKey(parameters.harmonizationKey);
    harmonizer->setScale(parameters.harmonizationScale);
    harmonizer->setNumVoices(parameters.harmonizationVoices);
    
    harmonizer->process(context);
}

void VoiceEditor::analyzeInput(const juce::AudioBuffer<float>& buffer) {
    // Pegel-Analyse
    results.inputLevel = buffer.getMagnitude(0, buffer.getNumSamples());
    
    // FFT-Analyse
    std::vector<float> fftData(1024);
    buffer.copyFrom(0, 0, fftData.data(), fftData.size());
    
    fft->performRealOnlyForwardTransform(fftData.data());
    
    // Pitch-Erkennung
    float maxMagnitude = 0.0f;
    int maxBin = 0;
    for (int i = 0; i < fftData.size() / 2; ++i) {
        float magnitude = std::abs(fftData[i]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            maxBin = i;
        }
    }
    results.pitch = maxBin * 44100.0f / fftData.size();
    
    // Formant-Analyse
    for (int i = 0; i < 3; ++i) {
        float formantFreq = 500.0f * (i + 1);
        int formantBin = formantFreq * fftData.size() / 44100.0f;
        results.formants[i] = std::abs(fftData[formantBin]);
    }
    
    // Harmonische Analyse
    for (int i = 0; i < 10; ++i) {
        int harmonicBin = maxBin * (i + 1);
        if (harmonicBin < fftData.size() / 2) {
            results.harmonics[i] = std::abs(fftData[harmonicBin]);
        }
    }
    
    // Rauschpegel
    float noiseSum = 0.0f;
    int noiseCount = 0;
    for (int i = 0; i < fftData.size() / 2; ++i) {
        if (i != maxBin && i != maxBin * 2 && i != maxBin * 3) {
            noiseSum += std::abs(fftData[i]);
            noiseCount++;
        }
    }
    results.noiseLevel = noiseSum / noiseCount;
}

void VoiceEditor::analyzeOutput(const juce::AudioBuffer<float>& buffer) {
    results.outputLevel = buffer.getMagnitude(0, buffer.getNumSamples());
}

// Setter-Methoden
void VoiceEditor::setVocoderEnabled(bool enable) {
    parameters.vocoderEnabled = enable;
}

void VoiceEditor::setVocoderMode(const std::string& mode) {
    parameters.vocoderMode = mode;
}

void VoiceEditor::setVocoderPitch(float pitch) {
    parameters.vocoderPitch = pitch;
}

void VoiceEditor::setVocoderFormant(float formant) {
    parameters.vocoderFormant = formant;
}

void VoiceEditor::setVocoderModulation(float modulation) {
    parameters.vocoderModulation = modulation;
}

void VoiceEditor::setReverbEnabled(bool enable) {
    parameters.reverbEnabled = enable;
}

void VoiceEditor::setReverbAmount(float amount) {
    parameters.reverbAmount = amount;
}

void VoiceEditor::setReverbSize(float size) {
    parameters.reverbSize = size;
}

void VoiceEditor::setReverbDamping(float damping) {
    parameters.reverbDamping = damping;
}

void VoiceEditor::setDelayEnabled(bool enable) {
    parameters.delayEnabled = enable;
}

void VoiceEditor::setDelayTime(float time) {
    parameters.delayTime = time;
}

void VoiceEditor::setDelayFeedback(float feedback) {
    parameters.delayFeedback = feedback;
}

void VoiceEditor::setDelayMix(float mix) {
    parameters.delayMix = mix;
}

void VoiceEditor::setChorusEnabled(bool enable) {
    parameters.chorusEnabled = enable;
}

void VoiceEditor::setChorusRate(float rate) {
    parameters.chorusRate = rate;
}

void VoiceEditor::setChorusDepth(float depth) {
    parameters.chorusDepth = depth;
}

void VoiceEditor::setChorusMix(float mix) {
    parameters.chorusMix = mix;
}

void VoiceEditor::setCompressorEnabled(bool enable) {
    parameters.compressorEnabled = enable;
}

void VoiceEditor::setCompressorThreshold(float threshold) {
    parameters.compressorThreshold = threshold;
}

void VoiceEditor::setCompressorRatio(float ratio) {
    parameters.compressorRatio = ratio;
}

void VoiceEditor::setCompressorAttack(float attack) {
    parameters.compressorAttack = attack;
}

void VoiceEditor::setCompressorRelease(float release) {
    parameters.compressorRelease = release;
}

void VoiceEditor::setPitchShiftEnabled(bool enable) {
    parameters.pitchShiftEnabled = enable;
}

void VoiceEditor::setPitchShiftAmount(float semitones) {
    parameters.pitchShiftAmount = semitones;
}

void VoiceEditor::setPitchShiftFormantPreservation(bool preserve) {
    parameters.pitchShiftFormantPreservation = preserve;
}

void VoiceEditor::setFormantShiftEnabled(bool enable) {
    parameters.formantShiftEnabled = enable;
}

void VoiceEditor::setFormantShiftAmount(float amount) {
    parameters.formantShiftAmount = amount;
}

void VoiceEditor::setFormantScale(float scale) {
    parameters.formantScale = scale;
}

void VoiceEditor::setHarmonizationEnabled(bool enable) {
    parameters.harmonizationEnabled = enable;
}

void VoiceEditor::setHarmonizationKey(const std::string& key) {
    parameters.harmonizationKey = key;
}

void VoiceEditor::setHarmonizationScale(const std::string& scale) {
    parameters.harmonizationScale = scale;
}

void VoiceEditor::setHarmonizationVoices(int numVoices) {
    parameters.harmonizationVoices = numVoices;
}

void VoiceEditor::setParameter(const std::string& name, float value) {
    if (name == "vocoder_enabled") parameters.vocoderEnabled = value > 0.5f;
    else if (name == "vocoder_pitch") parameters.vocoderPitch = value;
    else if (name == "vocoder_formant") parameters.vocoderFormant = value;
    else if (name == "vocoder_modulation") parameters.vocoderModulation = value;
    else if (name == "reverb_enabled") parameters.reverbEnabled = value > 0.5f;
    else if (name == "reverb_amount") parameters.reverbAmount = value;
    else if (name == "reverb_size") parameters.reverbSize = value;
    else if (name == "reverb_damping") parameters.reverbDamping = value;
    else if (name == "delay_enabled") parameters.delayEnabled = value > 0.5f;
    else if (name == "delay_time") parameters.delayTime = value;
    else if (name == "delay_feedback") parameters.delayFeedback = value;
    else if (name == "delay_mix") parameters.delayMix = value;
    else if (name == "chorus_enabled") parameters.chorusEnabled = value > 0.5f;
    else if (name == "chorus_rate") parameters.chorusRate = value;
    else if (name == "chorus_depth") parameters.chorusDepth = value;
    else if (name == "chorus_mix") parameters.chorusMix = value;
    else if (name == "compressor_enabled") parameters.compressorEnabled = value > 0.5f;
    else if (name == "compressor_threshold") parameters.compressorThreshold = value;
    else if (name == "compressor_ratio") parameters.compressorRatio = value;
    else if (name == "compressor_attack") parameters.compressorAttack = value;
    else if (name == "compressor_release") parameters.compressorRelease = value;
    else if (name == "pitch_shift_enabled") parameters.pitchShiftEnabled = value > 0.5f;
    else if (name == "pitch_shift_amount") parameters.pitchShiftAmount = value;
    else if (name == "pitch_shift_formant_preservation") parameters.pitchShiftFormantPreservation = value > 0.5f;
    else if (name == "formant_shift_enabled") parameters.formantShiftEnabled = value > 0.5f;
    else if (name == "formant_shift_amount") parameters.formantShiftAmount = value;
    else if (name == "formant_scale") parameters.formantScale = value;
    else if (name == "harmonization_enabled") parameters.harmonizationEnabled = value > 0.5f;
    else if (name == "harmonization_voices") parameters.harmonizationVoices = static_cast<int>(value);
}

float VoiceEditor::getParameter(const std::string& name) const {
    if (name == "vocoder_enabled") return parameters.vocoderEnabled ? 1.0f : 0.0f;
    else if (name == "vocoder_pitch") return parameters.vocoderPitch;
    else if (name == "vocoder_formant") return parameters.vocoderFormant;
    else if (name == "vocoder_modulation") return parameters.vocoderModulation;
    else if (name == "reverb_enabled") return parameters.reverbEnabled ? 1.0f : 0.0f;
    else if (name == "reverb_amount") return parameters.reverbAmount;
    else if (name == "reverb_size") return parameters.reverbSize;
    else if (name == "reverb_damping") return parameters.reverbDamping;
    else if (name == "delay_enabled") return parameters.delayEnabled ? 1.0f : 0.0f;
    else if (name == "delay_time") return parameters.delayTime;
    else if (name == "delay_feedback") return parameters.delayFeedback;
    else if (name == "delay_mix") return parameters.delayMix;
    else if (name == "chorus_enabled") return parameters.chorusEnabled ? 1.0f : 0.0f;
    else if (name == "chorus_rate") return parameters.chorusRate;
    else if (name == "chorus_depth") return parameters.chorusDepth;
    else if (name == "chorus_mix") return parameters.chorusMix;
    else if (name == "compressor_enabled") return parameters.compressorEnabled ? 1.0f : 0.0f;
    else if (name == "compressor_threshold") return parameters.compressorThreshold;
    else if (name == "compressor_ratio") return parameters.compressorRatio;
    else if (name == "compressor_attack") return parameters.compressorAttack;
    else if (name == "compressor_release") return parameters.compressorRelease;
    else if (name == "pitch_shift_enabled") return parameters.pitchShiftEnabled ? 1.0f : 0.0f;
    else if (name == "pitch_shift_amount") return parameters.pitchShiftAmount;
    else if (name == "pitch_shift_formant_preservation") return parameters.pitchShiftFormantPreservation ? 1.0f : 0.0f;
    else if (name == "formant_shift_enabled") return parameters.formantShiftEnabled ? 1.0f : 0.0f;
    else if (name == "formant_shift_amount") return parameters.formantShiftAmount;
    else if (name == "formant_scale") return parameters.formantScale;
    else if (name == "harmonization_enabled") return parameters.harmonizationEnabled ? 1.0f : 0.0f;
    else if (name == "harmonization_voices") return static_cast<float>(parameters.harmonizationVoices);
    return 0.0f;
}

void VoiceEditor::savePreset(const std::string& name) {
    // TODO: Implementierung des Preset-Speicherns
}

void VoiceEditor::loadPreset(const std::string& name) {
    // TODO: Implementierung des Preset-Ladens
}

void VoiceEditor::deletePreset(const std::string& name) {
    // TODO: Implementierung des Preset-Löschens
}

std::vector<std::string> VoiceEditor::getAvailablePresets() {
    // TODO: Implementierung der Preset-Liste
    return {};
}

void VoiceEditor::setDSPProcessing(const DSPProcessing& processing) {
    dspProcessing = processing;
    
    if (dspProcessing.adaptiveProcessing) {
        adaptiveProcessor = std::make_unique<juce::dsp::AdaptiveProcessor>();
        adaptiveProcessor->initialize();
    }
    
    if (dspProcessing.spectralEnhancement) {
        spectralEnhancer = std::make_unique<juce::dsp::SpectralEnhancer>();
        spectralEnhancer->initialize();
    }
    
    if (dspProcessing.phaseCorrection) {
        phaseCorrector = std::make_unique<juce::dsp::PhaseCorrector>();
        phaseCorrector->initialize();
    }
    
    if (dspProcessing.stereoEnhancement) {
        stereoEnhancer = std::make_unique<juce::dsp::StereoEnhancer>();
        stereoEnhancer->initialize();
    }
    
    if (dspProcessing.transientEnhancement) {
        transientEnhancer = std::make_unique<juce::dsp::TransientEnhancer>();
        transientEnhancer->initialize();
    }
    
    if (dspProcessing.harmonicEnhancement) {
        harmonicEnhancer = std::make_unique<juce::dsp::HarmonicEnhancer>();
        harmonicEnhancer->initialize();
    }
    
    if (dspProcessing.noiseReduction) {
        noiseReducer = std::make_unique<juce::dsp::NoiseReducer>();
        noiseReducer->initialize();
    }
    
    if (dspProcessing.dynamicCompression) {
        dynamicCompressor = std::make_unique<juce::dsp::DynamicCompressor>();
        dynamicCompressor->initialize();
    }
}

void VoiceEditor::setAIProcessing(const AIProcessing& processing) {
    aiProcessing = processing;
    
    if (aiProcessing.autoTune || aiProcessing.pitchCorrection) {
        aiVoiceProcessor = std::make_unique<AIVoiceProcessor>();
        aiVoiceProcessor->initialize();
    }
    
    if (aiProcessing.styleTransfer) {
        aiStyleTransfer = std::make_unique<AIStyleTransfer>();
        aiStyleTransfer->initialize();
    }
    
    if (aiProcessing.emotionAnalysis) {
        aiEmotionAnalyzer = std::make_unique<AIEmotionAnalyzer>();
        aiEmotionAnalyzer->initialize();
    }
    
    if (aiProcessing.genreAdaptation) {
        aiGenreAdapter = std::make_unique<AIGenreAdapter>();
        aiGenreAdapter->initialize();
    }
}

void VoiceEditor::setPerformanceOptimizations(const PerformanceOptimizations& optimizations) {
    performanceOptimizations = optimizations;
    
    // GPU-Beschleunigung konfigurieren
    if (performanceOptimizations.gpuAcceleration) {
        juce::dsp::GPUAcceleration::enable();
    }
    
    // SIMD-Optimierungen aktivieren
    if (performanceOptimizations.simdEnabled) {
        juce::dsp::SIMDOptimizations::enable();
    }
    
    // Thread-Pool konfigurieren
    if (performanceOptimizations.parallelProcessing) {
        juce::dsp::ThreadPool::setThreadCount(performanceOptimizations.threadPoolSize);
    }
    
    // Buffer-Größe und Sample-Rate setzen
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = performanceOptimizations.sampleRate;
    spec.maximumBlockSize = performanceOptimizations.bufferSize;
    spec.numChannels = 2;
    
    // DSP-Komponenten neu initialisieren
    initializeDSP();
}

void VoiceEditor::setPluginSupport(const PluginSupport& support) {
    pluginSupport = support;
    
    // Plugin-Manager initialisieren
    pluginManager = std::make_unique<PluginManager>();
    pluginManager->initialize();
    
    // Plugin-Status-Manager initialisieren
    pluginStateManager = std::make_unique<PluginStateManager>();
    pluginStateManager->initialize();
    
    // Plugin-Automation initialisieren
    pluginAutomation = std::make_unique<PluginAutomation>();
    pluginAutomation->initialize();
    
    // Plugin-Formate aktivieren
    if (pluginSupport.vst3Enabled) {
        pluginManager->enableVST3();
    }
    
    if (pluginSupport.auEnabled) {
        pluginManager->enableAU();
    }
    
    if (pluginSupport.aaxEnabled) {
        pluginManager->enableAAX();
    }
    
    if (pluginSupport.nativePlugins) {
        pluginManager->enableNativePlugins();
    }
    
    if (pluginSupport.customPlugins) {
        pluginManager->enableCustomPlugins();
    }
}

DSPProcessing VoiceEditor::getDSPProcessing() const {
    return dspProcessing;
}

AIProcessing VoiceEditor::getAIProcessing() const {
    return aiProcessing;
}

PerformanceOptimizations VoiceEditor::getPerformanceOptimizations() const {
    return performanceOptimizations;
}

PluginSupport VoiceEditor::getPluginSupport() const {
    return pluginSupport;
}

void VoiceEditor::initializeDSP() {
    // DSP-Komponenten initialisieren
    fft = std::make_unique<juce::dsp::FFT>(10); // 1024 Punkte
    
    if (parameters.vocoderEnabled) {
        vocoder = std::make_unique<juce::dsp::Vocoder>();
        vocoder->initialize();
    }
    
    if (parameters.reverbEnabled) {
        reverb = std::make_unique<juce::dsp::Reverb>();
        reverb->initialize();
    }
    
    if (parameters.delayEnabled) {
        delay = std::make_unique<juce::dsp::Delay<float>>();
        delay->initialize();
    }
    
    if (parameters.chorusEnabled) {
        chorus = std::make_unique<juce::dsp::Chorus<float>>();
        chorus->initialize();
    }
    
    if (parameters.compressorEnabled) {
        compressor = std::make_unique<juce::dsp::Compressor<float>>();
        compressor->initialize();
    }
    
    if (parameters.pitchShiftEnabled) {
        pitchShifter = std::make_unique<juce::dsp::PitchShifter<float>>();
        pitchShifter->initialize();
    }
    
    if (parameters.formantShiftEnabled) {
        formantShifter = std::make_unique<juce::dsp::FormantShifter>();
        formantShifter->initialize();
    }
    
    if (parameters.harmonizationEnabled) {
        harmonizer = std::make_unique<juce::dsp::Harmonizer>();
        harmonizer->initialize();
    }
    
    // Performance-Optimierungen anwenden
    if (performanceOptimizations.gpuAcceleration) {
        juce::dsp::GPUAcceleration::enable();
    }
    
    if (performanceOptimizations.simdEnabled) {
        juce::dsp::SIMDOptimizations::enable();
    }
    
    if (performanceOptimizations.parallelProcessing) {
        juce::dsp::ThreadPool::setThreadCount(performanceOptimizations.threadPoolSize);
    }
}

void VoiceEditor::updateAnalysisResults(const juce::AudioBuffer<float>& buffer) {
    analyzeInput(buffer);
    analyzeOutput(buffer);
}

void VoiceEditor::setMusicalEnhancement(const MusicalEnhancement& enhancement) {
    musicalEnhancement = enhancement;
    
    if (musicalEnhancement.harmonicEnrichment) {
        harmonicEnricher = std::make_unique<juce::dsp::HarmonicEnricher>();
        harmonicEnricher->initialize();
    }
    
    if (musicalEnhancement.dynamicExpression) {
        dynamicExpressor = std::make_unique<juce::dsp::DynamicExpressor>();
        dynamicExpressor->initialize();
    }
    
    if (musicalEnhancement.timbreControl) {
        timbreController = std::make_unique<juce::dsp::TimbreController>();
        timbreController->initialize();
    }
    
    if (musicalEnhancement.resonanceControl) {
        resonanceController = std::make_unique<juce::dsp::ResonanceController>();
        resonanceController->initialize();
    }
    
    if (musicalEnhancement.articulationControl) {
        articulationController = std::make_unique<juce::dsp::ArticulationController>();
        articulationController->initialize();
    }
    
    if (musicalEnhancement.vibratoControl) {
        vibratoController = std::make_unique<juce::dsp::VibratoController>();
        vibratoController->initialize();
    }
    
    if (musicalEnhancement.tremoloControl) {
        tremoloController = std::make_unique<juce::dsp::TremoloController>();
        tremoloController->initialize();
    }
    
    if (musicalEnhancement.portamentoControl) {
        portamentoController = std::make_unique<juce::dsp::PortamentoController>();
        portamentoController->initialize();
    }
    
    if (musicalEnhancement.legatoControl) {
        legatoController = std::make_unique<juce::dsp::LegatoController>();
        legatoController->initialize();
    }
    
    if (musicalEnhancement.staccatoControl) {
        staccatoController = std::make_unique<juce::dsp::StaccatoController>();
        staccatoController->initialize();
    }
}

void VoiceEditor::setVisualEnhancement(const VisualEnhancement& enhancement) {
    visualEnhancement = enhancement;
    
    if (visualEnhancement.waveformVisualization) {
        waveformVisualizer = std::make_unique<WaveformVisualizer>();
        waveformVisualizer->initialize();
    }
    
    if (visualEnhancement.spectrumVisualization) {
        spectrumVisualizer = std::make_unique<SpectrumVisualizer>();
        spectrumVisualizer->initialize();
    }
    
    if (visualEnhancement.phaseVisualization) {
        phaseVisualizer = std::make_unique<PhaseVisualizer>();
        phaseVisualizer->initialize();
    }
    
    if (visualEnhancement.stereoFieldVisualization) {
        stereoFieldVisualizer = std::make_unique<StereoFieldVisualizer>();
        stereoFieldVisualizer->initialize();
    }
    
    if (visualEnhancement.dynamicsVisualization) {
        dynamicsVisualizer = std::make_unique<DynamicsVisualizer>();
        dynamicsVisualizer->initialize();
    }
    
    if (visualEnhancement.effectsVisualization) {
        effectsVisualizer = std::make_unique<EffectsVisualizer>();
        effectsVisualizer->initialize();
    }
    
    if (visualEnhancement.automationVisualization) {
        automationVisualizer = std::make_unique<AutomationVisualizer>();
        automationVisualizer->initialize();
    }
    
    if (visualEnhancement.modulationVisualization) {
        modulationVisualizer = std::make_unique<ModulationVisualizer>();
        modulationVisualizer->initialize();
    }
    
    if (visualEnhancement.spatialVisualization) {
        spatialVisualizer = std::make_unique<SpatialVisualizer>();
        spatialVisualizer->initialize();
    }
    
    if (visualEnhancement.performanceVisualization) {
        performanceVisualizer = std::make_unique<PerformanceVisualizer>();
        performanceVisualizer->initialize();
    }
}

void VoiceEditor::setInstrumentParameters(const InstrumentParameters& parameters) {
    instrumentParameters = parameters;
    
    // Musikalische Parameter anwenden
    if (harmonicEnricher) {
        harmonicEnricher->setEnrichment(parameters.harmonicEnrichment);
    }
    
    if (dynamicExpressor) {
        dynamicExpressor->setExpression(parameters.dynamicExpression);
    }
    
    if (timbreController) {
        timbreController->setTimbre(parameters.timbreControl);
    }
    
    if (resonanceController) {
        resonanceController->setResonance(parameters.resonanceControl);
    }
    
    if (articulationController) {
        articulationController->setArticulation(parameters.articulationControl);
    }
    
    if (vibratoController) {
        vibratoController->setVibrato(parameters.vibratoControl);
    }
    
    if (tremoloController) {
        tremoloController->setTremolo(parameters.tremoloControl);
    }
    
    if (portamentoController) {
        portamentoController->setPortamento(parameters.portamentoControl);
    }
    
    if (legatoController) {
        legatoController->setLegato(parameters.legatoControl);
    }
    
    if (staccatoController) {
        staccatoController->setStaccato(parameters.staccatoControl);
    }
    
    // Optische Parameter anwenden
    if (waveformVisualizer) {
        waveformVisualizer->setScale(parameters.waveformScale);
    }
    
    if (spectrumVisualizer) {
        spectrumVisualizer->setScale(parameters.spectrumScale);
    }
    
    if (phaseVisualizer) {
        phaseVisualizer->setScale(parameters.phaseScale);
    }
    
    if (stereoFieldVisualizer) {
        stereoFieldVisualizer->setScale(parameters.stereoFieldScale);
    }
    
    if (dynamicsVisualizer) {
        dynamicsVisualizer->setScale(parameters.dynamicsScale);
    }
    
    if (effectsVisualizer) {
        effectsVisualizer->setScale(parameters.effectsScale);
    }
    
    if (automationVisualizer) {
        automationVisualizer->setScale(parameters.automationScale);
    }
    
    if (modulationVisualizer) {
        modulationVisualizer->setScale(parameters.modulationScale);
    }
    
    if (spatialVisualizer) {
        spatialVisualizer->setScale(parameters.spatialScale);
    }
    
    if (performanceVisualizer) {
        performanceVisualizer->setScale(parameters.performanceScale);
    }
}

MusicalEnhancement VoiceEditor::getMusicalEnhancement() const {
    return musicalEnhancement;
}

VisualEnhancement VoiceEditor::getVisualEnhancement() const {
    return visualEnhancement;
}

InstrumentParameters VoiceEditor::getInstrumentParameters() const {
    return instrumentParameters;
}

void VoiceEditor::setPlayingPosition(const PlayingPosition& position) {
    currentPosition = position;
    
    // Höhenanpassung
    adjustInstrumentHeight();
    
    // Skalierungsanpassung
    adjustInstrumentScale();
    
    // Interface-Position anpassen
    adjustInterfacePosition();
    
    // Visualisierung aktualisieren
    updateVisualization();
}

void VoiceEditor::togglePlayingPosition() {
    currentPosition.isStanding = !currentPosition.isStanding;
    
    // Standardwerte für die Position
    if (currentPosition.isStanding) {
        currentPosition.heightAdjustment = 0.0f;
        currentPosition.distanceAdjustment = 0.0f;
        currentPosition.angleAdjustment = 0.0f;
        currentPosition.scaleAdjustment = 1.0f;
    } else {
        currentPosition.heightAdjustment = -0.5f;  // Niedrigere Position im Sitzen
        currentPosition.distanceAdjustment = 0.2f; // Näher am Spieler
        currentPosition.angleAdjustment = -15.0f;  // Leichte Neigung nach oben
        currentPosition.scaleAdjustment = 0.9f;    // Etwas kleiner im Sitzen
    }
    
    // Anpassungen anwenden
    setPlayingPosition(currentPosition);
}

PlayingPosition VoiceEditor::getPlayingPosition() const {
    return currentPosition;
}

void VoiceEditor::setVisualAdjustment(const VisualAdjustment& adjustment) {
    visualAdjustment = adjustment;
    updateVisualAdjustment();
}

void VoiceEditor::updateVisualAdjustment() {
    // Instrument-Höhe anpassen
    if (waveformVisualizer) {
        waveformVisualizer->setHeight(visualAdjustment.instrumentHeight);
    }
    
    if (spectrumVisualizer) {
        spectrumVisualizer->setHeight(visualAdjustment.instrumentHeight);
    }
    
    // Instrument-Skalierung anpassen
    if (phaseVisualizer) {
        phaseVisualizer->setScale(visualAdjustment.instrumentScale);
    }
    
    if (stereoFieldVisualizer) {
        stereoFieldVisualizer->setScale(visualAdjustment.instrumentScale);
    }
    
    // Interface-Skalierung anpassen
    if (dynamicsVisualizer) {
        dynamicsVisualizer->setScale(visualAdjustment.interfaceScale);
        dynamicsVisualizer->setOffset(visualAdjustment.interfaceOffset);
    }
    
    if (effectsVisualizer) {
        effectsVisualizer->setScale(visualAdjustment.interfaceScale);
        effectsVisualizer->setOffset(visualAdjustment.interfaceOffset);
    }
    
    // Steuerungselemente anzeigen/ausblenden
    if (automationVisualizer) {
        automationVisualizer->setControlsVisible(currentPosition.isStanding ? 
            visualAdjustment.showStandingControls : visualAdjustment.showSittingControls);
    }
    
    if (modulationVisualizer) {
        modulationVisualizer->setControlsVisible(currentPosition.isStanding ? 
            visualAdjustment.showStandingControls : visualAdjustment.showSittingControls);
    }
}

VisualAdjustment VoiceEditor::getVisualAdjustment() const {
    return visualAdjustment;
}

void VoiceEditor::adjustInstrumentHeight() {
    float baseHeight = currentPosition.isStanding ? 1.0f : 0.7f;
    visualAdjustment.instrumentHeight = baseHeight + currentPosition.heightAdjustment;
    
    // Visualisierungen anpassen
    if (waveformVisualizer) {
        waveformVisualizer->setHeight(visualAdjustment.instrumentHeight);
    }
    
    if (spectrumVisualizer) {
        spectrumVisualizer->setHeight(visualAdjustment.instrumentHeight);
    }
}

void VoiceEditor::adjustInstrumentScale() {
    visualAdjustment.instrumentScale = currentPosition.scaleAdjustment;
    
    // Visualisierungen anpassen
    if (phaseVisualizer) {
        phaseVisualizer->setScale(visualAdjustment.instrumentScale);
    }
    
    if (stereoFieldVisualizer) {
        stereoFieldVisualizer->setScale(visualAdjustment.instrumentScale);
    }
}

void VoiceEditor::adjustInterfacePosition() {
    // Interface-Position basierend auf Spielposition anpassen
    float baseOffset = currentPosition.isStanding ? 0.0f : -0.3f;
    visualAdjustment.interfaceOffset = baseOffset + currentPosition.distanceAdjustment;
    
    // Interface-Skalierung anpassen
    visualAdjustment.interfaceScale = currentPosition.isStanding ? 1.0f : 0.9f;
    
    // Visualisierungen aktualisieren
    if (dynamicsVisualizer) {
        dynamicsVisualizer->setOffset(visualAdjustment.interfaceOffset);
        dynamicsVisualizer->setScale(visualAdjustment.interfaceScale);
    }
    
    if (effectsVisualizer) {
        effectsVisualizer->setOffset(visualAdjustment.interfaceOffset);
        effectsVisualizer->setScale(visualAdjustment.interfaceScale);
    }
}

void VoiceEditor::updateVisualization() {
    // Alle Visualisierungen aktualisieren
    if (waveformVisualizer) {
        waveformVisualizer->update();
    }
    
    if (spectrumVisualizer) {
        spectrumVisualizer->update();
    }
    
    if (phaseVisualizer) {
        phaseVisualizer->update();
    }
    
    if (stereoFieldVisualizer) {
        stereoFieldVisualizer->update();
    }
    
    if (dynamicsVisualizer) {
        dynamicsVisualizer->update();
    }
    
    if (effectsVisualizer) {
        effectsVisualizer->update();
    }
    
    if (automationVisualizer) {
        automationVisualizer->update();
    }
    
    if (modulationVisualizer) {
        modulationVisualizer->update();
    }
    
    if (spatialVisualizer) {
        spatialVisualizer->update();
    }
    
    if (performanceVisualizer) {
        performanceVisualizer->update();
    }
}

void VoiceEditor::setMenuState(const MenuStructure& newMenuState) {
    menuState = newMenuState;
    updateMenuVisibility();
    updateInterfaceState();
}

void VoiceEditor::toggleMenuOption(const std::string& menuName, const std::string& optionName) {
    if (menuName == "File") {
        if (optionName == "newProject") menuState.fileMenu.newProject = !menuState.fileMenu.newProject;
        else if (optionName == "openProject") menuState.fileMenu.openProject = !menuState.fileMenu.openProject;
        else if (optionName == "saveProject") menuState.fileMenu.saveProject = !menuState.fileMenu.saveProject;
        // ... weitere File-Menü-Optionen
    }
    else if (menuName == "Edit") {
        if (optionName == "undo") menuState.editMenu.undo = !menuState.editMenu.undo;
        else if (optionName == "redo") menuState.editMenu.redo = !menuState.editMenu.redo;
        else if (optionName == "cut") menuState.editMenu.cut = !menuState.editMenu.cut;
        // ... weitere Edit-Menü-Optionen
    }
    // ... weitere Menüs
    
    updateMenuVisibility();
    updateInterfaceState();
}

MenuStructure VoiceEditor::getMenuState() const {
    return menuState;
}

void VoiceEditor::executeMenuAction(const std::string& menuName, const std::string& actionName) {
    handleMenuAction(menuName, actionName);
}

void VoiceEditor::initializeMenuState() {
    // Standard-Menüstatus initialisieren
    menuState = MenuStructure();
    
    // Standardmäßig sichtbare Menüpunkte aktivieren
    menuState.viewMenu.showMixer = true;
    menuState.viewMenu.showTransport = true;
    menuState.viewMenu.showToolbar = true;
    
    menuState.mixMenu.showChannelStrip = true;
    menuState.mixMenu.showVolume = true;
    menuState.mixMenu.showPan = true;
    
    menuState.effectsMenu.showEQ = true;
    menuState.effectsMenu.showCompressor = true;
    
    updateMenuVisibility();
}

void VoiceEditor::updateMenuVisibility() {
    // Mixer-Ansicht
    if (menuState.viewMenu.showMixer) {
        if (dynamicsVisualizer) dynamicsVisualizer->setVisible(true);
        if (effectsVisualizer) effectsVisualizer->setVisible(true);
    } else {
        if (dynamicsVisualizer) dynamicsVisualizer->setVisible(false);
        if (effectsVisualizer) effectsVisualizer->setVisible(false);
    }
    
    // Effekt-Ansicht
    if (menuState.effectsMenu.showEQ) {
        if (spectrumVisualizer) spectrumVisualizer->setVisible(true);
    } else {
        if (spectrumVisualizer) spectrumVisualizer->setVisible(false);
    }
    
    // Automatisierung
    if (menuState.viewMenu.showAutomation) {
        if (automationVisualizer) automationVisualizer->setVisible(true);
    } else {
        if (automationVisualizer) automationVisualizer->setVisible(false);
    }
    
    // Weitere Menüpunkte entsprechend aktualisieren
    // ...
}

void VoiceEditor::handleMenuAction(const std::string& menuName, const std::string& actionName) {
    if (menuName == "File") {
        if (actionName == "newProject") {
            // Neues Projekt erstellen
            initialize();
        }
        else if (actionName == "openProject") {
            // Projekt öffnen
            // TODO: Implementierung
        }
        else if (actionName == "saveProject") {
            // Projekt speichern
            // TODO: Implementierung
        }
    }
    else if (menuName == "Edit") {
        if (actionName == "undo") {
            // Undo-Aktion
            // TODO: Implementierung
        }
        else if (actionName == "redo") {
            // Redo-Aktion
            // TODO: Implementierung
        }
    }
    else if (menuName == "View") {
        if (actionName == "showMixer") {
            menuState.viewMenu.showMixer = !menuState.viewMenu.showMixer;
            updateMenuVisibility();
        }
        else if (actionName == "showAutomation") {
            menuState.viewMenu.showAutomation = !menuState.viewMenu.showAutomation;
            updateMenuVisibility();
        }
    }
    else if (menuName == "Effects") {
        if (actionName == "showEQ") {
            menuState.effectsMenu.showEQ = !menuState.effectsMenu.showEQ;
            updateMenuVisibility();
        }
        else if (actionName == "showCompressor") {
            menuState.effectsMenu.showCompressor = !menuState.effectsMenu.showCompressor;
            updateMenuVisibility();
        }
    }
    // Weitere Menüaktionen entsprechend implementieren
}

void VoiceEditor::updateInterfaceState() {
    // Interface-Status basierend auf Menüeinstellungen aktualisieren
    
    // Mixer-Status
    if (menuState.mixMenu.showChannelStrip) {
        // Channel Strip anzeigen
        if (dynamicsVisualizer) {
            dynamicsVisualizer->setChannelStripVisible(true);
        }
    }
    
    // Effekt-Status
    if (menuState.effectsMenu.showEQ) {
        // EQ anzeigen
        if (spectrumVisualizer) {
            spectrumVisualizer->setEQVisible(true);
        }
    }
    
    // Automatisierungs-Status
    if (menuState.viewMenu.showAutomation) {
        // Automatisierung anzeigen
        if (automationVisualizer) {
            automationVisualizer->setAutomationVisible(true);
        }
    }
    
    // Weitere Interface-Elemente entsprechend aktualisieren
}

void VoiceEditor::setExtendedMenuState(const ExtendedMenuStructure& newExtendedMenuState) {
    extendedMenuState = newExtendedMenuState;
    updateExtendedMenuVisibility();
    updateExtendedInterfaceState();
}

void VoiceEditor::toggleExtendedMenuOption(const std::string& menuName, const std::string& optionName) {
    if (menuName == "Track") {
        if (optionName == "showTrackHeader") extendedMenuState.trackMenu.showTrackHeader = !extendedMenuState.trackMenu.showTrackHeader;
        else if (optionName == "showTrackControls") extendedMenuState.trackMenu.showTrackControls = !extendedMenuState.trackMenu.showTrackControls;
        else if (optionName == "showTrackColor") extendedMenuState.trackMenu.showTrackColor = !extendedMenuState.trackMenu.showTrackColor;
        // ... weitere Track-Menü-Optionen
    }
    else if (menuName == "Region") {
        if (optionName == "showRegionName") extendedMenuState.regionMenu.showRegionName = !extendedMenuState.regionMenu.showRegionName;
        else if (optionName == "showRegionColor") extendedMenuState.regionMenu.showRegionColor = !extendedMenuState.regionMenu.showRegionColor;
        else if (optionName == "showRegionIcon") extendedMenuState.regionMenu.showRegionIcon = !extendedMenuState.regionMenu.showRegionIcon;
        // ... weitere Region-Menü-Optionen
    }
    else if (menuName == "MIDI") {
        if (optionName == "showMIDIEditor") extendedMenuState.midiMenu.showMIDIEditor = !extendedMenuState.midiMenu.showMIDIEditor;
        else if (optionName == "showMIDIList") extendedMenuState.midiMenu.showMIDIList = !extendedMenuState.midiMenu.showMIDIList;
        else if (optionName == "showMIDIEvent") extendedMenuState.midiMenu.showMIDIEvent = !extendedMenuState.midiMenu.showMIDIEvent;
        // ... weitere MIDI-Menü-Optionen
    }
    // ... weitere Menüs
    
    updateExtendedMenuVisibility();
    updateExtendedInterfaceState();
}

ExtendedMenuStructure VoiceEditor::getExtendedMenuState() const {
    return extendedMenuState;
}

void VoiceEditor::executeExtendedMenuAction(const std::string& menuName, const std::string& actionName) {
    handleExtendedMenuAction(menuName, actionName);
}

void VoiceEditor::initializeExtendedMenuState() {
    // Standard-Erweiterte-Menüstatus initialisieren
    extendedMenuState = ExtendedMenuStructure();
    
    // Standardmäßig sichtbare Menüpunkte aktivieren
    extendedMenuState.trackMenu.showTrackHeader = true;
    extendedMenuState.trackMenu.showTrackControls = true;
    extendedMenuState.trackMenu.showTrackName = true;
    extendedMenuState.trackMenu.showTrackVolume = true;
    extendedMenuState.trackMenu.showTrackPan = true;
    
    extendedMenuState.regionMenu.showRegionName = true;
    extendedMenuState.regionMenu.showRegionColor = true;
    extendedMenuState.regionMenu.showRegionLength = true;
    
    extendedMenuState.midiMenu.showMIDIEditor = true;
    extendedMenuState.midiMenu.showMIDIEvent = true;
    
    extendedMenuState.audioMenu.showAudioEditor = true;
    extendedMenuState.audioMenu.showAudioEvent = true;
    
    extendedMenuState.scoreMenu.showScoreEditor = true;
    extendedMenuState.scoreMenu.showScoreEvent = true;
    
    extendedMenuState.smartControlsMenu.showSmartControls = true;
    extendedMenuState.smartControlsMenu.showSmartControlsEvent = true;
    
    extendedMenuState.drummerMenu.showDrummer = true;
    extendedMenuState.drummerMenu.showDrummerEvent = true;
    
    updateExtendedMenuVisibility();
}

void VoiceEditor::updateExtendedMenuVisibility() {
    // Track-Ansicht
    if (extendedMenuState.trackMenu.showTrackHeader) {
        if (dynamicsVisualizer) dynamicsVisualizer->setTrackHeaderVisible(true);
    } else {
        if (dynamicsVisualizer) dynamicsVisualizer->setTrackHeaderVisible(false);
    }
    
    if (extendedMenuState.trackMenu.showTrackControls) {
        if (effectsVisualizer) effectsVisualizer->setTrackControlsVisible(true);
    } else {
        if (effectsVisualizer) effectsVisualizer->setTrackControlsVisible(false);
    }
    
    // Region-Ansicht
    if (extendedMenuState.regionMenu.showRegionName) {
        if (spectrumVisualizer) spectrumVisualizer->setRegionNameVisible(true);
    } else {
        if (spectrumVisualizer) spectrumVisualizer->setRegionNameVisible(false);
    }
    
    // MIDI-Ansicht
    if (extendedMenuState.midiMenu.showMIDIEditor) {
        if (phaseVisualizer) phaseVisualizer->setMIDIEditorVisible(true);
    } else {
        if (phaseVisualizer) phaseVisualizer->setMIDIEditorVisible(false);
    }
    
    // Audio-Ansicht
    if (extendedMenuState.audioMenu.showAudioEditor) {
        if (stereoFieldVisualizer) stereoFieldVisualizer->setAudioEditorVisible(true);
    } else {
        if (stereoFieldVisualizer) stereoFieldVisualizer->setAudioEditorVisible(false);
    }
    
    // Score-Ansicht
    if (extendedMenuState.scoreMenu.showScoreEditor) {
        if (dynamicsVisualizer) dynamicsVisualizer->setScoreEditorVisible(true);
    } else {
        if (dynamicsVisualizer) dynamicsVisualizer->setScoreEditorVisible(false);
    }
    
    // Smart Controls
    if (extendedMenuState.smartControlsMenu.showSmartControls) {
        if (effectsVisualizer) effectsVisualizer->setSmartControlsVisible(true);
    } else {
        if (effectsVisualizer) effectsVisualizer->setSmartControlsVisible(false);
    }
    
    // Drummer
    if (extendedMenuState.drummerMenu.showDrummer) {
        if (automationVisualizer) automationVisualizer->setDrummerVisible(true);
    } else {
        if (automationVisualizer) automationVisualizer->setDrummerVisible(false);
    }
}

void VoiceEditor::handleExtendedMenuAction(const std::string& menuName, const std::string& actionName) {
    if (menuName == "Track") {
        if (actionName == "showTrackHeader") {
            extendedMenuState.trackMenu.showTrackHeader = !extendedMenuState.trackMenu.showTrackHeader;
            updateExtendedMenuVisibility();
        }
        else if (actionName == "showTrackControls") {
            extendedMenuState.trackMenu.showTrackControls = !extendedMenuState.trackMenu.showTrackControls;
            updateExtendedMenuVisibility();
        }
    }
    else if (menuName == "Region") {
        if (actionName == "showRegionName") {
            extendedMenuState.regionMenu.showRegionName = !extendedMenuState.regionMenu.showRegionName;
            updateExtendedMenuVisibility();
        }
        else if (actionName == "showRegionColor") {
            extendedMenuState.regionMenu.showRegionColor = !extendedMenuState.regionMenu.showRegionColor;
            updateExtendedMenuVisibility();
        }
    }
    else if (menuName == "MIDI") {
        if (actionName == "showMIDIEditor") {
            extendedMenuState.midiMenu.showMIDIEditor = !extendedMenuState.midiMenu.showMIDIEditor;
            updateExtendedMenuVisibility();
        }
        else if (actionName == "showMIDIList") {
            extendedMenuState.midiMenu.showMIDIList = !extendedMenuState.midiMenu.showMIDIList;
            updateExtendedMenuVisibility();
        }
    }
    // ... weitere Menüaktionen entsprechend implementieren
}

void VoiceEditor::updateExtendedInterfaceState() {
    // Erweiterte Interface-Status basierend auf Menüeinstellungen aktualisieren
    
    // Track-Status
    if (extendedMenuState.trackMenu.showTrackHeader) {
        if (dynamicsVisualizer) {
            dynamicsVisualizer->setTrackHeaderState(true);
        }
    }
    
    // Region-Status
    if (extendedMenuState.regionMenu.showRegionName) {
        if (spectrumVisualizer) {
            spectrumVisualizer->setRegionNameState(true);
        }
    }
    
    // MIDI-Status
    if (extendedMenuState.midiMenu.showMIDIEditor) {
        if (phaseVisualizer) {
            phaseVisualizer->setMIDIEditorState(true);
        }
    }
    
    // Audio-Status
    if (extendedMenuState.audioMenu.showAudioEditor) {
        if (stereoFieldVisualizer) {
            stereoFieldVisualizer->setAudioEditorState(true);
        }
    }
    
    // Score-Status
    if (extendedMenuState.scoreMenu.showScoreEditor) {
        if (dynamicsVisualizer) {
            dynamicsVisualizer->setScoreEditorState(true);
        }
    }
    
    // Smart Controls Status
    if (extendedMenuState.smartControlsMenu.showSmartControls) {
        if (effectsVisualizer) {
            effectsVisualizer->setSmartControlsState(true);
        }
    }
    
    // Drummer Status
    if (extendedMenuState.drummerMenu.showDrummer) {
        if (automationVisualizer) {
            automationVisualizer->setDrummerState(true);
        }
    }
}

void VoiceEditor::setRevolutionaryFeatures(const RevolutionaryFeatures& newFeatures) {
    revolutionaryFeatures = newFeatures;
    updateRevolutionaryFeatureVisibility();
    updateRevolutionaryInterfaceState();
}

void VoiceEditor::toggleRevolutionaryFeature(const std::string& category, const std::string& feature) {
    if (category == "AIComposition") {
        if (feature == "styleTransfer") revolutionaryFeatures.aiComposition.styleTransfer = !revolutionaryFeatures.aiComposition.styleTransfer;
        else if (feature == "genreAdaptation") revolutionaryFeatures.aiComposition.genreAdaptation = !revolutionaryFeatures.aiComposition.genreAdaptation;
        else if (feature == "moodAnalysis") revolutionaryFeatures.aiComposition.moodAnalysis = !revolutionaryFeatures.aiComposition.moodAnalysis;
        // ... weitere AI-Kompositions-Features
    }
    else if (category == "HolographicVisualization") {
        if (feature == "waveform3D") revolutionaryFeatures.holographicVisualization.waveform3D = !revolutionaryFeatures.holographicVisualization.waveform3D;
        else if (feature == "spectrum3D") revolutionaryFeatures.holographicVisualization.spectrum3D = !revolutionaryFeatures.holographicVisualization.spectrum3D;
        else if (feature == "phase3D") revolutionaryFeatures.holographicVisualization.phase3D = !revolutionaryFeatures.holographicVisualization.phase3D;
        // ... weitere holographische Visualisierungs-Features
    }
    else if (category == "ImmersiveControl") {
        if (feature == "gestureControl") revolutionaryFeatures.immersiveControl.gestureControl = !revolutionaryFeatures.immersiveControl.gestureControl;
        else if (feature == "voiceControl") revolutionaryFeatures.immersiveControl.voiceControl = !revolutionaryFeatures.immersiveControl.voiceControl;
        else if (feature == "eyeTracking") revolutionaryFeatures.immersiveControl.eyeTracking = !revolutionaryFeatures.immersiveControl.eyeTracking;
        // ... weitere immersive Steuerungs-Features
    }
    // ... weitere Kategorien
    
    updateRevolutionaryFeatureVisibility();
    updateRevolutionaryInterfaceState();
}

RevolutionaryFeatures VoiceEditor::getRevolutionaryFeatures() const {
    return revolutionaryFeatures;
}

void VoiceEditor::executeRevolutionaryFeature(const std::string& category, const std::string& feature) {
    handleRevolutionaryFeatureAction(category, feature);
}

void VoiceEditor::initializeRevolutionaryFeatures() {
    // Standard-Revolutionäre-Features initialisieren
    revolutionaryFeatures = RevolutionaryFeatures();
    
    // Standardmäßig aktivierte Features
    revolutionaryFeatures.aiComposition.styleTransfer = true;
    revolutionaryFeatures.aiComposition.genreAdaptation = true;
    revolutionaryFeatures.aiComposition.moodAnalysis = true;
    
    revolutionaryFeatures.holographicVisualization.waveform3D = true;
    revolutionaryFeatures.holographicVisualization.spectrum3D = true;
    revolutionaryFeatures.holographicVisualization.phase3D = true;
    
    revolutionaryFeatures.immersiveControl.gestureControl = true;
    revolutionaryFeatures.immersiveControl.voiceControl = true;
    revolutionaryFeatures.immersiveControl.eyeTracking = true;
    
    revolutionaryFeatures.collaborativeFeatures.realTimeCollaboration = true;
    revolutionaryFeatures.collaborativeFeatures.spatialCollaboration = true;
    revolutionaryFeatures.collaborativeFeatures.holographicCollaboration = true;
    
    revolutionaryFeatures.advancedAudioProcessing.quantumProcessing = true;
    revolutionaryFeatures.advancedAudioProcessing.neuralProcessing = true;
    revolutionaryFeatures.advancedAudioProcessing.adaptiveProcessing = true;
    
    revolutionaryFeatures.intelligentAutomation.neuralAutomation = true;
    revolutionaryFeatures.intelligentAutomation.predictiveAutomation = true;
    revolutionaryFeatures.intelligentAutomation.adaptiveAutomation = true;
    
    revolutionaryFeatures.advancedPluginIntegration.quantumPlugins = true;
    revolutionaryFeatures.advancedPluginIntegration.neuralPlugins = true;
    revolutionaryFeatures.advancedPluginIntegration.adaptivePlugins = true;
    
    revolutionaryFeatures.performanceOptimization.quantumOptimization = true;
    revolutionaryFeatures.performanceOptimization.neuralOptimization = true;
    revolutionaryFeatures.performanceOptimization.adaptiveOptimization = true;
    
    updateRevolutionaryFeatureVisibility();
}

void VoiceEditor::updateRevolutionaryFeatureVisibility() {
    // KI-Komposition
    if (revolutionaryFeatures.aiComposition.styleTransfer) {
        if (aiStyleTransfer) aiStyleTransfer->setStyleTransferVisible(true);
    } else {
        if (aiStyleTransfer) aiStyleTransfer->setStyleTransferVisible(false);
    }
    
    if (revolutionaryFeatures.aiComposition.genreAdaptation) {
        if (aiGenreAdapter) aiGenreAdapter->setGenreAdaptationVisible(true);
    } else {
        if (aiGenreAdapter) aiGenreAdapter->setGenreAdaptationVisible(false);
    }
    
    // Holographische Visualisierung
    if (revolutionaryFeatures.holographicVisualization.waveform3D) {
        if (waveformVisualizer) waveformVisualizer->set3DVisible(true);
    } else {
        if (waveformVisualizer) waveformVisualizer->set3DVisible(false);
    }
    
    if (revolutionaryFeatures.holographicVisualization.spectrum3D) {
        if (spectrumVisualizer) spectrumVisualizer->set3DVisible(true);
    } else {
        if (spectrumVisualizer) spectrumVisualizer->set3DVisible(false);
    }
    
    // Immersive Steuerung
    if (revolutionaryFeatures.immersiveControl.gestureControl) {
        // Gestensteuerung aktivieren
        enableGestureControl();
    } else {
        // Gestensteuerung deaktivieren
        disableGestureControl();
    }
    
    if (revolutionaryFeatures.immersiveControl.voiceControl) {
        // Sprachsteuerung aktivieren
        enableVoiceControl();
    } else {
        // Sprachsteuerung deaktivieren
        disableVoiceControl();
    }
    
    // Kollaborative Funktionen
    if (revolutionaryFeatures.collaborativeFeatures.realTimeCollaboration) {
        // Echtzeit-Kollaboration aktivieren
        enableRealTimeCollaboration();
    } else {
        // Echtzeit-Kollaboration deaktivieren
        disableRealTimeCollaboration();
    }
    
    // Erweiterte Audio-Verarbeitung
    if (revolutionaryFeatures.advancedAudioProcessing.quantumProcessing) {
        // Quanten-Verarbeitung aktivieren
        enableQuantumProcessing();
    } else {
        // Quanten-Verarbeitung deaktivieren
        disableQuantumProcessing();
    }
    
    // Intelligente Automatisierung
    if (revolutionaryFeatures.intelligentAutomation.neuralAutomation) {
        // Neuronale Automatisierung aktivieren
        enableNeuralAutomation();
    } else {
        // Neuronale Automatisierung deaktivieren
        disableNeuralAutomation();
    }
    
    // Erweiterte Plugin-Integration
    if (revolutionaryFeatures.advancedPluginIntegration.quantumPlugins) {
        // Quanten-Plugins aktivieren
        enableQuantumPlugins();
    } else {
        // Quanten-Plugins deaktivieren
        disableQuantumPlugins();
    }
    
    // Performance-Optimierung
    if (revolutionaryFeatures.performanceOptimization.quantumOptimization) {
        // Quanten-Optimierung aktivieren
        enableQuantumOptimization();
    } else {
        // Quanten-Optimierung deaktivieren
        disableQuantumOptimization();
    }
}

void VoiceEditor::handleRevolutionaryFeatureAction(const std::string& category, const std::string& feature) {
    if (category == "AIComposition") {
        if (feature == "styleTransfer") {
            revolutionaryFeatures.aiComposition.styleTransfer = !revolutionaryFeatures.aiComposition.styleTransfer;
            updateRevolutionaryFeatureVisibility();
        }
        else if (feature == "genreAdaptation") {
            revolutionaryFeatures.aiComposition.genreAdaptation = !revolutionaryFeatures.aiComposition.genreAdaptation;
            updateRevolutionaryFeatureVisibility();
        }
    }
    else if (category == "HolographicVisualization") {
        if (feature == "waveform3D") {
            revolutionaryFeatures.holographicVisualization.waveform3D = !revolutionaryFeatures.holographicVisualization.waveform3D;
            updateRevolutionaryFeatureVisibility();
        }
        else if (feature == "spectrum3D") {
            revolutionaryFeatures.holographicVisualization.spectrum3D = !revolutionaryFeatures.holographicVisualization.spectrum3D;
            updateRevolutionaryFeatureVisibility();
        }
    }
    else if (category == "ImmersiveControl") {
        if (feature == "gestureControl") {
            revolutionaryFeatures.immersiveControl.gestureControl = !revolutionaryFeatures.immersiveControl.gestureControl;
            updateRevolutionaryFeatureVisibility();
        }
        else if (feature == "voiceControl") {
            revolutionaryFeatures.immersiveControl.voiceControl = !revolutionaryFeatures.immersiveControl.voiceControl;
            updateRevolutionaryFeatureVisibility();
        }
    }
    // ... weitere Kategorien und Features
}

void VoiceEditor::updateRevolutionaryInterfaceState() {
    // KI-Komposition Status
    if (revolutionaryFeatures.aiComposition.styleTransfer) {
        if (aiStyleTransfer) {
            aiStyleTransfer->setStyleTransferState(true);
        }
    }
    
    // Holographische Visualisierung Status
    if (revolutionaryFeatures.holographicVisualization.waveform3D) {
        if (waveformVisualizer) {
            waveformVisualizer->set3DState(true);
        }
    }
    
    // Immersive Steuerung Status
    if (revolutionaryFeatures.immersiveControl.gestureControl) {
        setGestureControlState(true);
    }
    
    // Kollaborative Funktionen Status
    if (revolutionaryFeatures.collaborativeFeatures.realTimeCollaboration) {
        setRealTimeCollaborationState(true);
    }
    
    // Erweiterte Audio-Verarbeitung Status
    if (revolutionaryFeatures.advancedAudioProcessing.quantumProcessing) {
        setQuantumProcessingState(true);
    }
    
    // Intelligente Automatisierung Status
    if (revolutionaryFeatures.intelligentAutomation.neuralAutomation) {
        setNeuralAutomationState(true);
    }
    
    // Erweiterte Plugin-Integration Status
    if (revolutionaryFeatures.advancedPluginIntegration.quantumPlugins) {
        setQuantumPluginsState(true);
    }
    
    // Performance-Optimierung Status
    if (revolutionaryFeatures.performanceOptimization.quantumOptimization) {
        setQuantumOptimizationState(true);
    }
}

void VoiceEditor::initializeAdvancedFeatures() {
    // Initialisiere erweiterte holografische Visualisierungen
    if (advancedHolographic.volumetricWaveform) {
        volumetricWaveformVisualizer = std::make_unique<VolumetricWaveformVisualizer>();
        volumetricWaveformVisualizer->initialize();
    }
    
    if (advancedHolographic.dynamicSpectrum) {
        dynamicSpectrumVisualizer = std::make_unique<DynamicSpectrumVisualizer>();
        dynamicSpectrumVisualizer->initialize();
    }
    
    // Initialisiere immersive Steuerungselemente
    if (enhancedImmersive.advancedGestureControl) {
        advancedGestureController = std::make_unique<AdvancedGestureController>();
        advancedGestureController->initialize();
    }
    
    if (enhancedImmersive.naturalVoiceControl) {
        naturalVoiceController = std::make_unique<NaturalVoiceController>();
        naturalVoiceController->initialize();
    }
    
    // Initialisiere Kollaborationsfunktionen
    if (collaborative.realTimeCollaboration) {
        realTimeCollaboration = std::make_unique<RealTimeCollaboration>();
        realTimeCollaboration->initialize();
    }
    
    if (collaborative.sessionSharing) {
        sessionSharing = std::make_unique<SessionSharing>();
        sessionSharing->initialize();
    }
    
    // Initialisiere Performance-Optimierungen
    if (performance.adaptiveRendering) {
        adaptiveRenderer = std::make_unique<AdaptiveRenderer>();
        adaptiveRenderer->initialize();
    }
    
    if (performance.dynamicLOD) {
        dynamicLOD = std::make_unique<DynamicLOD>();
        dynamicLOD->initialize();
    }
}

void VoiceEditor::updateAdvancedFeatures() {
    // Aktualisiere holografische Visualisierungen
    if (volumetricWaveformVisualizer) {
        volumetricWaveformVisualizer->update();
    }
    
    if (dynamicSpectrumVisualizer) {
        dynamicSpectrumVisualizer->update();
    }
    
    // Aktualisiere Steuerungselemente
    if (advancedGestureController) {
        advancedGestureController->update();
    }
    
    if (naturalVoiceController) {
        naturalVoiceController->update();
    }
    
    // Aktualisiere Kollaborationsfunktionen
    if (realTimeCollaboration) {
        realTimeCollaboration->update();
    }
    
    if (sessionSharing) {
        sessionSharing->update();
    }
    
    // Aktualisiere Performance-Optimierungen
    if (adaptiveRenderer) {
        adaptiveRenderer->update();
    }
    
    if (dynamicLOD) {
        dynamicLOD->update();
    }
}

void VoiceEditor::optimizePerformance() {
    // GPU-Optimierung
    if (gpuOptimizer) {
        gpuOptimizer->optimize();
    }
    
    // CPU-Optimierung
    if (cpuOptimizer) {
        cpuOptimizer->optimize();
    }
    
    // Speicher-Optimierung
    if (memoryOptimizer) {
        memoryOptimizer->optimize();
    }
    
    // Netzwerk-Optimierung
    if (networkOptimizer) {
        networkOptimizer->optimize();
    }
}

void VoiceEditor::enhanceCollaboration() {
    // Echtzeit-Kollaboration
    if (realTimeCollaboration) {
        realTimeCollaboration->enhance();
    }
    
    // Sitzungs-Sharing
    if (sessionSharing) {
        sessionSharing->enhance();
    }
    
    // Remote-Steuerung
    if (remoteControl) {
        remoteControl->enhance();
    }
}

void VoiceEditor::improveUserInteraction() {
    // Gestensteuerung
    if (advancedGestureController) {
        advancedGestureController->improve();
    }
    
    // Sprachsteuerung
    if (naturalVoiceController) {
        naturalVoiceController->improve();
    }
    
    // Augenverfolgung
    if (preciseEyeTracker) {
        preciseEyeTracker->improve();
    }
}

void VoiceEditor::finalizeAIFunctions() {
    // KI-Mastering
    if (aiMastering) {
        aiMastering->finalize();
    }
    
    // KI-Mixing
    if (aiMixing) {
        aiMixing->finalize();
    }
    
    // KI-Komposition
    if (aiComposition) {
        aiComposition->finalize();
    }
}

} // namespace VR_DAW 