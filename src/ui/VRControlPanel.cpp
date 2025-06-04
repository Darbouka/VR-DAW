#include "VRControlPanel.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <juce_gui_extra/juce_gui_extra.h>

namespace VR_DAW {

VRControlPanel::VRControlPanel() {
    // Initialisiere Standard-Layouts
    initializeDefaultLayouts();
}

VRControlPanel::~VRControlPanel() {
    // Bereinige OpenGL-Ressourcen
    for (auto& [id, data] : renderData) {
        glDeleteVertexArrays(1, &data.vao);
        glDeleteBuffers(1, &data.vbo);
        glDeleteBuffers(1, &data.ebo);
        glDeleteTextures(1, &data.texture);
    }
}

void VRControlPanel::initialize() {
    // Initialisiere Rendering
    initializeRendering();
    
    // Erstelle Standard-Kontrollelemente
    createDefaultControls();
}

void VRControlPanel::initializeRendering() {
    // Erstelle Shader-Programme
    // TODO: Implementiere Shader-Programme
    
    // Erstelle Standard-Meshes
    // TODO: Implementiere Standard-Meshes
}

void VRControlPanel::createDefaultControls() {
    // Erstelle Mixer-Kontrollelemente
    Control mixerControl;
    mixerControl.id = "mixer";
    mixerControl.label = "Mixer";
    mixerControl.type = ControlType::Menu;
    mixerControl.position = glm::vec3(0.0f, 1.5f, -2.0f);
    mixerControl.size = glm::vec3(1.0f, 0.5f, 0.1f);
    mixerControl.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    mixerControl.isVisible = true;
    mixerControl.isInteractive = true;
    addControl(mixerControl);
    
    // Erstelle Effekt-Kontrollelemente
    Control effectsControl;
    effectsControl.id = "effects";
    effectsControl.label = "Effekte";
    effectsControl.type = ControlType::Menu;
    effectsControl.position = glm::vec3(1.0f, 1.5f, -2.0f);
    effectsControl.size = glm::vec3(1.0f, 0.5f, 0.1f);
    effectsControl.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    effectsControl.isVisible = true;
    effectsControl.isInteractive = true;
    addControl(effectsControl);
    
    // Erstelle Transport-Kontrollelemente
    Control transportControl;
    transportControl.id = "transport";
    transportControl.label = "Transport";
    transportControl.type = ControlType::Menu;
    transportControl.position = glm::vec3(-1.0f, 1.5f, -2.0f);
    transportControl.size = glm::vec3(1.0f, 0.5f, 0.1f);
    transportControl.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    transportControl.isVisible = true;
    transportControl.isInteractive = true;
    addControl(transportControl);
}

void VRControlPanel::update() {
    // Aktualisiere Kontrollelemente
    for (auto& control : controls) {
        if (control.isVisible) {
            updateRendering();
        }
    }
}

void VRControlPanel::render() {
    // Rendere Kontrollelemente
    for (const auto& control : controls) {
        if (control.isVisible) {
            renderControl(control);
        }
    }
}

void VRControlPanel::handleInteraction(const VRInterface::MotionData& motionData) {
    // Überprüfe Interaktionen mit Kontrollelementen
    for (const auto& control : controls) {
        if (control.isInteractive) {
            handleControlInteraction(control, motionData);
        }
    }
}

void VRControlPanel::handleControlInteraction(const Control& control, const VRInterface::MotionData& motionData) {
    // Berechne Distanz zwischen Controller und Kontrollelement
    float distance = glm::length(motionData.position - control.position);
    
    // Überprüfe, ob Controller nahe genug am Kontrollelement ist
    if (distance < 0.5f) {
        // Überprüfe, ob Controller den Trigger drückt
        if (motionData.triggerPressed) {
            if (!interactionState.isGrabbing) {
                // Starte Interaktion
                interactionState.isGrabbing = true;
                interactionState.grabbedControl = control.id;
                interactionState.grabOffset = motionData.position - control.position;
            }
        } else {
            if (interactionState.isGrabbing && interactionState.grabbedControl == control.id) {
                // Beende Interaktion
                interactionState.isGrabbing = false;
                interactionState.grabbedControl = "";
                
                // Rufe Callback auf
                if (control.callback) {
                    control.callback(control);
                }
            }
        }
    }
}

void VRControlPanel::renderControl(const Control& control) {
    switch (control.type) {
        case ControlType::Button:
            renderButton(control);
            break;
        case ControlType::Slider:
            renderSlider(control);
            break;
        case ControlType::Knob:
            renderKnob(control);
            break;
        case ControlType::Toggle:
            renderToggle(control);
            break;
        case ControlType::Menu:
            renderMenu(control);
            break;
        case ControlType::Display:
            renderDisplay(control);
            break;
        case ControlType::Meter:
            renderMeter(control);
            break;
        case ControlType::Waveform:
            renderWaveform(control);
            break;
        case ControlType::Spectrum:
            renderSpectrum(control);
            break;
    }
}

void VRControlPanel::renderButton(const Control& control) {
    // Erstelle Button-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Button
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Label
    // TODO: Implementiere Text-Rendering
}

void VRControlPanel::renderSlider(const Control& control) {
    // Erstelle Slider-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.1f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.1f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.1f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.1f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Slider
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Slider-Knopf
    // TODO: Implementiere Knopf-Rendering
}

void VRControlPanel::renderKnob(const Control& control) {
    // Erstelle Knob-Mesh (Kreis)
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    const int segments = 32;
    const float radius = 0.5f;
    
    // Mittelpunkt
    vertices.push_back({{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}});
    
    // Kreis-Punkte
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * glm::pi<float>();
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back({{x, y, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f + 0.5f * x, 0.5f + 0.5f * y}});
    }
    
