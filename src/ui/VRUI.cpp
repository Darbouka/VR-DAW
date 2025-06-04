#include "VRUI.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace VR_DAW {

VRUI& VRUI::getInstance() {
    static VRUI instance;
    return instance;
}

void VRUI::createElement(const UIElement& element) {
    elements.push_back(element);
    arrangeElements();
}

void VRUI::removeElement(const std::string& id) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        elements.erase(it);
        arrangeElements();
    }
}

void VRUI::updateElement(const UIElement& element) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&element](const auto& e) {
            return e.id == element.id;
        });
    
    if (it != elements.end()) {
        *it = element;
    }
}

VRUI::UIElement* VRUI::getElement(const std::string& id) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    return it != elements.end() ? &(*it) : nullptr;
}

void VRUI::setLayout(const Layout& layout) {
    currentLayout = layout;
    arrangeElements();
}

void VRUI::arrangeElements() {
    if (elements.empty()) {
        return;
    }
    
    if (currentLayout.isCircular) {
        // Kreisförmiges Layout
        float angleStep = 2.0f * M_PI / elements.size();
        float currentAngle = 0.0f;
        
        for (auto& element : elements) {
            float x = currentLayout.center.x + currentLayout.radius * std::cos(currentAngle);
            float y = currentLayout.center.y;
            float z = currentLayout.center.z + currentLayout.radius * std::sin(currentAngle);
            
            element.position = glm::vec3(x, y, z);
            
            // Rotation zum Zentrum
            glm::vec3 direction = glm::normalize(currentLayout.center - element.position);
            element.rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
            
            currentAngle += angleStep;
        }
    } else {
        // Lineares Layout
        float currentX = currentLayout.center.x - (elements.size() - 1) * currentLayout.spacing / 2.0f;
        
        for (auto& element : elements) {
            element.position = glm::vec3(currentX, currentLayout.center.y, currentLayout.center.z);
            element.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // Keine Rotation
            
            currentX += currentLayout.spacing;
        }
    }
}

void VRUI::handleRaycast(const glm::vec3& origin, const glm::vec3& direction) {
    for (auto& element : elements) {
        if (!element.isVisible || !element.isInteractive) {
            continue;
        }
        
        // Einfache Raycast-Implementierung
        glm::vec3 elementCenter = element.position;
        glm::vec3 elementNormal = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * element.rotation);
        
        float denom = glm::dot(direction, elementNormal);
        if (std::abs(denom) > 1e-6) {
            glm::vec3 p0l0 = elementCenter - origin;
            float t = glm::dot(p0l0, elementNormal) / denom;
            
            if (t >= 0.0f) {
                glm::vec3 intersection = origin + direction * t;
                if (isPointInElement(intersection, element)) {
                    handleElementInteraction(element);
                    break;
                }
            }
        }
    }
}

void VRUI::handleControllerInput(const glm::vec3& position, const glm::quat& rotation) {
    for (auto& element : elements) {
        if (!element.isVisible || !element.isInteractive) {
            continue;
        }
        
        // Controller-Interaktion mit UI-Elementen
        glm::vec3 controllerForward = glm::vec3(0.0f, 0.0f, 1.0f) * rotation;
        handleRaycast(position, controllerForward);
    }
}

void VRUI::handleHandGesture(const std::string& gesture) {
    // Handgesten-Interaktion mit UI-Elementen
    for (auto& element : elements) {
        if (!element.isVisible || !element.isInteractive) {
            continue;
        }
        
        if (gesture == "grab" && element.onClick) {
            element.onClick();
        }
    }
}

void VRUI::render() {
    for (const auto& element : elements) {
        if (element.isVisible) {
            renderElement(element);
        }
    }
}

void VRUI::update() {
    float deltaTime = 1.0f / 60.0f;  // Annahme: 60 FPS
    
    for (auto& element : elements) {
        updateElementTransform(element, deltaTime);
    }
    
    updateAnimations(deltaTime);
}

void VRUI::setTheme(const Theme& theme) {
    currentTheme = theme;
    applyTheme(theme);
}

VRUI::Theme VRUI::getTheme() const {
    return currentTheme;
}

