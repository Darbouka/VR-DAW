#include "TextProcessor.hpp"
#include <algorithm>
#include <regex>
#include <sstream>
#include <locale>
#include <codecvt>

namespace VR_DAW {

TextProcessor::TextProcessor() {
    initializeModels();
}

TextProcessor::~TextProcessor() = default;

void TextProcessor::initializeModels() {
    languageModel = std::make_unique<LanguageModel>();
    embeddingModel = std::make_unique<EmbeddingModel>();
}

std::string TextProcessor::process(const std::string& input) {
    // Text vorverarbeiten
    auto preprocessed = preprocess(input);
    
    // Tokenisierung
    auto tokens = tokenize(preprocessed);
    
    // Embedding generieren
    auto embedding = getEmbedding(preprocessed);
    
    // Sprache und Emotion anwenden
    applyEmotion(preprocessed);
    
    // Antwort generieren
    auto response = languageModel->generateResponse(embedding, parameters.temperature, parameters.maxTokens);
    
    // Antwort nachbearbeiten
    return postprocess(response);
}

void TextProcessor::setLanguage(const std::string& language) {
    parameters.language = language;
    if (languageModel) {
        languageModel->setLanguage(language);
    }
}

void TextProcessor::setEmotion(const std::string& emotion) {
    parameters.emotion = emotion;
}

std::vector<std::string> TextProcessor::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (std::getline(iss, token, ' ')) {
        // Token bereinigen
        token = std::regex_replace(token, std::regex("[^\\w\\s]"), "");
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::vector<float> TextProcessor::getEmbedding(const std::string& text) {
    if (!embeddingModel) return std::vector<float>();
    return embeddingModel->getEmbedding(text);
}

void TextProcessor::loadModel(const std::string& modelPath) {
    if (languageModel) {
        languageModel->load(modelPath);
    }
    if (embeddingModel) {
        embeddingModel->load(modelPath);
    }
}

void TextProcessor::saveModel(const std::string& modelPath) {
    if (languageModel) {
        languageModel->save(modelPath);
    }
    if (embeddingModel) {
        embeddingModel->save(modelPath);
    }
}

void TextProcessor::trainModel(const std::string& trainingDataPath) {
    if (languageModel && embeddingModel) {
        // Trainingsdaten laden und verarbeiten
        // Hier würde die Implementierung des Trainings folgen
    }
}

std::string TextProcessor::preprocess(const std::string& text) {
    std::string processed = text;
    
    // Groß-/Kleinschreibung normalisieren
    std::transform(processed.begin(), processed.end(), processed.begin(), ::tolower);
    
    // Sonderzeichen entfernen
    processed = std::regex_replace(processed, std::regex("[^\\w\\s]"), "");
    
    // Mehrfache Leerzeichen entfernen
    processed = std::regex_replace(processed, std::regex("\\s+"), " ");
    
    // Trim
    processed.erase(0, processed.find_first_not_of(" "));
    processed.erase(processed.find_last_not_of(" ") + 1);
    
    return processed;
}

std::string TextProcessor::postprocess(const std::string& text) {
    std::string processed = text;
    
    // Satzzeichen wiederherstellen
    processed = std::regex_replace(processed, std::regex("\\s+([.,!?])"), "$1");
    
    // Ersten Buchstaben groß schreiben
    if (!processed.empty()) {
        processed[0] = std::toupper(processed[0]);
    }
    
    // Emotionale Anpassungen
    if (parameters.emotion == "Freude") {
        processed = "😊 " + processed + " 😊";
    } else if (parameters.emotion == "Trauer") {
        processed = "😢 " + processed + " 😢";
    } else if (parameters.emotion == "Wut") {
        processed = "😠 " + processed + " 😠";
    }
    
    return processed;
}

void TextProcessor::applyEmotion(const std::string& text) {
    if (!languageModel) return;
    
    // Emotionale Parameter anpassen
    if (parameters.emotion == "Freude") {
        parameters.temperature = 0.8f;
        languageModel->setEmotionalBias(1.2f);
    } else if (parameters.emotion == "Trauer") {
        parameters.temperature = 0.6f;
        languageModel->setEmotionalBias(0.8f);
    } else if (parameters.emotion == "Wut") {
        parameters.temperature = 0.9f;
        languageModel->setEmotionalBias(1.5f);
    } else {
        parameters.temperature = 0.7f;
        languageModel->setEmotionalBias(1.0f);
    }
}

} // namespace VR_DAW 