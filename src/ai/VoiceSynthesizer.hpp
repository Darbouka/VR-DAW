#pragma once

#include <string>
#include <vector>
#include <map>

namespace VR_DAW {

class VoiceSynthesizer {
public:
    VoiceSynthesizer();
    ~VoiceSynthesizer();
    
    std::vector<float> synthesize(const std::string& text);
    void setVoiceStyle(const std::string& style);
    void setLanguage(const std::string& language);
    void setEmotion(const std::string& emotion);
    void setVoiceActor(const std::string& actorName);
    std::vector<std::string> getAvailableVoiceActors() const;
    void loadVoiceActorModel(const std::string& actorName);
    void trainVoiceActorModel(const std::string& actorName, const std::string& trainingDataPath);
    void setVolume(float volume);
    void setPitch(float pitch);
    void setSpeechRate(float rate);
    void setFormantShift(float shift);
    void loadModel(const std::string& modelPath);
    void saveModel(const std::string& modelPath);
    void trainModel(const std::string& trainingDataPath);
    void optimizeVoiceParameters();
    bool testVoiceQuality(const std::string& actorName);
    bool checkAudioQuality(const std::vector<float>& buffer);
    void runComprehensiveTest();
    void initializeVocoder();
    void setVocoderMode(const std::string& mode);
    void setVocoderCarrier(const std::string& voiceActor);
    void processVocoderInput(const std::vector<float>& inputBuffer);
    std::vector<float> getVocoderOutput() const;
private:
    std::string currentVoiceActor;
    struct Parameters {
        std::string voiceStyle = "Neutral";
        std::string language = "Deutsch";
        std::string emotion = "Neutral";
        float volume = 1.0f;
        float pitch = 1.0f;
        float speechRate = 1.0f;
        float formantShift = 1.0f;
        struct VoiceActorParams {
            float basePitch = 1.0f;
            float baseFormantShift = 1.0f;
            float baseSpeechRate = 1.0f;
            float voiceTimbre = 1.0f;
            float voiceRoughness = 0.0f;
            float voiceBreathiness = 0.0f;
        } voiceActorParams;
    } parameters;
};

} // namespace VR_DAW 