#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../vr/VRInterface.hpp"
#include "../audio/AudioEngine.hpp"

namespace VR_DAW {

class VRControlPanel {
public:
    VRControlPanel();
    ~VRControlPanel();

    // Kontrollelement-Typen
    enum class ControlType {
        Button,
        Slider,
        Knob,
        Toggle,
        Menu,
        Display,
        Meter,
        Waveform,
        Spectrum
    };

    // Kontrollelement-Struktur
    struct Control {
        std::string id;
        std::string label;
        ControlType type;
        glm::vec3 position;
        glm::vec3 size;
        glm::quat rotation;
        bool isVisible;
        bool isInteractive;
        std::function<void(const Control&)> callback;
    };

    // Hauptfunktionen
    void initialize();
    void update();
    void render();
    void handleInteraction(const VRInterface::MotionData& motionData);

    // Kontrollelement-Management
    void addControl(const Control& control);
    void removeControl(const std::string& id);
    void updateControl(const Control& control);
    void setControlVisibility(const std::string& id, bool visible);
    void setControlInteraction(const std::string& id, bool interactive);

    // Layout-Management
    void setLayout(const std::string& layoutName);
    void saveLayout(const std::string& layoutName);
    void loadLayout(const std::string& layoutName);

    // Audio-Integration
    void connectToAudioEngine(AudioEngine* engine);
    void updateAudioParameters(const std::string& controlId, float value);

private:
    // Kontrollelemente
    std::vector<Control> controls;
    std::map<std::string, Control> controlRegistry;
    std::map<std::string, std::vector<Control>> layouts;

    // VR-Integration
    VRInterface* vrInterface;
    AudioEngine* audioEngine;

    // Rendering
    struct RenderData {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        GLuint texture;
    };
    std::map<std::string, RenderData> renderData;

    // Interaktion
    struct InteractionState {
        bool isGrabbing;
        std::string grabbedControl;
        glm::vec3 grabOffset;
    };
    InteractionState interactionState;

    // Hilfsfunktionen
    void initializeRendering();
    void updateRendering();
    void handleControlInteraction(const Control& control, const VRInterface::MotionData& motionData);
    void updateControlPosition(const std::string& id, const glm::vec3& position);
    void updateControlRotation(const std::string& id, const glm::quat& rotation);
    void updateControlSize(const std::string& id, const glm::vec3& size);
    void renderControl(const Control& control);
    void renderButton(const Control& control);
    void renderSlider(const Control& control);
    void renderKnob(const Control& control);
    void renderToggle(const Control& control);
    void renderMenu(const Control& control);
    void renderDisplay(const Control& control);
    void renderMeter(const Control& control);
    void renderWaveform(const Control& control);
    void renderSpectrum(const Control& control);
};

} // namespace VR_DAW 