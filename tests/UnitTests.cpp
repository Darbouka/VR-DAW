#include <gtest/gtest.h>
#include "../src/audio/AudioEngine.hpp"
#include "../src/audio/DynamicsProcessor.hpp"
#include "../src/audio/PluginManager.hpp"
#include "../src/vr/VRInterface.hpp"
#include "../src/ui/VRControlPanel.hpp"
#include "../src/ui/TutorialSystem.hpp"

namespace VR_DAW {
namespace Tests {

// Audio-Engine Tests
TEST(AudioEngineTest, Initialization) {
    AudioEngine engine;
    EXPECT_TRUE(engine.initialize());
    EXPECT_EQ(engine.getSampleRate(), 44100.0);
    EXPECT_EQ(engine.getMaxBlockSize(), 512);
    EXPECT_EQ(engine.getNumChannels(), 2);
}

TEST(AudioEngineTest, PluginLoading) {
    AudioEngine engine;
    EXPECT_TRUE(engine.initialize());
    
    // Lade Test-Plugin
    EXPECT_TRUE(engine.loadPlugin("test_plugin"));
    EXPECT_TRUE(engine.isPluginLoaded("test_plugin"));
    
    // Entlade Plugin
    engine.unloadPlugin("test_plugin");
    EXPECT_FALSE(engine.isPluginLoaded("test_plugin"));
}

TEST(AudioEngineTest, AudioProcessing) {
    AudioEngine engine;
    EXPECT_TRUE(engine.initialize());
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiMessages;
    
    // Verarbeite Audio
    engine.processBlock(buffer, midiMessages);
    
    // Überprüfe Buffer
    EXPECT_EQ(buffer.getNumChannels(), 2);
    EXPECT_EQ(buffer.getNumSamples(), 512);
}

// Dynamics Processor Tests
TEST(DynamicsProcessorTest, Compressor) {
    DynamicsProcessor processor;
    
    // Setze Kompressor-Parameter
    CompressorParameters params;
    params.threshold = -20.0f;
    params.ratio = 4.0f;
    params.attack = 10.0f;
    params.release = 100.0f;
    processor.setCompressorParameters(params);
    
    // Erstelle Test-Buffer
    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    
    // Füge Test-Signal hinzu
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            buffer.setSample(channel, sample, 0.5f * sin(2.0f * M_PI * 440.0f * sample / 44100.0f));
        }
    }
    
    // Verarbeite Buffer
    processor.processBlock(buffer);
    
    // Überprüfe Kompression
    float maxSample = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            maxSample = std::max(maxSample, std::abs(buffer.getSample(channel, sample)));
        }
    }
    EXPECT_LT(maxSample, 0.5f);
}

// VR Interface Tests
TEST(VRInterfaceTest, Initialization) {
    VRInterface vr;
    EXPECT_TRUE(vr.initialize());
    EXPECT_TRUE(vr.isInitialized());
}

TEST(VRInterfaceTest, RoomManagement) {
    VRInterface vr;
    EXPECT_TRUE(vr.initialize());
    
    // Erstelle Raum
    auto roomId = vr.createRoom("Test Room", glm::vec3(0.0f), glm::vec3(5.0f, 3.0f, 5.0f));
    EXPECT_TRUE(roomId != "");
    
    // Überprüfe Raum-Eigenschaften
    auto room = vr.getRoom(roomId);
    EXPECT_EQ(room.name, "Test Room");
    EXPECT_EQ(room.size, glm::vec3(5.0f, 3.0f, 5.0f));
}

TEST(VRInterfaceTest, WindowManagement) {
    VRInterface vr;
    EXPECT_TRUE(vr.initialize());
    
    // Erstelle Fenster
    auto windowId = vr.createWindow("Test Window", glm::vec3(0.0f), glm::vec2(1.0f, 1.0f));
    EXPECT_TRUE(windowId != "");
    
    // Überprüfe Fenster-Eigenschaften
    auto window = vr.getWindow(windowId);
    EXPECT_EQ(window.title, "Test Window");
    EXPECT_EQ(window.size, glm::vec2(1.0f, 1.0f));
}

// VR Control Panel Tests
TEST(VRControlPanelTest, ControlCreation) {
    VRControlPanel panel;
    
    // Erstelle Steuerelemente
    auto buttonId = panel.addControl("Test Button", ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.2f, 0.2f));
    EXPECT_TRUE(buttonId != "");
    
    auto sliderId = panel.addControl("Test Slider", ControlType::Slider,
        glm::vec3(0.0f), glm::vec2(0.4f, 0.1f));
    EXPECT_TRUE(sliderId != "");
}

TEST(VRControlPanelTest, ControlInteraction) {
    VRControlPanel panel;
    
    // Erstelle Button
    auto buttonId = panel.addControl("Test Button", ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.2f, 0.2f));
    
    // Simuliere Interaktion
    bool clicked = false;
    panel.setControlCallback(buttonId, [&clicked]() { clicked = true; });
    
    // Simuliere Klick
    panel.handleInteraction(buttonId, InteractionType::Click);
    EXPECT_TRUE(clicked);
}

// Tutorial System Tests
TEST(TutorialSystemTest, TutorialManagement) {
    TutorialSystem tutorials;
    
    // Starte Tutorial
    tutorials.startTutorial("Grundlagen");
    EXPECT_TRUE(tutorials.isTutorialActive());
    
    // Überprüfe aktuellen Schritt
    EXPECT_EQ(tutorials.getCurrentStep(), 0);
    
    // Gehe zum nächsten Schritt
    tutorials.nextStep();
    EXPECT_EQ(tutorials.getCurrentStep(), 1);
    
    // Gehe zurück
    tutorials.previousStep();
    EXPECT_EQ(tutorials.getCurrentStep(), 0);
}

TEST(TutorialSystemTest, TutorialContent) {
    TutorialSystem tutorials;
    
    // Starte Tutorial
    tutorials.startTutorial("Grundlagen");
    
    // Überprüfe Tutorial-Inhalt
    auto tutorial = tutorials.getCurrentTutorial();
    EXPECT_EQ(tutorial.title, "Grundlagen");
    EXPECT_FALSE(tutorial.steps.empty());
    
    // Überprüfe Schritt-Inhalt
    auto step = tutorial.steps[0];
    EXPECT_EQ(step.type, "Navigation");
    EXPECT_FALSE(step.description.empty());
}

} // namespace Tests
} // namespace VR_DAW 