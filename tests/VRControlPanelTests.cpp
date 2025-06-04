#include <gtest/gtest.h>
#include <chrono>
#include "../src/ui/VRControlPanel.hpp"

namespace VR_DAW {
namespace Tests {

class VRControlPanelTest : public ::testing::Test {
protected:
    void SetUp() override {
        panel = std::make_unique<VRControlPanel>();
        panel->initialize();
    }
    
    void TearDown() override {
        panel->shutdown();
    }
    
    std::unique_ptr<VRControlPanel> panel;
};

// Initialisierung Tests
TEST_F(VRControlPanelTest, Initialization) {
    EXPECT_TRUE(panel->isInitialized());
    EXPECT_TRUE(panel->getControlCount() == 0);
}

// Control-Element Tests
TEST_F(VRControlPanelTest, ControlCreation) {
    // Erstelle verschiedene Control-Elemente
    auto buttonId = panel->addControl("Test Button", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.1f, 0.1f));
    EXPECT_TRUE(buttonId != "");
    
    auto sliderId = panel->addControl("Test Slider", VRControlPanel::ControlType::Slider,
        glm::vec3(0.2f), glm::vec2(0.2f, 0.1f));
    EXPECT_TRUE(sliderId != "");
    
    auto knobId = panel->addControl("Test Knob", VRControlPanel::ControlType::Knob,
        glm::vec3(0.4f), glm::vec2(0.1f, 0.1f));
    EXPECT_TRUE(knobId != "");
    
    EXPECT_EQ(panel->getControlCount(), 3);
}

// Interaktion Tests
TEST_F(VRControlPanelTest, InteractionHandling) {
    // Erstelle Button
    auto buttonId = panel->addControl("Test Button", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.1f, 0.1f));
    
    bool buttonClicked = false;
    panel->setControlCallback(buttonId, [&buttonClicked]() {
        buttonClicked = true;
    });
    
    // Simuliere Button-Klick
    panel->handleInteraction(buttonId, VRControlPanel::InteractionType::Click);
    EXPECT_TRUE(buttonClicked);
}

// Layout-Management Tests
TEST_F(VRControlPanelTest, LayoutManagement) {
    // Erstelle Layout
    std::string layoutName = "Test Layout";
    panel->saveLayout(layoutName);
    
    // Überprüfe Layout-Speicherung
    auto layouts = panel->getAvailableLayouts();
    EXPECT_TRUE(std::find(layouts.begin(), layouts.end(), layoutName) != layouts.end());
    
    // Lade Layout
    panel->loadLayout(layoutName);
    EXPECT_EQ(panel->getCurrentLayout(), layoutName);
}

// Rendering-Performance Tests
TEST_F(VRControlPanelTest, RenderingPerformance) {
    // Erstelle viele Controls
    for (int i = 0; i < 100; ++i) {
        panel->addControl("Control " + std::to_string(i),
            VRControlPanel::ControlType::Button,
            glm::vec3(i * 0.1f), glm::vec2(0.1f, 0.1f));
    }
    
    // Teste Rendering-Performance
    auto start = std::chrono::high_resolution_clock::now();
    
    // Rendere 100 Frames
    for (int i = 0; i < 100; ++i) {
        panel->render();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 100 Frames
}

// Audio-Integration Tests
TEST_F(VRControlPanelTest, AudioIntegration) {
    // Erstelle Audio-Controls
    auto volumeId = panel->addControl("Volume", VRControlPanel::ControlType::Slider,
        glm::vec3(0.0f), glm::vec2(0.2f, 0.1f));
    
    auto panId = panel->addControl("Pan", VRControlPanel::ControlType::Knob,
        glm::vec3(0.3f), glm::vec2(0.1f, 0.1f));
    
    // Verbinde mit Audio-Parametern
    panel->connectToAudioParameter(volumeId, "volume");
    panel->connectToAudioParameter(panId, "pan");
    
    // Teste Parameter-Updates
    panel->updateAudioParameter("volume", 0.5f);
    panel->updateAudioParameter("pan", -0.5f);
    
    auto volumeValue = panel->getAudioParameterValue("volume");
    auto panValue = panel->getAudioParameterValue("pan");
    
    EXPECT_FLOAT_EQ(volumeValue, 0.5f);
    EXPECT_FLOAT_EQ(panValue, -0.5f);
}

// Sichtbarkeit Tests
TEST_F(VRControlPanelTest, VisibilityControl) {
    auto controlId = panel->addControl("Test Control", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.1f, 0.1f));
    
    // Teste Sichtbarkeit
    panel->setControlVisibility(controlId, false);
    EXPECT_FALSE(panel->isControlVisible(controlId));
    
    panel->setControlVisibility(controlId, true);
    EXPECT_TRUE(panel->isControlVisible(controlId));
}

// Interaktivität Tests
TEST_F(VRControlPanelTest, InteractivityControl) {
    auto controlId = panel->addControl("Test Control", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.1f, 0.1f));
    
    // Teste Interaktivität
    panel->setControlInteractivity(controlId, false);
    EXPECT_FALSE(panel->isControlInteractive(controlId));
    
    panel->setControlInteractivity(controlId, true);
    EXPECT_TRUE(panel->isControlInteractive(controlId));
}

// Fehlerbehandlung Tests
TEST_F(VRControlPanelTest, ErrorHandling) {
    // Teste ungültige Control-Parameter
    EXPECT_FALSE(panel->addControl("", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(0.0f)).isValid());
    
    EXPECT_FALSE(panel->addControl("Test", VRControlPanel::ControlType::Button,
        glm::vec3(0.0f), glm::vec2(-1.0f)).isValid());
    
    // Teste ungültige Layout-Operationen
    EXPECT_FALSE(panel->loadLayout("NonExistentLayout"));
    
    // Teste ungültige Audio-Parameter
    EXPECT_FALSE(panel->connectToAudioParameter("invalidId", "volume"));
    EXPECT_FALSE(panel->updateAudioParameter("invalidParam", 0.5f));
}

} // namespace Tests
} // namespace VR_DAW 