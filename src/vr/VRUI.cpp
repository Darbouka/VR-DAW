#define GLM_ENABLE_EXPERIMENTAL
#include "VRUI.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <chrono>
#include <stdexcept>
#include <sstream>

namespace VR_DAW {

struct VRUI::Impl {
    std::vector<UIElement> elements;
    std::vector<TrackView> trackViews;
    std::vector<PluginView> pluginViews;
    std::string currentLayout;
    glm::vec3 controllerPosition;
    glm::vec3 controllerDirection;
    UIElement* focusedElement;
    bool isInitialized;
    std::map<std::string, std::function<float(float)>> easingFunctions;
    std::map<UIElement*, std::vector<Animation>> animations;
    std::vector<TextElement> textElements;
    bool debugEnabled;
    float renderScale;
    int renderQuality;
    std::vector<SynthesizerView> synthesizerViews;
    float uiScale;
    glm::vec3 defaultPosition;
    std::function<void(const AudioEvent&)> audioCallback;
    
#ifdef USE_JACK
    jack_client_t* jackClient;
    jack_port_t* inputPort;
    jack_port_t* outputPort;
#endif
    
    // Verbesserte Fehlerbehandlung
    std::string lastError;
    bool hasError;
    std::vector<std::string> errorLog;
    
    // Performance-Metriken
    struct PerformanceMetrics {
        float frameTime;
        float renderTime;
        float updateTime;
        size_t drawCalls;
        size_t activeElements;
    } metrics;
    
