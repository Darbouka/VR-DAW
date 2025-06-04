#include "SampleToMIDIConverter.hpp"
#include <juce_audio_formats/juce_audio_formats.h>

namespace VR_DAW {

SampleToMIDIConverter::SampleToMIDIConverter() {
    initialize();
}

SampleToMIDIConverter::~SampleToMIDIConverter() {
    shutdown();
}

void SampleToMIDIConverter::initialize() {
    initializeDSP();
}

void SampleToMIDIConverter::shutdown() {
    fft.reset();
    pitchDetector.reset();
    onsetDetector.reset();
    spectralAnalyzer.reset();
}

void SampleToMIDIConverter::initializeDSP() {
    // FFT für Spektralanalyse
    fft = std::make_unique<juce::dsp::FFT>(10); // 1024 Punkte
    
    // Pitch-Detektor
    pitchDetector = std::make_unique<juce::dsp::PitchDetector>();
    pitchDetector->setThreshold(parameters.pitchDetectionThreshold);
    
    // Onset-Detektor
    onsetDetector = std::make_unique<juce::dsp::OnsetDetector>();
    onsetDetector->setThreshold(parameters.onsetDetectionThreshold);
    
    // Spektral-Analyzer
    spectralAnalyzer = std::make_unique<juce::dsp::SpectralAnalyzer>();
    spectralAnalyzer->setResolution(parameters.spectralAnalysisResolution);
}

void SampleToMIDIConverter::convertSampleToMIDI(const std::string& inputPath, const std::string& outputPath) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(juce::File(inputPath)));
    if (!reader) return;
    
    juce::AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
    
    analyzeAndConvert(buffer);
    
    // MIDI-Datei erstellen
    juce::MidiFile midiFile;
    juce::MidiMessageSequence sequence;
    
    // MIDI-Noten hinzufügen
    for (size_t i = 0; i < results.midiNotes.size(); ++i) {
        sequence.addEvent(results.midiNotes[i]);
    }
    
    midiFile.addTrack(sequence);
    
    // MIDI-Datei speichern
    std::unique_ptr<juce::FileOutputStream> outputStream = std::make_unique<juce::FileOutputStream>(juce::File(outputPath));
    if (outputStream->openedOk()) {
        midiFile.writeTo(*outputStream);
    }
}

void SampleToMIDIConverter::analyzeAndConvert(const juce::AudioBuffer<float>& buffer) {
    // Effekte analysieren
    analyzeEffects(buffer);
    
    // Mixing analysieren
    analyzeMixing(buffer);
    
    // Mastering analysieren
    analyzeMastering(buffer);
    
    // Pitch und Onsets erkennen
    detectPitch(buffer);
    detectOnsets(buffer);
    
    // Spektrum analysieren
    analyzeSpectrum(buffer);
    
    // MIDI generieren
    generateMIDI();
}

void SampleToMIDIConverter::analyzeEffects(const juce::AudioBuffer<float>& buffer) {
    detectReverb(buffer);
    detectDelay(buffer);
    detectCompression(buffer);
    detectEQ(buffer);
}

void SampleToMIDIConverter::detectReverb(const juce::AudioBuffer<float>& buffer) {
    // Reverb-Analyse durch FFT und Impulsantwort
    std::vector<float> fftData(1024);
    buffer.copyFrom(0, 0, fftData.data(), fftData.size());
    
    fft->performRealOnlyForwardTransform(fftData.data());
    
    // Reverb-Parameter aus dem Spektrum extrahieren
    float decaySum = 0.0f;
    int decayCount = 0;
    
    for (int i = 0; i < fftData.size() / 2; ++i) {
        float magnitude = std::abs(fftData[i]);
        if (magnitude > 0.1f) {
            decaySum += magnitude;
            decayCount++;
        }
    }
    
    results.effects.reverbAmount = decaySum / decayCount;
    results.effects.reverbSize = std::min(1.0f, results.effects.reverbAmount * 2.0f);
}

void SampleToMIDIConverter::detectDelay(const juce::AudioBuffer<float>& buffer) {
    // Delay-Analyse durch Autokorrelation
    std::vector<float> correlation(1024);
    
    for (int i = 0; i < correlation.size(); ++i) {
        float sum = 0.0f;
        for (int j = 0; j < buffer.getNumSamples() - i; ++j) {
            sum += buffer.getSample(0, j) * buffer.getSample(0, j + i);
        }
        correlation[i] = sum;
    }
    
    // Delay-Zeit und Feedback finden
    float maxCorrelation = 0.0f;
    int delayTime = 0;
    
    for (int i = 1; i < correlation.size(); ++i) {
        if (correlation[i] > maxCorrelation) {
            maxCorrelation = correlation[i];
            delayTime = i;
        }
    }
    
    results.effects.delayTime = delayTime / 44100.0f;
    results.effects.delayFeedback = maxCorrelation;
}

