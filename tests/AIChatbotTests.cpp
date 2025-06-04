#include <gtest/gtest.h>
#include <chrono>
#include "../src/ai/AIChatbot.hpp"

namespace VR_DAW {
namespace Tests {

class AIChatbotTest : public ::testing::Test {
protected:
    void SetUp() override {
        chatbot = &AIChatbot::getInstance();
        chatbot->initialize();
    }
    
    void TearDown() override {
        chatbot->shutdown();
    }
    
    AIChatbot* chatbot;
};

// Chat-Funktionalität Test
TEST_F(AIChatbotTest, ChatFunctionality) {
    // Teste einfache Konversation
    auto response = chatbot->processInput("Hallo, wie geht es dir?");
    EXPECT_FALSE(response.empty());
    
    // Teste verschiedene Emotionen
    chatbot->setEmotion("Freude");
    response = chatbot->processInput("Das ist toll!");
    EXPECT_TRUE(response.find("😊") != std::string::npos);
    
    chatbot->setEmotion("Trauer");
    response = chatbot->processInput("Das ist schade.");
    EXPECT_TRUE(response.find("😢") != std::string::npos);
    
    chatbot->setEmotion("Wut");
    response = chatbot->processInput("Das ist ärgerlich!");
    EXPECT_TRUE(response.find("😠") != std::string::npos);
}

// Sprachausgabe Test
TEST_F(AIChatbotTest, VoiceOutput) {
    // Teste verschiedene Stimmen
    chatbot->setVoiceStyle("Robot");
    auto response = chatbot->processInput("Ich bin ein Roboter.");
    
    // Überprüfe Audio-Ausgabe
    juce::AudioBuffer<float> buffer(2, 44100); // 1 Sekunde Stereo
    chatbot->processAudioOutput(buffer);
    EXPECT_GT(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f);
    
    // Teste verschiedene Sprachen
    chatbot->setLanguage("Englisch");
    response = chatbot->processInput("Hello, how are you?");
    EXPECT_FALSE(response.empty());
    
    chatbot->setLanguage("Französisch");
    response = chatbot->processInput("Bonjour, comment allez-vous?");
    EXPECT_FALSE(response.empty());
}

// Parameter Test
TEST_F(AIChatbotTest, Parameters) {
    // Teste Lautstärke
    chatbot->setVolume(0.5f);
    juce::AudioBuffer<float> buffer(2, 44100);
    chatbot->processAudioOutput(buffer);
    EXPECT_LE(buffer.getMagnitude(0, buffer.getNumSamples()), 0.5f);
    
    // Teste Tonhöhe
    chatbot->setPitch(1.5f);
    chatbot->processAudioOutput(buffer);
    
    // Teste Sprechgeschwindigkeit
    chatbot->setSpeechRate(1.2f);
    chatbot->processAudioOutput(buffer);
}

// KI-Modell Test
TEST_F(AIChatbotTest, ModelFunctionality) {
    // Lade Modell
    chatbot->loadModel("models/chatbot_model.bin");
    
    // Teste Antwortgenerierung
    auto response = chatbot->processInput("Was ist deine Lieblingsfarbe?");
    EXPECT_FALSE(response.empty());
    
    // Teste Kontextverständnis
    response = chatbot->processInput("Warum magst du diese Farbe?");
    EXPECT_FALSE(response.empty());
    
    // Teste Speichern des Modells
    chatbot->saveModel("models/chatbot_model_updated.bin");
}

// Performance Test
TEST_F(AIChatbotTest, Performance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Führe mehrere Anfragen aus
    for (int i = 0; i < 10; ++i) {
        auto response = chatbot->processInput("Test " + std::to_string(i));
        EXPECT_FALSE(response.empty());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe, dass die Verarbeitung nicht zu lange dauert
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 10 Anfragen
}

} // namespace Tests
} // namespace VR_DAW 