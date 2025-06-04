#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace VR_DAW {

class TextProcessor {
public:
    TextProcessor();
    ~TextProcessor();
    
    // Text-Verarbeitung
    std::string process(const std::string& input);
    void setLanguage(const std::string& language);
    void setEmotion(const std::string& emotion);
    
    // Tokenisierung und Analyse
    std::vector<std::string> tokenize(const std::string& text);
    std::vector<float> getEmbedding(const std::string& text);
    
    // Konfiguration
    void loadModel(const std::string& modelPath);
    void saveModel(const std::string& modelPath);
    void trainModel(const std::string& trainingDataPath);
    
private:
    // KI-Modell
    std::unique_ptr<class LanguageModel> languageModel;
    std::unique_ptr<class EmbeddingModel> embeddingModel;
    
    // Parameter
    struct Parameters {
        std::string language = "Deutsch";
        std::string emotion = "Neutral";
        float temperature = 0.7f;
        int maxTokens = 100;
    } parameters;
    
    // Interne Hilfsfunktionen
    void initializeModels();
    std::string preprocess(const std::string& text);
    std::string postprocess(const std::string& text);
    void applyEmotion(const std::string& text);
};

} // namespace VR_DAW 