    // Debug-Informationen
    struct DebugInfo {
        bool showFPS;
        bool showMemory;
        bool showPerformance;
        bool showElementBounds;
    } debugInfo;
};

VRUI::VRUI()
    : pImpl(std::make_unique<Impl>())
    , initialized(false)
    , debugEnabled(false)
    , renderScale(1.0f)
    , renderQuality(1)
{
    pImpl->focusedElement = nullptr;
    pImpl->isInitialized = false;
    pImpl->currentLayout = "default";
    pImpl->hasError = false;
    pImpl->debugInfo = {true, false, false, false};
    
    // Easing-Funktionen initialisieren
    initializeEasingFunctions();
}

VRUI::~VRUI() {
    shutdown();
}

bool VRUI::initialize() {
    if (initialized) {
        logError("VRUI ist bereits initialisiert");
        return false;
    }

    try {
        initializeFontSystem();
        initializeAudioSystem();
        initializeRenderingSystem();
        
        initialized = true;
        pImpl->hasError = false;
        return true;
    } catch (const std::exception& e) {
        logError("Initialisierungsfehler: " + std::string(e.what()));
        return false;
    }
}

void VRUI::shutdown() {
    if (!initialized) return;

    pImpl->elements.clear();
    pImpl->trackViews.clear();
    pImpl->pluginViews.clear();
    pImpl->isInitialized = false;
    pImpl->debugEnabled = false;
    pImpl->renderScale = 1.0f;
    pImpl->renderQuality = 1;
    pImpl->synthesizerViews.clear();
}

void VRUI::update() {
    if (!initialized) {
        logError("VRUI ist nicht initialisiert");
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        updateAnimations();
        updateElementTransforms();
        processInteractions();
        
        if (pImpl->focusedElement) {
            UIElement* element = findElementAtPosition(pImpl->controllerPosition);
            if (element != pImpl->focusedElement) {
                pImpl->focusedElement = element;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        pImpl->metrics.updateTime = std::chrono::duration<float>(endTime - startTime).count();
    } catch (const std::exception& e) {
        logError("Update-Fehler: " + std::string(e.what()));
    }
}

void VRUI::render() {
    if (!initialized) {
        logError("VRUI ist nicht initialisiert");
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    pImpl->metrics.drawCalls = 0;
    
    try {
        // Track-Views rendern
        for (const auto& view : pImpl->trackViews) {
            renderTrackView(view);
            pImpl->metrics.drawCalls++;
        }

        // Plugin-Views rendern
        for (const auto& view : pImpl->pluginViews) {
            renderPluginView(view);
            pImpl->metrics.drawCalls++;
        }

        // Synthesizer-Views rendern
        for (const auto& view : pImpl->synthesizerViews) {
            renderSynthesizerView(view);
            pImpl->metrics.drawCalls++;
        }

        // UI-Elemente rendern
        pImpl->metrics.activeElements = 0;
        for (const auto& element : pImpl->elements) {
            if (element.visible) {
                renderElement(element);
                pImpl->metrics.activeElements++;
                pImpl->metrics.drawCalls++;
            }
        }

        if (pImpl->debugEnabled) {
            renderDebugInfo();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        pImpl->metrics.renderTime = std::chrono::duration<float>(endTime - startTime).count();
    } catch (const std::exception& e) {
        logError("Render-Fehler: " + std::string(e.what()));
    }
}

VRUI::UIElement* VRUI::createButton(const std::string& id, const glm::vec3& position, const glm::vec3& scale) {
    UIElement element;
    element.type = UIElement::Type::Button;
    element.id = id;
    element.position = position;
    element.scale = scale;
    element.rotation = glm::vec3(0.0f);
    element.visible = true;
    element.interactive = true;

    pImpl->elements.push_back(element);
    return &pImpl->elements.back();
}

VRUI::UIElement* VRUI::createSlider(const std::string& id, const glm::vec3& position, const glm::vec3& scale) {
    UIElement element;
    element.type = UIElement::Type::Slider;
    element.id = id;
    element.position = position;
    element.scale = scale;
    element.rotation = glm::vec3(0.0f);
    element.visible = true;
    element.interactive = true;

    pImpl->elements.push_back(element);
    return &pImpl->elements.back();
}

VRUI::UIElement* VRUI::createKnob(const std::string& id, const glm::vec3& position, float radius) {
    UIElement element;
    element.type = UIElement::Type::Knob;
    element.id = id;
    element.position = position;
    element.scale = glm::vec3(radius);
    element.rotation = glm::vec3(0.0f);
    element.visible = true;
    element.interactive = true;

    pImpl->elements.push_back(element);
    return &pImpl->elements.back();
}

VRUI::UIElement* VRUI::createWaveform(const std::string& id, const glm::vec3& position, const glm::vec3& size) {
    UIElement element;
    element.type = UIElement::Type::Waveform;
    element.id = id;
    element.position = position;
    element.scale = size;
    element.rotation = glm::vec3(0.0f);
    element.visible = true;
    element.interactive = false;

    pImpl->elements.push_back(element);
    return &pImpl->elements.back();
}

VRUI::TrackView* VRUI::createTrackView(int trackId, const std::string& name) {
    TrackView view;
    view.trackId = trackId;
    view.name = name;
    view.position = glm::vec3(0.0f);
    view.size = glm::vec3(1.0f, 0.2f, 0.1f);

    // Track-spezifische UI-Elemente erstellen
    auto* muteButton = createButton("mute_" + std::to_string(trackId),
                                  view.position + glm::vec3(-0.4f, 0.0f, 0.0f),
                                  glm::vec3(0.1f));
    
    auto* soloButton = createButton("solo_" + std::to_string(trackId),
                                  view.position + glm::vec3(-0.3f, 0.0f, 0.0f),
                                  glm::vec3(0.1f));
    
    auto* volumeSlider = createSlider("volume_" + std::to_string(trackId),
                                    view.position + glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(0.2f, 0.05f, 0.05f));
    
    auto* panKnob = createKnob("pan_" + std::to_string(trackId),
                              view.position + glm::vec3(0.3f, 0.0f, 0.0f),
                              0.05f);

    view.controls.push_back(*muteButton);
    view.controls.push_back(*soloButton);
    view.controls.push_back(*volumeSlider);
    view.controls.push_back(*panKnob);

    pImpl->trackViews.push_back(view);
    return &pImpl->trackViews.back();
}

void VRUI::updateTrackView(TrackView* view, const std::vector<float>& waveform) {
    if (!view) return;
    auto waveformElement = std::find_if(pImpl->elements.begin(), pImpl->elements.end(),
        [view](const UIElement& e) {
            return e.type == UIElement::Type::Waveform && e.id == "waveform_" + std::to_string(view->trackId);
        });
    if (waveformElement != pImpl->elements.end()) {
        setWaveformData(waveformElement->id, waveform);
    }
}

void VRUI::deleteTrackView(TrackView* view) {
    if (!view) return;

    // UI-Elemente des Tracks entfernen
    pImpl->elements.erase(
        std::remove_if(pImpl->elements.begin(), pImpl->elements.end(),
            [view](const UIElement& e) {
                return e.id.find(std::to_string(view->trackId)) != std::string::npos;
            }),
        pImpl->elements.end()
    );

    // Track-View entfernen
    pImpl->trackViews.erase(
        std::remove_if(pImpl->trackViews.begin(), pImpl->trackViews.end(),
            [view](const TrackView& v) { return v.trackId == view->trackId; }),
        pImpl->trackViews.end()
    );
}

void VRUI::handleControllerInput(const glm::vec3& position, const glm::vec3& direction) {
    pImpl->controllerPosition = position;
    pImpl->controllerDirection = direction;

    UIElement* element = findElementAtPosition(position);
    if (element && element->interactive) {
        if (element->onClick) {
            element->onClick();
        }
    }
}

void VRUI::handleGesture(const std::string& gestureType, const glm::vec3& position) {
    if (gestureType == "grab") {
        UIElement* element = findElementAtPosition(position);
        if (element) {
            animateElement(*element, position, 0.3f);
        }
    }
}

void VRUI::handleVoiceCommand(const std::string& command) {
    if (command == "arrange tracks") {
        arrangeTracks();
    } else if (command == "arrange plugins") {
        arrangePlugins();
    }
}

void VRUI::setLayout(const std::string& layout) {
    if (!initialized) return;
    pImpl->currentLayout = layout;
    updateLayout();
}

void VRUI::updateLayout() {
    if (!initialized) return;

    float spacing = 0.3f;
    float startY = 0.0f;

    for (size_t i = 0; i < pImpl->trackViews.size(); ++i) {
        auto& view = pImpl->trackViews[i];
        view.position = glm::vec3(0.0f, startY - i * spacing, 0.0f);
        
        // UI-Elemente neu positionieren
        for (auto& control : view.controls) {
            updateElementTransform(control);
        }
    }

    float spacingPlugins = 0.4f;
    float startX = 1.5f;

    for (size_t i = 0; i < pImpl->pluginViews.size(); ++i) {
        auto& view = pImpl->pluginViews[i];
        view.position = glm::vec3(startX + i * spacingPlugins, 0.0f, 0.0f);
        
        // Parameter neu positionieren
        for (auto& param : view.parameters) {
            updateElementTransform(param);
        }
    }
}

void VRUI::arrangeTracks() {
    float spacing = 0.3f;
    float startY = 0.0f;

    for (size_t i = 0; i < pImpl->trackViews.size(); ++i) {
        auto& view = pImpl->trackViews[i];
        view.position = glm::vec3(0.0f, startY - i * spacing, 0.0f);
        
        // UI-Elemente neu positionieren
        for (auto& control : view.controls) {
            updateElementTransform(control);
        }
    }
}

void VRUI::arrangePlugins() {
    float spacing = 0.4f;
    float startX = 1.5f;

    for (size_t i = 0; i < pImpl->pluginViews.size(); ++i) {
        auto& view = pImpl->pluginViews[i];
        view.position = glm::vec3(startX + i * spacing, 0.0f, 0.0f);
        
        // Parameter neu positionieren
        for (auto& param : view.parameters) {
            updateElementTransform(param);
        }
    }
}

void VRUI::focusElement(const std::string& elementId) {
    auto it = std::find_if(pImpl->elements.begin(), pImpl->elements.end(),
        [&elementId](const UIElement& e) { return e.id == elementId; });

    if (it != pImpl->elements.end()) {
        pImpl->focusedElement = &(*it);
        animateElement(*it, glm::vec3(0.0f, 0.0f, -0.5f), 0.5f);
    }
}

void VRUI::renderElement(const UIElement& element) {
    glm::mat4 modelMatrix = calculateModelMatrix(element);
    
    switch (element.type) {
        case UIElement::Type::Button:
            renderButton(element, modelMatrix);
            break;
        case UIElement::Type::Slider:
            renderSlider(element, modelMatrix);
            break;
        case UIElement::Type::Knob:
            renderKnob(element, modelMatrix);
            break;
        case UIElement::Type::Waveform:
            renderWaveform(element, modelMatrix);
            break;
        case UIElement::Type::Text:
            renderText(TextElement{element.text, element.position, element.scale});
            break;
        default:
            break;
    }
}

void VRUI::renderTrackView(const TrackView& view) {
    // Track-Hintergrund rendern
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), view.position);
    modelMatrix = glm::scale(modelMatrix, view.size);
    
    // Track-Name rendern
    // TODO: Text-Rendering implementieren
    
    // Track-Controls rendern
    for (const auto& control : view.controls) {
        renderElement(control);
    }
}

void VRUI::renderPluginView(const PluginView& view) {
    // Plugin-Hintergrund rendern
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), view.position);
    modelMatrix = glm::scale(modelMatrix, view.size);
    
    // Plugin-Name rendern
    // TODO: Text-Rendering implementieren
    
    // Plugin-Parameter rendern
    for (const auto& param : view.parameters) {
        renderElement(param);
    }
}

VRUI::UIElement* VRUI::findElementAtPosition(const glm::vec3& position) {
    for (auto& element : pImpl->elements) {
        if (element.visible && element.interactive && isPointInElement(position, element)) {
            return &element;
        }
    }
    return nullptr;
}

bool VRUI::isPointInElement(const glm::vec3& point, const UIElement& element) {
    glm::vec3 localPoint = point - element.position;
    return std::abs(localPoint.x) <= element.scale.x / 2.0f &&
           std::abs(localPoint.y) <= element.scale.y / 2.0f &&
           std::abs(localPoint.z) <= element.scale.z / 2.0f;
}

void VRUI::updateAnimations() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
    lastUpdateTime = currentTime;

