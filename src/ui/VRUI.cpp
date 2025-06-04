#include "VRUI.hpp"
#include <algorithm>
#include <cmath>

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
    // Haptisches Feedback implementieren
    // Hier würde die Integration mit dem VR-Controller-System erfolgen
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

} // namespace VR_DAW 