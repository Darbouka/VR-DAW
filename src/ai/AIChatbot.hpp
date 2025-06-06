#pragma once

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

namespace VR_DAW {

class TextProcessor;
class NeuralNetwork;

class AIChatbot {
public:
    AIChatbot();
    ~AIChatbot();

    void initialize();
    void shutdown();
    
    std::string processInput(const std::string& userInput);
    void processAudioOutput(std::vector<float>& buffer);
    
    void setVoiceStyle(const std::string& style);
    void setEmotion(const std::string& emotion);
    void setLanguage(const std::string& language);
    
    void enableVoiceEffects(bool enable);
    void setVoiceEffect(const std::string& effect, float value);
    
    void startRecording();
    void stopRecording();
    bool isRecording() const;
    
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const;

private:
    std::unique_ptr<TextProcessor> textProcessor;
    std::unique_ptr<NeuralNetwork> model;
    std::queue<std::vector<float>> audioQueue;
    std::mutex audioMutex;
    
    bool recording = false;
    bool playing = false;
    std::string currentVoiceStyle;
    std::string currentEmotion;
    std::string currentLanguage;
    bool voiceEffectsEnabled = true;
    
    void applyVoiceEffects(std::vector<float>& buffer);
    void processAudioQueue();
    void updateVoiceParameters();
};

} // namespace VR_DAW 