    for (auto& [element, animations] : pImpl->animations) {
        for (auto& animation : animations) {
            animation.elapsed += deltaTime;
            float t = std::min(animation.elapsed / animation.duration, 1.0f);
            
            if (animation.easingFunction) {
                t = animation.easingFunction(t);
            }
            
            updateElementAnimation(*element, animation, t);
        }

        // Abgeschlossene Animationen entfernen
        animations.erase(
            std::remove_if(animations.begin(), animations.end(),
                [](const Animation& a) { return a.elapsed >= a.duration; }),
            animations.end()
        );
    }
}

void VRUI::animateElement(UIElement& element, const glm::vec3& targetPosition, float duration) {
    Animation animation;
    animation.type = Animation::Type::Position;
    animation.startValue = element.position;
    animation.endValue = targetPosition;
    animation.duration = duration;
    animation.elapsed = 0.0f;
    animation.easingFunction = pImpl->easingFunctions["easeInOutQuad"];
    
    pImpl->animations[&element].push_back(animation);
}

glm::mat4 VRUI::calculateModelMatrix(const UIElement& element) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, element.position);
    model = glm::rotate(model, element.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, element.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, element.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, element.scale);
    return model;
}

void VRUI::updateElementTransform(UIElement& element) {
    // TODO: Transform-Update-Logik implementieren
}

void VRUI::initializeFontSystem() {
    auto& textRenderer = TextRenderer::getInstance();
    textRenderer.initialize();
    
    // Standard-Schriftarten laden
    textRenderer.loadFont("default", "fonts/Roboto-Regular.ttf");
    textRenderer.loadFont("bold", "fonts/Roboto-Bold.ttf");
    textRenderer.loadFont("mono", "fonts/RobotoMono-Regular.ttf");
}

void VRUI::loadFont(const std::string& fontName) {
    // TODO: Font laden und in Textur speichern
}

VRUI::TextElement* VRUI::createText(const std::string& text, const glm::vec3& position, float fontSize) {
    TextElement element;
    element.text = text;
    element.position = position;
    element.scale = glm::vec3(1.0f);
    element.color = glm::vec4(1.0f);
    element.fontSize = fontSize;
    element.fontName = "default";
    pImpl->textElements.push_back(element);
    return &pImpl->textElements.back();
}

void VRUI::updateText(TextElement* text, const std::string& newText) {
    if (text) {
        text->text = newText;
    }
}

void VRUI::renderText(const TextElement& text) {
    if (!initialized) return;
    
    auto& textRenderer = TextRenderer::getInstance();
    
    // Text-Metriken berechnen
    glm::vec2 textSize = textRenderer.getTextSize(text.text, text.fontSize);
    
    // Text-Position berechnen
    glm::vec3 position = text.position;
    if (text.alignment == TextElement::Alignment::Center) {
        position.x -= textSize.x / 2.0f;
    } else if (text.alignment == TextElement::Alignment::Right) {
        position.x -= textSize.x;
    }
    
    // 3D-Text rendern
    textRenderer.renderText3D(text.text, position, text.fontSize, text.color);
    
    // Debug-Rendering
    if (pImpl->debugEnabled) {
        renderTextBounds(text, textSize);
    }
}

void VRUI::renderTextBounds(const TextElement& text, const glm::vec2& textSize) {
    // Text-Grenzen visualisieren
    glm::vec3 min = text.position;
    glm::vec3 max = text.position + glm::vec3(textSize, 0.0f);
    
    // Debug-Linien für Text-Grenzen
    std::vector<glm::vec3> lines = {
        min, glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z), max,
        max, glm::vec3(min.x, max.y, min.z),
        glm::vec3(min.x, max.y, min.z), min
    };
    
    // Debug-Linien rendern
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (const auto& line : lines) {
        glVertex3f(line.x, line.y, line.z);
    }
    glEnd();
}

void VRUI::addAnimation(UIElement& element, const Animation& animation) {
    pImpl->animations[&element].push_back(animation);
}

void VRUI::removeAnimation(UIElement& element, Animation::Type type) {
    auto& animations = pImpl->animations[&element];
    animations.erase(
        std::remove_if(animations.begin(), animations.end(),
            [type](const Animation& a) { return a.type == type; }),
        animations.end()
    );
}

void VRUI::updateElementAnimation(UIElement& element, const Animation& animation, float t) {
    float easedT = applyEasing(t, animation.easingFunction);
    glm::vec3 currentValue = interpolate(animation.startValue, animation.endValue, easedT);

    switch (animation.type) {
        case Animation::Type::Position:
            element.position = currentValue;
            break;
        case Animation::Type::Scale:
            element.scale = currentValue;
            break;
        case Animation::Type::Rotation:
            element.rotation = currentValue;
            break;
        // Weitere Animationstypen hier...
    }
}