void SampleToMIDIConverter::detectCompression(const juce::AudioBuffer<float>& buffer) {
    // Kompression durch Dynamik-Analyse erkennen
    float peakLevel = 0.0f;
    float rmsLevel = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float sample = std::abs(buffer.getSample(0, i));
        peakLevel = std::max(peakLevel, sample);
        rmsLevel += sample * sample;
    }
    
    rmsLevel = std::sqrt(rmsLevel / buffer.getNumSamples());
    
    // Kompressions-Parameter berechnen
    results.effects.compressionThreshold = peakLevel * 0.7f;
    results.effects.compressionRatio = peakLevel / (rmsLevel + 0.0001f);
}

void SampleToMIDIConverter::detectEQ(const juce::AudioBuffer<float>& buffer) {
    // EQ-Analyse durch FFT
    std::vector<float> fftData(1024);
    buffer.copyFrom(0, 0, fftData.data(), fftData.size());
    
    fft->performRealOnlyForwardTransform(fftData.data());
    
    // EQ-Bänder extrahieren
    results.effects.eqBands.resize(10);
    for (int i = 0; i < 10; ++i) {
        int startBin = i * fftData.size() / 20;
        int endBin = (i + 1) * fftData.size() / 20;
        
        float bandSum = 0.0f;
        for (int j = startBin; j < endBin; ++j) {
            bandSum += std::abs(fftData[j]);
        }
        
        results.effects.eqBands[i] = bandSum / (endBin - startBin);
    }
}

void SampleToMIDIConverter::analyzeMixing(const juce::AudioBuffer<float>& buffer) {
    detectPanning(buffer);
    detectVolume(buffer);
    detectStereoWidth(buffer);
}

void SampleToMIDIConverter::detectPanning(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 2) {
        results.mixing.panning = 0.0f;
        return;
    }
    
    float leftSum = 0.0f;
    float rightSum = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        leftSum += std::abs(buffer.getSample(0, i));
        rightSum += std::abs(buffer.getSample(1, i));
    }
    
    results.mixing.panning = (rightSum - leftSum) / (rightSum + leftSum + 0.0001f);
}

void SampleToMIDIConverter::detectVolume(const juce::AudioBuffer<float>& buffer) {
    float sum = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        sum += std::abs(buffer.getSample(0, i));
    }
    
    results.mixing.volume = sum / buffer.getNumSamples();
}

void SampleToMIDIConverter::detectStereoWidth(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 2) {
        results.mixing.stereoWidth = 0.0f;
        return;
    }
    
    float midSum = 0.0f;
    float sideSum = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float mid = (buffer.getSample(0, i) + buffer.getSample(1, i)) * 0.5f;
        float side = (buffer.getSample(0, i) - buffer.getSample(1, i)) * 0.5f;
        
        midSum += std::abs(mid);
        sideSum += std::abs(side);
    }
    
    results.mixing.stereoWidth = sideSum / (midSum + 0.0001f);
}

void SampleToMIDIConverter::analyzeMastering(const juce::AudioBuffer<float>& buffer) {
    detectLoudness(buffer);
    detectDynamicRange(buffer);
    detectLimiting(buffer);
}

void SampleToMIDIConverter::detectLoudness(const juce::AudioBuffer<float>& buffer) {
    float sum = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        sum += buffer.getSample(0, i) * buffer.getSample(0, i);
    }
    
    results.mastering.loudness = std::sqrt(sum / buffer.getNumSamples());
}

void SampleToMIDIConverter::detectDynamicRange(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    float rms = 0.0f;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float sample = std::abs(buffer.getSample(0, i));
        peak = std::max(peak, sample);
        rms += sample * sample;
    }
    
    rms = std::sqrt(rms / buffer.getNumSamples());
    results.mastering.dynamicRange = peak / (rms + 0.0001f);
}

void SampleToMIDIConverter::detectLimiting(const juce::AudioBuffer<float>& buffer) {
    float peak = 0.0f;
    int clippedSamples = 0;
    
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float sample = std::abs(buffer.getSample(0, i));
        peak = std::max(peak, sample);
        if (sample > 0.95f) clippedSamples++;
    }
    
    results.mastering.limitingThreshold = peak;
}

void SampleToMIDIConverter::detectPitch(const juce::AudioBuffer<float>& buffer) {
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
    
    // MIDI-Note berechnen
    float frequency = maxBin * 44100.0f / fftData.size();
    int midiNote = static_cast<int>(69 + 12 * std::log2(frequency / 440.0f));
    
    // MIDI-Note hinzufügen
    results.midiNotes.push_back(juce::MidiMessage::noteOn(1, midiNote, static_cast<uint8>(maxMagnitude * 127)));
    results.noteVelocities.push_back(maxMagnitude);
}