void VRUI::addAnimation(const Animation& animation) {
    activeAnimations.push_back(animation);
}

void VRUI::removeAnimation(const std::string& elementId) {
    auto it = std::find_if(activeAnimations.begin(), activeAnimations.end(),
        [&elementId](const auto& animation) {
            return animation.elementId == elementId;
        });
    
    if (it != activeAnimations.end()) {
        activeAnimations.erase(it);
    }
}

void VRUI::playHapticFeedback(const std::string& elementId, float intensity) {
    // Haptisches Feedback nur spielen, wenn verfügbar
    if (currentUIOptimizations.hapticFeedback && hapticFeedback) {
        hapticFeedback->playFeedback(elementId, intensity);
    } else {
        // Fallback: Visuelle und auditive Rückmeldung
        playSoundFeedback(elementId, "click");
        // Visuelle Rückmeldung durch Animation
        Animation feedbackAnimation;
        feedbackAnimation.elementId = elementId;
        feedbackAnimation.duration = 0.2f;
        feedbackAnimation.onComplete = []() {};
        addAnimation(feedbackAnimation);
    }
}

void VRUI::playSoundFeedback(const std::string& elementId, const std::string& soundId) {
    // Sound-Feedback implementieren
    // Hier würde die Integration mit dem Audio-System erfolgen
}

void VRUI::updateElementTransform(UIElement& element, float deltaTime) {
    // Element-Transformation aktualisieren
    // Hier könnte die Physik-Integration erfolgen
}

void VRUI::handleElementInteraction(UIElement& element) {
    if (element.onClick) {
        element.onClick();
    }
}

void VRUI::renderElement(const UIElement& element) {
    // UI-Element rendern
    // Hier würde die Integration mit dem Rendering-System erfolgen
}

bool VRUI::isPointInElement(const glm::vec3& point, const UIElement& element) {
    // Einfache Kollisionserkennung
    glm::vec3 localPoint = point - element.position;
    localPoint = localPoint * glm::inverse(element.rotation);
    
    // Annahme: UI-Element ist ein 2D-Rechteck in der XY-Ebene
    float halfWidth = element.scale.x / 2.0f;
    float halfHeight = element.scale.y / 2.0f;
    
    return std::abs(localPoint.x) <= halfWidth &&
           std::abs(localPoint.y) <= halfHeight &&
           std::abs(localPoint.z) <= 0.01f;  // Dünne Ebene
}

void VRUI::updateAnimations(float deltaTime) {
    auto it = activeAnimations.begin();
    while (it != activeAnimations.end()) {
        auto* element = getElement(it->elementId);
        if (!element) {
            it = activeAnimations.erase(it);
            continue;
        }
        
        // Position interpolieren
        element->position = glm::mix(element->position, it->targetPosition, deltaTime / it->duration);
        
        // Rotation interpolieren
        element->rotation = glm::slerp(element->rotation, it->targetRotation, deltaTime / it->duration);
        
        // Skalierung interpolieren
        element->scale = glm::mix(element->scale, it->targetScale, deltaTime / it->duration);
        
        it->duration -= deltaTime;
        if (it->duration <= 0.0f) {
            if (it->onComplete) {
                it->onComplete();
            }
            it = activeAnimations.erase(it);
        } else {
            ++it;
        }
    }
}

void VRUI::applyTheme(const Theme& theme) {
    for (auto& element : elements) {
        // Theme auf UI-Elemente anwenden
        // Hier würde die Integration mit dem Rendering-System erfolgen
    }
}

VRUI::VRUI() {
    initialize();
}

VRUI::~VRUI() {
    shutdown();
}

void VRUI::initialize() {
    // UI-Komponenten initialisieren
    adaptiveLayout = std::make_unique<AdaptiveLayout>();
    gestureController = std::make_unique<GestureController>();
    
    // Haptic Feedback nur initialisieren, wenn verfügbar
    if (currentUIOptimizations.hapticFeedback) {
        hapticFeedback = std::make_unique<HapticFeedback>();
        hapticFeedback->initialize();
    }
    
    // Standard-Parameter setzen
    currentParams = {
        true,   // adaptiveLayout
        true,   // gestureControl
        currentUIOptimizations.hapticFeedback,  // hapticFeedback
        60.0f,  // updateRate
        0.1f    // elementSpacing
    };
}