glm::vec3 VRUI::interpolate(const glm::vec3& start, const glm::vec3& end, float t) {
    return start + (end - start) * t;
}

glm::vec2 VRUI::calculateTextBounds(const TextElement& text) {
    // TODO: Text-Breite und -Höhe berechnen
    return glm::vec2(text.text.length() * text.fontSize * 0.6f, text.fontSize);
}

void VRUI::setWaveformData(const std::string& elementId, const std::vector<float>& data) {
    // Dummy: Hier könnte die Waveform-Daten für ein UI-Element gespeichert werden
}

void VRUI::updateElementTransforms() {
    if (!initialized) return;

    // Hier würde die Aktualisierung der Element-Transformationen erfolgen
}

void VRUI::processInteractions() {
    if (!initialized) return;

    // Hier würde die Verarbeitung der Interaktionen erfolgen
}

void VRUI::renderDebugInfo() {
    if (!pImpl->debugInfo.showFPS && !pImpl->debugInfo.showMemory && 
        !pImpl->debugInfo.showPerformance && !pImpl->debugInfo.showElementBounds) {
        return;
    }

    std::stringstream ss;
    
    if (pImpl->debugInfo.showFPS) {
        float fps = 1.0f / pImpl->metrics.frameTime;
        ss << "FPS: " << std::fixed << std::setprecision(1) << fps << "\n";
    }
    
    if (pImpl->debugInfo.showPerformance) {
        ss << "Frame Time: " << pImpl->metrics.frameTime * 1000.0f << "ms\n";
        ss << "Render Time: " << pImpl->metrics.renderTime * 1000.0f << "ms\n";
        ss << "Update Time: " << pImpl->metrics.updateTime * 1000.0f << "ms\n";
        ss << "Draw Calls: " << pImpl->metrics.drawCalls << "\n";
        ss << "Active Elements: " << pImpl->metrics.activeElements << "\n";
    }
    
    if (pImpl->debugInfo.showMemory) {
        // TODO: Speichernutzung anzeigen
    }
    
    if (pImpl->debugInfo.showElementBounds) {
        for (const auto& element : pImpl->elements) {
            if (element.visible) {
                renderElementBounds(element);
            }
        }
    }
    
    // Debug-Text rendern
    auto* debugText = createText(ss.str(), glm::vec3(-0.9f, 0.9f, 0.0f), 0.05f);
    renderText(*debugText);
}

void VRUI::renderElementBounds(const UIElement& element) {
    glm::vec3 min = element.position - element.scale * 0.5f;
    glm::vec3 max = element.position + element.scale * 0.5f;
    
    std::vector<glm::vec3> lines = {
        min, glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z), max,
        max, glm::vec3(min.x, max.y, min.z),
        glm::vec3(min.x, max.y, min.z), min,
        min, glm::vec3(min.x, min.y, max.z),
        glm::vec3(min.x, min.y, max.z), glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z), glm::vec3(max.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z), glm::vec3(min.x, max.y, max.z),
        glm::vec3(min.x, max.y, max.z), glm::vec3(min.x, min.y, max.z)
    };
    
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (const auto& line : lines) {
        glVertex3f(line.x, line.y, line.z);
    }
    glEnd();
}

void VRUI::setRenderScale(float scale) {
    renderScale = std::max(0.1f, scale);
}

void VRUI::setRenderQuality(int quality) {
    renderQuality = std::max(0, std::min(3, quality));
}

void VRUI::enableDebugRendering(bool enable) {
    pImpl->debugEnabled = enable;
}

void VRUI::setUIScale(float scale) {
    if (!initialized) return;
    pImpl->uiScale = std::max(0.1f, scale);
    updateLayout();
}

void VRUI::setDefaultPosition(const glm::vec3& position) {
    if (!initialized) return;
    pImpl->defaultPosition = position;
    updateLayout();
}

void VRUI::handleMIDIMessage(const MIDIMessage& msg) {
    if (!initialized) return;

    // MIDI-Nachrichten an entsprechende UI-Elemente weiterleiten
    switch (msg.type) {
        case MIDIMessage::Type::NoteOn: {
            // Note-On UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == msg.channel) {
                    // Oszillator-Controls aktualisieren
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("type") != std::string::npos) {
                            float normalizedNote = msg.value1 / 127.0f;
                            osc.value = ParameterValidation::denormalizeValue("osc_mix", normalizedNote);
                        }
                    }
                }
            }
            break;
        }
        case MIDIMessage::Type::NoteOff: {
            // Note-Off UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == msg.channel) {
                    // Oszillator-Controls zurücksetzen
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("type") != std::string::npos) {
                            osc.value = ParameterValidation::getRange("osc_mix").defaultValue;
                        }
                    }
                }
            }
            break;
        }
        case MIDIMessage::Type::ControlChange: {
            // Control-Change UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == msg.channel) {
                    // Parameter basierend auf Controller-Nummer aktualisieren
                    switch (msg.value1) {
                        case 1: // Modulation
                            for (auto& lfo : view.lfos) {
                                if (lfo.id.find("depth") != std::string::npos) {
                                    float normalizedDepth = msg.value2 / 127.0f;
                                    lfo.value = ParameterValidation::denormalizeValue("lfo_depth", normalizedDepth);
                                }
                            }
                            break;
                        case 7: // Volume
                            for (auto& control : view.controls) {
                                if (control.id.find("volume") != std::string::npos) {
                                    float normalizedVolume = msg.value2 / 127.0f;
                                    control.value = ParameterValidation::denormalizeValue("volume", normalizedVolume);
                                }
                            }
                            break;
                        case 10: // Pan
                            for (auto& control : view.controls) {
                                if (control.id.find("pan") != std::string::npos) {
                                    float normalizedPan = (msg.value2 - 64) / 64.0f;
                                    control.value = ParameterValidation::denormalizeValue("pan", normalizedPan);
                                }
                            }
                            break;
                        case 74: // Filter Cutoff
                            for (auto& filter : view.filters) {
                                if (filter.id.find("cutoff") != std::string::npos) {
                                    float normalizedCutoff = msg.value2 / 127.0f;
                                    filter.value = ParameterValidation::denormalizeValue("filter_cutoff", normalizedCutoff);
                                }
                            }
                            break;
                        case 71: // Filter Resonance
                            for (auto& filter : view.filters) {
                                if (filter.id.find("resonance") != std::string::npos) {
                                    float normalizedResonance = msg.value2 / 127.0f;
                                    filter.value = ParameterValidation::denormalizeValue("filter_resonance", normalizedResonance);
                                }
                            }
                            break;
                    }
                }
            }
            break;
        }
        case MIDIMessage::Type::PitchBend: {
            // Pitch-Bend UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == msg.channel) {
                    // Oszillator-Detune aktualisieren
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("detune") != std::string::npos) {
                            float normalizedDetune = (msg.value1 + msg.value2 * 128) / 16384.0f;
                            osc.value = ParameterValidation::denormalizeValue("osc_detune", normalizedDetune);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    // Audio-Event erstellen und Callback aufrufen
    if (pImpl->audioCallback) {
        AudioEvent event;
        event.type = static_cast<AudioEvent::Type>(msg.type);
        event.channel = msg.channel;
        event.value1 = msg.value1;
        event.value2 = msg.value2;
        pImpl->audioCallback(event);
    }
}

