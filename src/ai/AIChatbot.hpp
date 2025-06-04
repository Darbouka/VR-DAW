#pragma once

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../audio/VoiceEditor.hpp"

namespace VR_DAW {

class AIChatbot {
public:
    static AIChatbot& getInstance();
    
    // Initialisierung und Shutdown
    void initialize();
    void shutdown();
    
    // Chat-Funktionen
    std::string processInput(const std::string& userInput);
    void setVoiceStyle(const std::string& style);
    void setLanguage(const std::string& language);
    void setEmotion(const std::string& emotion);
    
    // Audio-Verarbeitung
    void processAudioOutput(juce::AudioBuffer<float>& buffer);
    void setVolume(float volume);
    void setPitch(float pitch);
    void setSpeechRate(float rate);
    
    // Konfiguration
    void loadModel(const std::string& modelPath);
    void saveModel(const std::string& modelPath);
    void trainModel(const std::string& trainingDataPath);
    
private:
    AIChatbot() = default;
    ~AIChatbot() = default;
    
    // KI-Modell
    std::unique_ptr<class NeuralNetwork> model;
    std::unique_ptr<class TextProcessor> textProcessor;
    std::unique_ptr<class VoiceSynthesizer> voiceSynthesizer;
    
    // Audio-Verarbeitung
    std::unique_ptr<VoiceEditor> voiceEditor;
    std::queue<juce::AudioBuffer<float>> audioQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    
    // Parameter
    struct Parameters {
        std::string voiceStyle = "Neutral";
        std::string language = "Deutsch";
        std::string emotion = "Neutral";
        float volume = 1.0f;
        float pitch = 1.0f;
        float speechRate = 1.0f;
    } parameters;
    
    // Interne Hilfsfunktionen
    void initializeDSP();
    void processTextToSpeech(const std::string& text);
    void synthesizeVoice(const std::string& text);
    void applyVoiceEffects(juce::AudioBuffer<float>& buffer);
};

} // namespace VR_DAW 