void VRUI::shutdown() {
    adaptiveLayout.reset();
    gestureController.reset();
    hapticFeedback.reset();
}

void VRUI::update() {
    // UI aktualisieren
    if (adaptiveLayout) {
        adaptiveLayout->update();
    }
    
    // Gesten verarbeiten
    if (gestureController) {
        gestureController->processGestures();
    }
    
    // Haptisches Feedback aktualisieren
    if (hapticFeedback) {
        hapticFeedback->update();
    }
}

void VRUI::render() {
    // UI-Elemente rendern
    for (const auto& element : elements) {
        renderElement(element);
    }
}

void VRUI::setAdaptiveLayout(bool enable) {
    currentParams.adaptiveLayout = enable;
    if (adaptiveLayout) {
        adaptiveLayout->setEnabled(enable);
    }
}

void VRUI::setGestureControl(bool enable) {
    currentParams.gestureControl = enable;
    if (gestureController) {
        gestureController->setEnabled(enable);
    }
}

void VRUI::setHapticFeedback(bool enable) {
    currentParams.hapticFeedback = enable;
    if (hapticFeedback) {
        hapticFeedback->setEnabled(enable);
    }
}

void VRUI::setUpdateRate(float rate) {
    currentParams.updateRate = rate;
    if (adaptiveLayout) {
        adaptiveLayout->setUpdateRate(rate);
    }
}

void VRUI::setElementSpacing(float spacing) {
    currentParams.elementSpacing = spacing;
    if (adaptiveLayout) {
        adaptiveLayout->setElementSpacing(spacing);
    }
}

void VRUI::createUIElement(const std::string& type, const std::string& id) {
    UIElement element;
    element.id = id;
    element.type = type;
    
    // Standard-Position und -Skalierung
    element.position = glm::vec3(0.0f);
    element.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    element.scale = glm::vec3(1.0f);
    
    elements.push_back(std::move(element));
}

void VRUI::updateUIElement(const std::string& id, const void* data) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        it->data = const_cast<void*>(data);
    }
}

void VRUI::removeUIElement(const std::string& id) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        elements.erase(it);
    }
}

void VRUI::setElementPosition(const std::string& id, const glm::vec3& position) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        it->position = position;
    }
}

void VRUI::setElementRotation(const std::string& id, const glm::quat& rotation) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        it->rotation = rotation;
    }
}

void VRUI::setElementScale(const std::string& id, const glm::vec3& scale) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != elements.end()) {
        it->scale = scale;
    }
}

void VRUI::renderUIElement(const UIElement& element) {
    // Element-Material
    Material elementMaterial;
    elementMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
    elementMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
    elementMaterial.shininess = 32.0f;
    
    // Element-Mesh
    Mesh elementMesh;
    elementMesh.vertices = {
        // Vorderseite
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    elementMesh.indices = {0, 1, 2, 2, 3, 0};
    
    // Element-Modell-Matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), element.position);
    model *= glm::toMat4(element.rotation);
    model = glm::scale(model, element.scale);
    
    // Element rendern
    renderMesh(elementMesh, model, elementMaterial);
}

// AdaptiveLayout-Implementierung
void AdaptiveLayout::update() {
    if (!enabled) return;
    
    // Layout aktualisieren
    arrangeElements();
}

void AdaptiveLayout::arrangeElements() {
    // Elemente anordnen
    // Hier würde die Logik für die adaptive Anordnung implementiert
}

// GestureController-Implementierung
void GestureController::processGestures() {
    if (!enabled) return;
    
    // Gesten verarbeiten
    // Hier würde die Logik für die Gestenerkennung implementiert
}

// HapticFeedback-Implementierung
void HapticFeedback::update() {
    if (!enabled) return;
    
    // Haptisches Feedback aktualisieren
    // Hier würde die Logik für das haptische Feedback implementiert
}

} // namespace VR_DAW 