void VRUI::setAudioCallback(std::function<void(const AudioEvent&)> callback) {
    if (!initialized) return;
    pImpl->audioCallback = callback;
}

VRUI::SynthesizerView* VRUI::createSynthesizerView(int trackId, const std::string& name) {
    SynthesizerView view;
    view.trackId = trackId;
    view.name = name;
    view.position = glm::vec3(0.0f);
    view.size = glm::vec3(1.0f, 0.8f, 0.1f);

    // Synthesizer in der Audio-Engine erstellen
    SynthesizerConfig config;
    auto& engine = AudioEngine::getInstance();
    engine.createSynthesizer(trackId, config);

    // Synthesizer-Name
    auto* nameText = createText(name, view.position + glm::vec3(0.0f, 0.35f, 0.0f), 0.1f);
    view.controls.push_back(*nameText);

    // Allgemeine Steuerelemente
    auto* volumeSlider = createSlider("volume_" + std::to_string(trackId),
                                    view.position + glm::vec3(-0.4f, 0.25f, 0.0f),
                                    glm::vec3(0.2f, 0.05f, 0.05f));
    volumeSlider->value = ParameterValidation::getRange("volume").defaultValue;
    volumeSlider->onClick = [trackId, volumeSlider]() {
        auto& engine = AudioEngine::getInstance();
        SynthesizerConfig config;
        config.defaultVolume = ParameterValidation::denormalizeValue("volume", volumeSlider->value);
        engine.updateSynthesizer(trackId, config);
    };
    view.controls.push_back(*volumeSlider);

    auto* panKnob = createKnob("pan_" + std::to_string(trackId),
                              view.position + glm::vec3(-0.2f, 0.25f, 0.0f),
                              0.05f);
    panKnob->value = ParameterValidation::getRange("pan").defaultValue;
    panKnob->onClick = [trackId, panKnob]() {
        auto& engine = AudioEngine::getInstance();
        SynthesizerConfig config;
        config.defaultPan = ParameterValidation::denormalizeValue("pan", panKnob->value);
        engine.updateSynthesizer(trackId, config);
    };
    view.controls.push_back(*panKnob);

    // Oszillatoren
    for (int i = 0; i < 3; ++i) {
        SynthesizerConfig::OscillatorConfig defaultOsc;
        createOscillatorControls(view, defaultOsc, i);
    }

    // Filter
    SynthesizerConfig::FilterConfig defaultFilter;
    createFilterControls(view, defaultFilter);

    // Hüllkurven
    SynthesizerConfig::EnvelopeConfig defaultEnv;
    createEnvelopeControls(view, defaultEnv, "Amplitude");
    createEnvelopeControls(view, defaultEnv, "Filter");

    // LFOs
    SynthesizerConfig::LFOConfig defaultLFO;
    createLFOControls(view, defaultLFO, 0);
    createLFOControls(view, defaultLFO, 1);

    // Effekte
    SynthesizerConfig::EffectConfig defaultEffect;
    createEffectControls(view, defaultEffect, 0);
    createEffectControls(view, defaultEffect, 1);

    pImpl->synthesizerViews.push_back(view);
    return &pImpl->synthesizerViews.back();
}

void VRUI::updateSynthesizerView(SynthesizerView* view, const SynthesizerConfig& config) {
    if (!view) return;

    // Oszillatoren aktualisieren
    for (size_t i = 0; i < config.oscillators.size() && i < view->oscillators.size(); ++i) {
        updateOscillatorControls(view->oscillators[i], config.oscillators[i]);
    }

    // Filter aktualisieren
    updateFilterControls(view->filters[0], config.filter);

    // Hüllkurven aktualisieren
    updateEnvelopeControls(view->envelopes[0], config.amplitudeEnvelope);
    updateEnvelopeControls(view->envelopes[1], config.filterEnvelope);

    // LFOs aktualisieren
    for (size_t i = 0; i < config.lfos.size() && i < view->lfos.size(); ++i) {
        updateLFOControls(view->lfos[i], config.lfos[i]);
    }

    // Effekte aktualisieren
    for (size_t i = 0; i < config.effects.size() && i < view->effects.size(); ++i) {
        updateEffectControls(view->effects[i], config.effects[i]);
    }
}

void VRUI::deleteSynthesizerView(SynthesizerView* view) {
    if (!view) return;

    // Synthesizer aus der Audio-Engine entfernen
    AudioEngine::getInstance().deleteSynthesizer(view->trackId);

    // UI-Elemente des Synthesizers entfernen
    pImpl->elements.erase(
        std::remove_if(pImpl->elements.begin(), pImpl->elements.end(),
            [view](const UIElement& e) {
                return e.id.find(std::to_string(view->trackId)) != std::string::npos;
            }),
        pImpl->elements.end()
    );

    // Synthesizer-View entfernen
    pImpl->synthesizerViews.erase(
        std::remove_if(pImpl->synthesizerViews.begin(), pImpl->synthesizerViews.end(),
            [view](const SynthesizerView& v) { return v.trackId == view->trackId; }),
        pImpl->synthesizerViews.end()
    );
}

