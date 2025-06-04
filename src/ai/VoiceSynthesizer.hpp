#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace VR_DAW {

class VoiceModel;
class PhonemeConverter;
class Vocoder;
class VoiceEditor;

class VoiceSynthesizer {
public:
    VoiceSynthesizer();
    ~VoiceSynthesizer();
    
    // Sprachsynthese
    juce::AudioBuffer<float> synthesize(const std::string& text);
    void setVoiceStyle(const std::string& style);
    void setLanguage(const std::string& language);
    void setEmotion(const std::string& emotion);
    
    // Synchronsprecher-Funktionen
    void setVoiceActor(const std::string& actorName);
    std::vector<std::string> getAvailableVoiceActors() const;
    void loadVoiceActorModel(const std::string& actorName);
    void trainVoiceActorModel(const std::string& actorName, const std::string& trainingDataPath);
    
    // Parameter
    void setVolume(float volume);
    void setPitch(float pitch);
    void setSpeechRate(float rate);
    void setFormantShift(float shift);
    
    // Konfiguration
    void loadModel(const std::string& modelPath);
    void saveModel(const std::string& modelPath);
    void trainModel(const std::string& trainingDataPath);
    
    // Neue Optimierungs- und Testfunktionen
    void optimizeVoiceParameters();
    bool testVoiceQuality(const std::string& actorName);
    bool checkAudioQuality(const juce::AudioBuffer<float>& buffer);
    void runComprehensiveTest();
    
    // Neue Vocoder-Funktionen
    void initializeVocoder();
    void setVocoderMode(const std::string& mode); // "Robot", "Choir", "Whisper", etc.
    void setVocoderCarrier(const std::string& voiceActor);
    void processVocoderInput(const juce::AudioBuffer<float>& inputBuffer);
    juce::AudioBuffer<float> getVocoderOutput() const;
    
    // Neue Voice Editor Funktionen
    void initializeVoiceEditor();
    void setVoiceEditorMode(const std::string& mode); // "Pitch", "Formant", "Timbre", etc.
    void setVoiceEditorTarget(const std::string& voiceActor);
    void processVoiceEditorInput(const juce::AudioBuffer<float>& inputBuffer);
    juce::AudioBuffer<float> getVoiceEditorOutput() const;
    
    // Echtzeit-Verarbeitung
    void startRealtimeProcessing();
    void stopRealtimeProcessing();
    void setRealtimeMode(const std::string& mode); // "Vocoder", "Editor", "Both"
    void processRealtimeInput(const juce::AudioBuffer<float>& inputBuffer);
    juce::AudioBuffer<float> getRealtimeOutput() const;
    
    // Parameter für Echtzeit-Verarbeitung
    void setRealtimeLatency(int milliseconds);
    void setRealtimeBufferSize(int samples);
    void setRealtimeSampleRate(double sampleRate);
    
    // Callback für Echtzeit-Verarbeitung
    using RealtimeCallback = std::function<void(const juce::AudioBuffer<float>&)>;
    void setRealtimeCallback(RealtimeCallback callback);

    // Sprachauswahl für Synchronsprecher
    enum class VoiceLanguage {
        German,     // Deutsche Synchro
        Original,   // Originalstimme
        Both        // Beide Sprachen
    };

    // Neue Sprachauswahl-Funktionen
    void setVoiceLanguage(const std::string& actorName, VoiceLanguage language);
    VoiceLanguage getVoiceLanguage(const std::string& actorName) const;
    std::vector<std::string> getAvailableLanguages(const std::string& actorName) const;
    
    // Sprachspezifische Parameter
    struct LanguageParameters {
        float accentStrength = 0.0f;      // Stärke des Akzents
        float pronunciationAccuracy = 1.0f; // Aussprachegenauigkeit
        float languageBlend = 0.0f;       // Mischung zwischen Sprachen
    };

    // Sprachparameter setzen
    void setLanguageParameters(const std::string& actorName, const LanguageParameters& params);
    LanguageParameters getLanguageParameters(const std::string& actorName) const;

private:
    // KI-Modell
    std::unique_ptr<VoiceModel> voiceModel;
    std::unique_ptr<PhonemeConverter> phonemeConverter;
    
    // Synchronsprecher-Modelle
    std::map<std::string, std::unique_ptr<VoiceModel>> voiceActorModels;
    std::string currentVoiceActor;
    
    // Parameter
    struct Parameters {
        std::string voiceStyle = "Neutral";
        std::string language = "Deutsch";
        std::string emotion = "Neutral";
        float volume = 1.0f;
        float pitch = 1.0f;
        float speechRate = 1.0f;
        float formantShift = 1.0f;
        
        // Synchronsprecher-spezifische Parameter
        struct VoiceActorParams {
            float basePitch = 1.0f;
            float baseFormantShift = 1.0f;
            float baseSpeechRate = 1.0f;
            float voiceTimbre = 1.0f;
            float voiceRoughness = 0.0f;
            float voiceBreathiness = 0.0f;
        } voiceActorParams;
    } parameters;
    
    // Interne Hilfsfunktionen
    void initializeModels();
    std::vector<std::string> textToPhonemes(const std::string& text);
    juce::AudioBuffer<float> phonemesToAudio(const std::vector<std::string>& phonemes);
    void applyVoiceStyle(juce::AudioBuffer<float>& buffer);
    void applyEmotion(juce::AudioBuffer<float>& buffer);
    void applyVoiceActorParameters(juce::AudioBuffer<float>& buffer);
    void initializeVoiceActorParameters();
    
    // Neue private Member
    std::unique_ptr<Vocoder> vocoder;
    std::unique_ptr<VoiceEditor> voiceEditor;
    
    // Echtzeit-Verarbeitung
    bool isRealtimeProcessing;
    std::string realtimeMode;
    int realtimeLatency;
    int realtimeBufferSize;
    double realtimeSampleRate;
    RealtimeCallback realtimeCallback;
    
    // Interne Hilfsfunktionen
    void initializeRealtimeProcessing();
    void processRealtimeBuffer(const juce::AudioBuffer<float>& inputBuffer);
    void applyRealtimeEffects(juce::AudioBuffer<float>& buffer);
    
    struct RealtimeParameters {
        bool isActive = false;
        std::string currentMode = "Vocoder";
        std::string currentVoiceActor;
        float processingLatency = 0.0f;
        int bufferSize = 1024;
        double sampleRate = 44100.0;
        
        struct VocoderParams {
            float carrierMix = 0.5f;
            float modulatorMix = 0.5f;
            float formantShift = 1.0f;
            float pitchShift = 1.0f;
            int numBands = 16;
        } vocoderParams;
        
        struct EditorParams {
            float pitchShift = 1.0f;
            float formantShift = 1.0f;
            float timbreShift = 1.0f;
            float breathiness = 0.0f;
            float roughness = 0.0f;
        } editorParams;
    } realtimeParams;

    // Sprachspezifische Daten
    struct VoiceActorData {
        VoiceLanguage currentLanguage = VoiceLanguage::German;
        LanguageParameters languageParams;
        std::map<VoiceLanguage, std::unique_ptr<VoiceModel>> languageModels;
        std::string originalLanguage;  // z.B. "English", "French", etc.
    };

    std::map<std::string, VoiceActorData> voiceActorData;

    // Sprachspezifische Hilfsfunktionen
    void initializeLanguageModels(const std::string& actorName);
    void loadLanguageModel(const std::string& actorName, VoiceLanguage language);
    void blendLanguages(const std::string& actorName, float blendFactor);
    void applyLanguageParameters(const std::string& actorName, const LanguageParameters& params);
};

} // namespace VR_DAW 