void SampleToMIDIConverter::detectOnsets(const juce::AudioBuffer<float>& buffer) {
    std::vector<float> onsetData(buffer.getNumSamples());
    buffer.copyFrom(0, 0, onsetData.data(), buffer.getNumSamples());
    
    // Onset-Erkennung
    std::vector<float> onsetScores;
    onsetDetector->process(onsetData, onsetScores);
    
    // Onsets in MIDI-Noten umwandeln
    for (size_t i = 0; i < onsetScores.size(); ++i) {
        if (onsetScores[i] > parameters.onsetDetectionThreshold) {
            int samplePos = i * buffer.getNumSamples() / onsetScores.size();
            float velocity = onsetScores[i];
            
            // Pitch für diesen Onset erkennen
            juce::AudioBuffer<float> onsetBuffer(1, 1024);
            buffer.copyFrom(0, samplePos, onsetBuffer, 0, 0, 1024);
            detectPitch(onsetBuffer);
            
            // Notendauer berechnen
            float duration = 0.1f; // Standard-Dauer
            if (i + 1 < onsetScores.size()) {
                duration = (onsetScores[i + 1] - onsetScores[i]) * buffer.getNumSamples() / 44100.0f;
            }
            results.noteDurations.push_back(duration);
        }
    }
}

void SampleToMIDIConverter::analyzeSpectrum(const juce::AudioBuffer<float>& buffer) {
    std::vector<float> fftData(1024);
    buffer.copyFrom(0, 0, fftData.data(), fftData.size());
    
    fft->performRealOnlyForwardTransform(fftData.data());
    
    // Spektrum analysieren
    spectralAnalyzer->process(fftData);
}

void SampleToMIDIConverter::generateMIDI() {
    // MIDI-Noten mit Effekten, Mixing und Mastering anwenden
    juce::MidiBuffer midiBuffer;
    
    // Effekte anwenden
    if (parameters.preserveEffects) {
        applyEffects(midiBuffer);
    }
    
    // Mixing anwenden
    if (parameters.preserveMixing) {
        applyMixing(midiBuffer);
    }
    
    // Mastering anwenden
    if (parameters.preserveMastering) {
        applyMastering(midiBuffer);
    }
}

void SampleToMIDIConverter::applyEffects(juce::MidiBuffer& midiBuffer) {
    // MIDI-Controller für Effekte
    for (int i = 0; i < midiBuffer.getNumEvents(); ++i) {
        juce::MidiMessage message = midiBuffer.getEventPointer(i)->message;
        
        if (message.isController()) {
            // Reverb
            if (message.getControllerNumber() == 91) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>(results.effects.reverbAmount * 127));
            }
            // Delay
            else if (message.getControllerNumber() == 94) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>(results.effects.delayTime * 127));
            }
            // Kompression
            else if (message.getControllerNumber() == 93) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>(results.effects.compressionRatio * 127));
            }
        }
    }
}

void SampleToMIDIConverter::applyMixing(juce::MidiBuffer& midiBuffer) {
    // MIDI-Controller für Mixing
    for (int i = 0; i < midiBuffer.getNumEvents(); ++i) {
        juce::MidiMessage message = midiBuffer.getEventPointer(i)->message;
        
        if (message.isController()) {
            // Panning
            if (message.getControllerNumber() == 10) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>((results.mixing.panning + 1.0f) * 63.5f));
            }
            // Volume
            else if (message.getControllerNumber() == 7) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>(results.mixing.volume * 127));
            }
        }
    }
}

void SampleToMIDIConverter::applyMastering(juce::MidiBuffer& midiBuffer) {
    // MIDI-Controller für Mastering
    for (int i = 0; i < midiBuffer.getNumEvents(); ++i) {
        juce::MidiMessage message = midiBuffer.getEventPointer(i)->message;
        
        if (message.isController()) {
            // Limiter
            if (message.getControllerNumber() == 92) {
                message = juce::MidiMessage::controllerEvent(message.getChannel(),
                    message.getControllerNumber(),
                    static_cast<uint8>(results.mastering.limitingThreshold * 127));
            }
        }
    }
}

void SampleToMIDIConverter::setParameter(const std::string& name, float value) {
    if (name == "pitch_detection_threshold") parameters.pitchDetectionThreshold = value;
    else if (name == "onset_detection_threshold") parameters.onsetDetectionThreshold = value;
    else if (name == "spectral_analysis_resolution") parameters.spectralAnalysisResolution = value;
    else if (name == "preserve_effects") parameters.preserveEffects = value > 0.5f;
    else if (name == "preserve_mixing") parameters.preserveMixing = value > 0.5f;
    else if (name == "preserve_mastering") parameters.preserveMastering = value > 0.5f;
}

float SampleToMIDIConverter::getParameter(const std::string& name) const {
    if (name == "pitch_detection_threshold") return parameters.pitchDetectionThreshold;
    else if (name == "onset_detection_threshold") return parameters.onsetDetectionThreshold;
    else if (name == "spectral_analysis_resolution") return parameters.spectralAnalysisResolution;
    else if (name == "preserve_effects") return parameters.preserveEffects ? 1.0f : 0.0f;
    else if (name == "preserve_mixing") return parameters.preserveMixing ? 1.0f : 0.0f;
    else if (name == "preserve_mastering") return parameters.preserveMastering ? 1.0f : 0.0f;
    return 0.0f;
}

} // namespace VR_DAW 