void VRUI::arrangeSynthesizers() {
    float spacing = 1.2f;
    float startX = -1.5f;

    for (size_t i = 0; i < pImpl->synthesizerViews.size(); ++i) {
        auto& view = pImpl->synthesizerViews[i];
        view.position = glm::vec3(startX + i * spacing, 0.0f, 0.0f);
        
        // UI-Elemente neu positionieren
        for (auto& control : view.controls) {
            updateElementTransform(control);
        }
        for (auto& osc : view.oscillators) {
            updateElementTransform(osc);
        }
        for (auto& filter : view.filters) {
            updateElementTransform(filter);
        }
        for (auto& env : view.envelopes) {
            updateElementTransform(env);
        }
        for (auto& lfo : view.lfos) {
            updateElementTransform(lfo);
        }
        for (auto& effect : view.effects) {
            updateElementTransform(effect);
        }
    }
}

void VRUI::createOscillatorControls(SynthesizerView& view, const SynthesizerConfig::OscillatorConfig& config, int index) {
    float x = -0.4f + index * 0.3f;
    float y = 0.15f;

    // Oszillator-Typ
    auto* typeButton = createButton("osc_type_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                  view.position + glm::vec3(x, y, 0.0f),
                                  glm::vec3(0.1f));
    view.oscillators.push_back(*typeButton);

    // Mix
    auto* mixSlider = createSlider("osc_mix_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                 view.position + glm::vec3(x, y - 0.1f, 0.0f),
                                 glm::vec3(0.1f, 0.02f, 0.02f));
    view.oscillators.push_back(*mixSlider);

    // Detune
    auto* detuneKnob = createKnob("osc_detune_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                view.position + glm::vec3(x, y - 0.2f, 0.0f),
                                0.03f);
    view.oscillators.push_back(*detuneKnob);
}

void VRUI::createFilterControls(SynthesizerView& view, const SynthesizerConfig::FilterConfig& config) {
    float x = 0.2f;
    float y = 0.15f;

    // Filter-Typ
    auto* typeButton = createButton("filter_type_" + std::to_string(view.trackId),
                                  view.position + glm::vec3(x, y, 0.0f),
                                  glm::vec3(0.1f));
    view.filters.push_back(*typeButton);

    // Cutoff
    auto* cutoffSlider = createSlider("filter_cutoff_" + std::to_string(view.trackId),
                                    view.position + glm::vec3(x, y - 0.1f, 0.0f),
                                    glm::vec3(0.2f, 0.02f, 0.02f));
    view.filters.push_back(*cutoffSlider);

    // Resonance
    auto* resonanceKnob = createKnob("filter_resonance_" + std::to_string(view.trackId),
                                   view.position + glm::vec3(x, y - 0.2f, 0.0f),
                                   0.03f);
    view.filters.push_back(*resonanceKnob);
}

void VRUI::createEnvelopeControls(SynthesizerView& view, const SynthesizerConfig::EnvelopeConfig& config, const std::string& name) {
    float x = 0.0f;
    float y = -0.1f;
    float spacing = 0.15f;

    // Hüllkurven-Name
    auto* nameText = createText(name, view.position + glm::vec3(x, y, 0.0f), 0.05f);
    view.envelopes.push_back(*nameText);

    // Attack
    auto* attackSlider = createSlider("env_attack_" + std::to_string(view.trackId),
                                    view.position + glm::vec3(x, y - spacing, 0.0f),
                                    glm::vec3(0.15f, 0.02f, 0.02f));
    view.envelopes.push_back(*attackSlider);

    // Decay
    auto* decaySlider = createSlider("env_decay_" + std::to_string(view.trackId),
                                   view.position + glm::vec3(x, y - spacing * 2, 0.0f),
                                   glm::vec3(0.15f, 0.02f, 0.02f));
    view.envelopes.push_back(*decaySlider);

    // Sustain
    auto* sustainSlider = createSlider("env_sustain_" + std::to_string(view.trackId),
                                     view.position + glm::vec3(x, y - spacing * 3, 0.0f),
                                     glm::vec3(0.15f, 0.02f, 0.02f));
    view.envelopes.push_back(*sustainSlider);

    // Release
    auto* releaseSlider = createSlider("env_release_" + std::to_string(view.trackId),
                                     view.position + glm::vec3(x, y - spacing * 4, 0.0f),
                                     glm::vec3(0.15f, 0.02f, 0.02f));
    view.envelopes.push_back(*releaseSlider);
}

void VRUI::createLFOControls(SynthesizerView& view, const SynthesizerConfig::LFOConfig& config, int index) {
    float x = -0.4f + index * 0.3f;
    float y = -0.3f;

    // LFO-Wellenform
    auto* waveformButton = createButton("lfo_waveform_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                      view.position + glm::vec3(x, y, 0.0f),
                                      glm::vec3(0.1f));
    view.lfos.push_back(*waveformButton);

    // Rate
    auto* rateSlider = createSlider("lfo_rate_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                  view.position + glm::vec3(x, y - 0.1f, 0.0f),
                                  glm::vec3(0.1f, 0.02f, 0.02f));
    view.lfos.push_back(*rateSlider);

    // Depth
    auto* depthKnob = createKnob("lfo_depth_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                               view.position + glm::vec3(x, y - 0.2f, 0.0f),
                               0.03f);
    view.lfos.push_back(*depthKnob);
}

void VRUI::createEffectControls(SynthesizerView& view, const SynthesizerConfig::EffectConfig& config, int index) {
    float x = -0.4f + index * 0.3f;
    float y = -0.5f;

    // Effekt-Typ
    auto* typeButton = createButton("effect_type_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                  view.position + glm::vec3(x, y, 0.0f),
                                  glm::vec3(0.1f));
    view.effects.push_back(*typeButton);

    // Effekt-Parameter
    for (const auto& param : config.parameters) {
        auto* paramSlider = createSlider("effect_" + param.first + "_" + std::to_string(view.trackId) + "_" + std::to_string(index),
                                       view.position + glm::vec3(x, y - 0.1f - 0.1f * view.effects.size(), 0.0f),
                                       glm::vec3(0.1f, 0.02f, 0.02f));
        view.effects.push_back(*paramSlider);
    }
}

void VRUI::updateOscillatorControls(UIElement& element, const SynthesizerConfig::OscillatorConfig& config) {
    if (element.id.find("type") != std::string::npos) {
        // Oszillator-Typ aktualisieren
        element.text = config.type;
    } else if (element.id.find("mix") != std::string::npos) {
        // Mix-Wert aktualisieren
        element.value = config.mix;
    } else if (element.id.find("detune") != std::string::npos) {
        // Detune-Wert aktualisieren
        element.value = config.detune;
    }
}

