#include <gtest/gtest.h>
#include "../src/ai/VoiceSynthesizer.hpp"

namespace VR_DAW {
namespace Tests {

class VoiceSynthesizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        synthesizer = std::make_unique<VoiceSynthesizer>();
    }
    
    void TearDown() override {
        synthesizer.reset();
    }
    
    std::unique_ptr<VoiceSynthesizer> synthesizer;
};

// Synchronsprecher-Verfügbarkeit Test
TEST_F(VoiceSynthesizerTest, VoiceActorAvailability) {
    auto actors = synthesizer->getAvailableVoiceActors();
    
    // Überprüfe, ob alle erwarteten Synchronsprecher verfügbar sind
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Vin Diesel") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Dwayne Johnson") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Darth Vader") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Luke Skywalker") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Obi-Wan Kenobi") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Tomb Raider") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Iron Man") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Captain America") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Black Widow") != actors.end());
    EXPECT_TRUE(std::find(actors.begin(), actors.end(), "Thor") != actors.end());
}

// Synchronsprecher-Wechsel Test
TEST_F(VoiceSynthesizerTest, VoiceActorSwitching) {
    // Teste verschiedene Synchronsprecher
    synthesizer->setVoiceActor("Vin Diesel");
    auto buffer1 = synthesizer->synthesize("Ich bin Vin Diesel.");
    EXPECT_GT(buffer1.getMagnitude(0, buffer1.getNumSamples()), 0.0f);
    
    synthesizer->setVoiceActor("Darth Vader");
    auto buffer2 = synthesizer->synthesize("Ich bin dein Vater.");
    EXPECT_GT(buffer2.getMagnitude(0, buffer2.getNumSamples()), 0.0f);
    
    // Überprüfe, dass die Stimmen unterschiedlich sind
    EXPECT_NE(buffer1.getMagnitude(0, buffer1.getNumSamples()),
              buffer2.getMagnitude(0, buffer2.getNumSamples()));
}

// Modell-Laden Test
TEST_F(VoiceSynthesizerTest, ModelLoading) {
    // Teste Laden der Synchronsprecher-Modelle
    synthesizer->loadVoiceActorModel("Vin Diesel");
    synthesizer->loadVoiceActorModel("Darth Vader");
    synthesizer->loadVoiceActorModel("Iron Man");
    
    // Überprüfe, ob die Modelle geladen wurden
    auto buffer = synthesizer->synthesize("Test");
    EXPECT_GT(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f);
}

// Parameter-Anpassung Test
TEST_F(VoiceSynthesizerTest, ParameterAdjustment) {
    synthesizer->setVoiceActor("Vin Diesel");
    
    // Teste verschiedene Parameter
    synthesizer->setVolume(0.5f);
    auto buffer1 = synthesizer->synthesize("Test 1");
    
    synthesizer->setPitch(1.5f);
    auto buffer2 = synthesizer->synthesize("Test 2");
    
    synthesizer->setSpeechRate(1.2f);
    auto buffer3 = synthesizer->synthesize("Test 3");
    
    // Überprüfe, dass die Parameter wirksam wurden
    EXPECT_NE(buffer1.getMagnitude(0, buffer1.getNumSamples()),
              buffer2.getMagnitude(0, buffer2.getNumSamples()));
    EXPECT_NE(buffer2.getMagnitude(0, buffer2.getNumSamples()),
              buffer3.getMagnitude(0, buffer3.getNumSamples()));
}

// Performance Test
TEST_F(VoiceSynthesizerTest, Performance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Teste verschiedene Synchronsprecher in schneller Folge
    std::vector<std::string> actors = {
        "Vin Diesel", "Darth Vader", "Iron Man", "Thor"
    };
    
    for (const auto& actor : actors) {
        synthesizer->setVoiceActor(actor);
        auto buffer = synthesizer->synthesize("Performance Test");
        EXPECT_GT(buffer.getMagnitude(0, buffer.getNumSamples()), 0.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe, dass die Verarbeitung nicht zu lange dauert
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für alle Tests
}

} // namespace Tests
} // namespace VR_DAW 