#pragma once

#include <memory>
#include <vector>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "VoiceEditor.hpp"

namespace VR_DAW {

class SampleToMIDIConverter {
public:
    SampleToMIDIConverter();
    ~SampleToMIDIConverter();
    
    // Initialisierung und Shutdown
    void initialize();
    void shutdown();
    
    // Audio-zu-MIDI Konvertierung
    void convertSampleToMIDI(const std::string& inputPath, const std::string& outputPath);
    void analyzeAndConvert(const juce::AudioBuffer<float>& buffer);
    
    // Effekt-Analyse
    void analyzeEffects(const juce::AudioBuffer<float>& buffer);
    void detectReverb(const juce::AudioBuffer<float>& buffer);
    void detectDelay(const juce::AudioBuffer<float>& buffer);
    void detectCompression(const juce::AudioBuffer<float>& buffer);
    void detectEQ(const juce::AudioBuffer<float>& buffer);
    
    // Mixing-Analyse
    void analyzeMixing(const juce::AudioBuffer<float>& buffer);
    void detectPanning(const juce::AudioBuffer<float>& buffer);
    void detectVolume(const juce::AudioBuffer<float>& buffer);
    void detectStereoWidth(const juce::AudioBuffer<float>& buffer);
    
    // Mastering-Analyse
    void analyzeMastering(const juce::AudioBuffer<float>& buffer);
    void detectLoudness(const juce::AudioBuffer<float>& buffer);
    void detectDynamicRange(const juce::AudioBuffer<float>& buffer);
    void detectLimiting(const juce::AudioBuffer<float>& buffer);
    
    // Parameter
    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;
    
private:
    // DSP-Komponenten
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::PitchDetector> pitchDetector;
    std::unique_ptr<juce::dsp::OnsetDetector> onsetDetector;
    std::unique_ptr<juce::dsp::SpectralAnalyzer> spectralAnalyzer;
    
    // Analyse-Ergebnisse
    struct AnalysisResults {
        // MIDI-Konvertierung
        std::vector<juce::MidiMessage> midiNotes;
        std::vector<float> noteVelocities;
        std::vector<float> noteDurations;
        
        // Effekt-Analyse
        struct {
            float reverbAmount = 0.0f;
            float reverbSize = 0.0f;
            float delayTime = 0.0f;
            float delayFeedback = 0.0f;
            float compressionThreshold = 0.0f;
            float compressionRatio = 0.0f;
            std::vector<float> eqBands;
        } effects;
        
        // Mixing-Analyse
        struct {
            float panning = 0.0f;
            float volume = 0.0f;
            float stereoWidth = 0.0f;
        } mixing;
        
        // Mastering-Analyse
        struct {
            float loudness = 0.0f;
            float dynamicRange = 0.0f;
            float limitingThreshold = 0.0f;
        } mastering;
    } results;
    
    // Verarbeitungsparameter
    struct ProcessingParameters {
        float pitchDetectionThreshold = 0.5f;
        float onsetDetectionThreshold = 0.5f;
        float spectralAnalysisResolution = 0.1f;
        bool preserveEffects = true;
        bool preserveMixing = true;
        bool preserveMastering = true;
    } parameters;
    
    // Interne Hilfsfunktionen
    void initializeDSP();
    void detectPitch(const juce::AudioBuffer<float>& buffer);
    void detectOnsets(const juce::AudioBuffer<float>& buffer);
    void analyzeSpectrum(const juce::AudioBuffer<float>& buffer);
    void generateMIDI();
    void applyEffects(juce::MidiBuffer& midiBuffer);
    void applyMixing(juce::MidiBuffer& midiBuffer);
    void applyMastering(juce::MidiBuffer& midiBuffer);
};

} // namespace VR_DAW 