void VRUI::updateFilterControls(UIElement& element, const SynthesizerConfig::FilterConfig& config) {
    if (element.id.find("type") != std::string::npos) {
        // Filter-Typ aktualisieren
        element.text = config.type;
    } else if (element.id.find("cutoff") != std::string::npos) {
        // Cutoff-Wert aktualisieren
        element.value = config.cutoff;
    } else if (element.id.find("resonance") != std::string::npos) {
        // Resonance-Wert aktualisieren
        element.value = config.resonance;
    }
}

void VRUI::updateEnvelopeControls(UIElement& element, const SynthesizerConfig::EnvelopeConfig& config) {
    if (element.id.find("attack") != std::string::npos) {
        element.value = config.attack;
    } else if (element.id.find("decay") != std::string::npos) {
        element.value = config.decay;
    } else if (element.id.find("sustain") != std::string::npos) {
        element.value = config.sustain;
    } else if (element.id.find("release") != std::string::npos) {
        element.value = config.release;
    }
}

void VRUI::updateLFOControls(UIElement& element, const SynthesizerConfig::LFOConfig& config) {
    if (element.id.find("waveform") != std::string::npos) {
        element.text = config.waveform;
    } else if (element.id.find("rate") != std::string::npos) {
        element.value = config.rate;
    } else if (element.id.find("depth") != std::string::npos) {
        element.value = config.depth;
    }
}

void VRUI::updateEffectControls(UIElement& element, const SynthesizerConfig::EffectConfig& config) {
    if (element.id.find("type") != std::string::npos) {
        element.text = config.type;
    } else {
        // Effekt-Parameter aktualisieren
        for (const auto& param : config.parameters) {
            if (element.id.find(param.first) != std::string::npos) {
                element.value = param.second;
                break;
            }
        }
    }
}

void VRUI::renderSynthesizerView(const SynthesizerView& view) {
    // Synthesizer-Hintergrund rendern
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), view.position);
    modelMatrix = glm::scale(modelMatrix, view.size);
    
    // Synthesizer-Name rendern
    for (const auto& control : view.controls) {
        if (control.type == UIElement::Type::Text) {
            renderText(TextElement{control.text, control.position, control.scale});
        } else {
            renderElement(control);
        }
    }

    // Oszillatoren rendern
    for (const auto& osc : view.oscillators) {
        renderElement(osc);
    }

    // Filter rendern
    for (const auto& filter : view.filters) {
        renderElement(filter);
    }

    // Hüllkurven rendern
    for (const auto& env : view.envelopes) {
        renderElement(env);
    }

    // LFOs rendern
    for (const auto& lfo : view.lfos) {
        renderElement(lfo);
    }

    // Effekte rendern
    for (const auto& effect : view.effects) {
        renderElement(effect);
    }
}

VRUI::ParameterRange VRUI::ParameterValidation::getRange(const std::string& paramName) {
    static const std::map<std::string, ParameterRange> ranges = {
        // Allgemeine Parameter
        {"volume", {0.0f, 1.0f, 0.7f, ""}},
        {"pan", {-1.0f, 1.0f, 0.0f, ""}},
        
        // Oszillator-Parameter
        {"osc_mix", {0.0f, 1.0f, 0.5f, ""}},
        {"osc_detune", {-12.0f, 12.0f, 0.0f, "st"}},
        
        // Filter-Parameter
        {"filter_cutoff", {20.0f, 20000.0f, 1000.0f, "Hz"}},
        {"filter_resonance", {0.0f, 20.0f, 1.0f, ""}},
        
        // Hüllkurven-Parameter
        {"env_attack", {0.001f, 10.0f, 0.1f, "s"}},
        {"env_decay", {0.001f, 10.0f, 0.1f, "s"}},
        {"env_sustain", {0.0f, 1.0f, 0.7f, ""}},
        {"env_release", {0.001f, 10.0f, 0.1f, "s"}},
        
        // LFO-Parameter
        {"lfo_rate", {0.1f, 20.0f, 1.0f, "Hz"}},
        {"lfo_depth", {0.0f, 1.0f, 0.5f, ""}},
        
        // Effekt-Parameter
        {"effect_mix", {0.0f, 1.0f, 0.5f, ""}},
        {"effect_feedback", {0.0f, 0.99f, 0.3f, ""}},
        {"effect_time", {0.001f, 5.0f, 0.3f, "s"}}
    };

    auto it = ranges.find(paramName);
    if (it != ranges.end()) {
        return it->second;
    }
    
    // Standard-Bereich für unbekannte Parameter
    return {0.0f, 1.0f, 0.5f, ""};
}

bool VRUI::ParameterValidation::validateValue(const std::string& paramName, float value) {
    auto range = getRange(paramName);
    return value >= range.min && value <= range.max;
}

float VRUI::ParameterValidation::normalizeValue(const std::string& paramName, float value) {
    auto range = getRange(paramName);
    return (value - range.min) / (range.max - range.min);
}

float VRUI::ParameterValidation::denormalizeValue(const std::string& paramName, float normalizedValue) {
    auto range = getRange(paramName);
    return range.min + normalizedValue * (range.max - range.min);
}

void VRUI::initializeEasingFunctions() {
    pImpl->easingFunctions["linear"] = [](float t) { return t; };
    pImpl->easingFunctions["easeInQuad"] = [](float t) { return t * t; };
    pImpl->easingFunctions["easeOutQuad"] = [](float t) { return t * (2.0f - t); };
    pImpl->easingFunctions["easeInOutQuad"] = [](float t) { 
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; 
    };
    pImpl->easingFunctions["easeInCubic"] = [](float t) { return t * t * t; };
    pImpl->easingFunctions["easeOutCubic"] = [](float t) { 
        float f = t - 1.0f;
        return f * f * f + 1.0f;
    };
    pImpl->easingFunctions["easeInOutCubic"] = [](float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    };
}