    // Indizes für Dreiecke
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Knob
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Markierungen
    // TODO: Implementiere Markierungs-Rendering
}

void VRControlPanel::renderToggle(const Control& control) {
    // Erstelle Toggle-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Toggle
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Toggle-Knopf
    // TODO: Implementiere Knopf-Rendering
}

void VRControlPanel::renderMenu(const Control& control) {
    // Erstelle Menu-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 0.9f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Menu
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Menu-Einträge
    // TODO: Implementiere Menu-Einträge-Rendering
}

void VRControlPanel::renderDisplay(const Control& control) {
    // Erstelle Display-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Display
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Display-Inhalt
    // TODO: Implementiere Display-Inhalt-Rendering
}

void VRControlPanel::renderMeter(const Control& control) {
    // Erstelle Meter-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Meter
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Meter-Anzeige
    // TODO: Implementiere Meter-Anzeige-Rendering
}

void VRControlPanel::renderWaveform(const Control& control) {
    // Erstelle Waveform-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Waveform
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Waveform-Daten
    // TODO: Implementiere Waveform-Daten-Rendering
}

void VRControlPanel::renderSpectrum(const Control& control) {
    // Erstelle Spectrum-Mesh
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Erstelle Material
    Material material;
    material.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
    material.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.shininess = 32.0f;
    material.transparency = 1.0f;
    
    // Erstelle Transformations-Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, control.position);
    model = glm::rotate(model, glm::angle(control.rotation), glm::axis(control.rotation));
    model = glm::scale(model, glm::vec3(control.size.x, control.size.y, 1.0f));
    
    // Rendere Spectrum
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    vrInterface->renderMesh(mesh, model, material);
    
    // Rendere Spectrum-Daten
    // TODO: Implementiere Spectrum-Daten-Rendering
}

void VRControlPanel::addControl(const Control& control) {
    controls.push_back(control);
    controlRegistry[control.id] = control;
}

void VRControlPanel::removeControl(const std::string& id) {
    auto it = std::find_if(controls.begin(), controls.end(),
        [&id](const Control& control) { return control.id == id; });
    
    if (it != controls.end()) {
        controls.erase(it);
        controlRegistry.erase(id);
    }
}

void VRControlPanel::updateControl(const Control& control) {
    auto it = controlRegistry.find(control.id);
    if (it != controlRegistry.end()) {
        it->second = control;
        
        auto vecIt = std::find_if(controls.begin(), controls.end(),
            [&control](const Control& c) { return c.id == control.id; });
        
        if (vecIt != controls.end()) {
            *vecIt = control;
        }
    }
}

void VRControlPanel::setControlVisibility(const std::string& id, bool visible) {
    auto it = controlRegistry.find(id);
    if (it != controlRegistry.end()) {
        it->second.isVisible = visible;
        
        auto vecIt = std::find_if(controls.begin(), controls.end(),
            [&id](const Control& control) { return control.id == id; });
        
        if (vecIt != controls.end()) {
            vecIt->isVisible = visible;
        }
    }
}

void VRControlPanel::setControlInteraction(const std::string& id, bool interactive) {
    auto it = controlRegistry.find(id);
    if (it != controlRegistry.end()) {
        it->second.isInteractive = interactive;
        
        auto vecIt = std::find_if(controls.begin(), controls.end(),
            [&id](const Control& control) { return control.id == id; });
        
        if (vecIt != controls.end()) {
            vecIt->isInteractive = interactive;
        }
    }
}

void VRControlPanel::setLayout(const std::string& layoutName) {
    auto it = layouts.find(layoutName);
    if (it != layouts.end()) {
        controls = it->second;
        for (const auto& control : controls) {
            controlRegistry[control.id] = control;
        }
    }
}

void VRControlPanel::saveLayout(const std::string& layoutName) {
    layouts[layoutName] = controls;
}

void VRControlPanel::loadLayout(const std::string& layoutName) {
    setLayout(layoutName);
}

void VRControlPanel::connectToAudioEngine(AudioEngine* engine) {
    audioEngine = engine;
}

void VRControlPanel::updateAudioParameters(const std::string& controlId, float value) {
    if (audioEngine) {
        // TODO: Implementiere Audio-Parameter-Update
    }
}

} // namespace VR_DAW 