void VRUI::initializeAudioSystem() {
#ifdef USE_JACK
    try {
        pImpl->jackClient = jack_client_open("VR_DAW", JackNoStartServer, nullptr);
        if (!pImpl->jackClient) {
            throw std::runtime_error("JACK-Client konnte nicht erstellt werden");
        }
        
        pImpl->inputPort = jack_port_register(pImpl->jackClient, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        pImpl->outputPort = jack_port_register(pImpl->jackClient, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        
        if (!pImpl->inputPort || !pImpl->outputPort) {
            throw std::runtime_error("JACK-Ports konnten nicht registriert werden");
        }

        // Audio-Callback registrieren
        jack_set_process_callback(pImpl->jackClient, processAudio, this);
        
        // JACK-Client aktivieren
        if (jack_activate(pImpl->jackClient) != 0) {
            throw std::runtime_error("JACK-Client konnte nicht aktiviert werden");
        }
    } catch (const std::exception& e) {
        logError("Audio-System-Initialisierungsfehler: " + std::string(e.what()));
        throw;
    }
#endif
}

int VRUI::processAudio(jack_nframes_t nframes, void* arg) {
    auto* ui = static_cast<VRUI*>(arg);
    if (!ui || !ui->initialized) return 0;

#ifdef USE_JACK
    // Audio-Daten verarbeiten
    float* input = static_cast<float*>(jack_port_get_buffer(ui->pImpl->inputPort, nframes));
    float* output = static_cast<float*>(jack_port_get_buffer(ui->pImpl->outputPort, nframes));
    
    if (input && output) {
        // Audio-Engine verarbeiten
        auto& engine = AudioEngine::getInstance();
        engine.processAudio(input, output, nframes);
        
        // UI-Elemente aktualisieren
        ui->updateAudioVisualization(output, nframes);
    }
#endif

    return 0;
}

void VRUI::updateAudioVisualization(const float* audioData, size_t numFrames) {
    if (!initialized) return;

    // Waveform-Daten für alle Synthesizer-Views aktualisieren
    for (auto& view : pImpl->synthesizerViews) {
        std::vector<float> waveform;
        waveform.reserve(numFrames);
        
        // Audio-Daten für diesen Synthesizer extrahieren
        for (size_t i = 0; i < numFrames; ++i) {
            waveform.push_back(audioData[i]);
        }
        
        // Waveform-UI-Elemente aktualisieren
        for (auto& element : pImpl->elements) {
            if (element.type == UIElement::Type::Waveform && 
                element.id.find(std::to_string(view.trackId)) != std::string::npos) {
                setWaveformData(element.id, waveform);
            }
        }
    }
}

void VRUI::handleAudioEvent(const AudioEvent& event) {
    if (!initialized) return;

    // Audio-Event an entsprechende UI-Elemente weiterleiten
    switch (event.type) {
        case AudioEvent::Type::NoteOn: {
            // Note-On UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == event.channel) {
                    // Oszillator-Controls aktualisieren
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("type") != std::string::npos) {
                            float normalizedNote = event.value1 / 127.0f;
                            osc.value = ParameterValidation::denormalizeValue("osc_mix", normalizedNote);
                        }
                    }
                }
            }
            break;
        }
        case AudioEvent::Type::NoteOff: {
            // Note-Off UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == event.channel) {
                    // Oszillator-Controls zurücksetzen
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("type") != std::string::npos) {
                            osc.value = ParameterValidation::getRange("osc_mix").defaultValue;
                        }
                    }
                }
            }
            break;
        }
        case AudioEvent::Type::ControlChange: {
            // Control-Change UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == event.channel) {
                    // Parameter basierend auf Controller-Nummer aktualisieren
                    switch (event.value1) {
                        case 1: // Modulation
                            for (auto& lfo : view.lfos) {
                                if (lfo.id.find("depth") != std::string::npos) {
                                    float normalizedDepth = event.value2 / 127.0f;
                                    lfo.value = ParameterValidation::denormalizeValue("lfo_depth", normalizedDepth);
                                }
                            }
                            break;
                        case 7: // Volume
                            for (auto& control : view.controls) {
                                if (control.id.find("volume") != std::string::npos) {
                                    float normalizedVolume = event.value2 / 127.0f;
                                    control.value = ParameterValidation::denormalizeValue("volume", normalizedVolume);
                                }
                            }
                            break;
                        case 10: // Pan
                            for (auto& control : view.controls) {
                                if (control.id.find("pan") != std::string::npos) {
                                    float normalizedPan = (event.value2 - 64) / 64.0f;
                                    control.value = ParameterValidation::denormalizeValue("pan", normalizedPan);
                                }
                            }
                            break;
                        case 74: // Filter Cutoff
                            for (auto& filter : view.filters) {
                                if (filter.id.find("cutoff") != std::string::npos) {
                                    float normalizedCutoff = event.value2 / 127.0f;
                                    filter.value = ParameterValidation::denormalizeValue("filter_cutoff", normalizedCutoff);
                                }
                            }
                            break;
                        case 71: // Filter Resonance
                            for (auto& filter : view.filters) {
                                if (filter.id.find("resonance") != std::string::npos) {
                                    float normalizedResonance = event.value2 / 127.0f;
                                    filter.value = ParameterValidation::denormalizeValue("filter_resonance", normalizedResonance);
                                }
                            }
                            break;
                    }
                }
            }
            break;
        }
        case AudioEvent::Type::PitchBend: {
            // Pitch-Bend UI-Elemente aktualisieren
            for (auto& view : pImpl->synthesizerViews) {
                if (view.trackId == event.channel) {
                    // Oszillator-Detune aktualisieren
                    for (auto& osc : view.oscillators) {
                        if (osc.id.find("detune") != std::string::npos) {
                            float normalizedDetune = (event.value1 + event.value2 * 128) / 16384.0f;
                            osc.value = ParameterValidation::denormalizeValue("osc_detune", normalizedDetune);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void VRUI::logError(const std::string& error) {
    pImpl->lastError = error;
    pImpl->hasError = true;
    pImpl->errorLog.push_back(error);
    
    if (pImpl->errorLog.size() > 100) {
        pImpl->errorLog.erase(pImpl->errorLog.begin());
    }
}

std::string VRUI::getLastError() const {
    return pImpl->lastError;
}

bool VRUI::hasError() const {
    return pImpl->hasError;
}

std::vector<std::string> VRUI::getErrorLog() const {
    return pImpl->errorLog;
}

void VRUI::clearErrors() {
    pImpl->lastError.clear();
    pImpl->hasError = false;
    pImpl->errorLog.clear();
}

} // namespace VR_DAW 