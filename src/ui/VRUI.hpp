#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

namespace VR_DAW {

class VRUI {
public:
    static VRUI& getInstance();
    
    // UI-Element-Typen
    enum class ElementType {
        Button,
        Slider,
        Knob,
        Display,
        Panel,
        Menu,
        List,
        Grid
    };
    
    // UI-Element-Struktur
    struct UIElement {
        std::string id;
        ElementType type;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        bool isVisible;
        bool isInteractive;
        std::function<void()> onClick;
        std::function<void(float)> onValueChange;
        void* userData;
    };
    
    // UI-Element-Verwaltung
    void createElement(const UIElement& element);
    void removeElement(const std::string& id);
    void updateElement(const UIElement& element);
    UIElement* getElement(const std::string& id);
    
    // UI-Layout
    struct Layout {
        glm::vec3 center;
        float radius;
        float spacing;
        bool isCircular;
    };
    
    void setLayout(const Layout& layout);
    void arrangeElements();
    
    // Interaktion
    void handleRaycast(const glm::vec3& origin, const glm::vec3& direction);
    void handleControllerInput(const glm::vec3& position, const glm::quat& rotation);
    void handleHandGesture(const std::string& gesture);
    
    // Rendering
    void render();
    void update();
    
    // UI-Themes
    struct Theme {
        glm::vec4 backgroundColor;
        glm::vec4 textColor;
        glm::vec4 highlightColor;
        float opacity;
        float scale;
    };
    
    void setTheme(const Theme& theme);
    Theme getTheme() const;
    
    // UI-Animation
    struct Animation {
        std::string elementId;
        glm::vec3 targetPosition;
        glm::quat targetRotation;
        glm::vec3 targetScale;
        float duration;
        std::function<void()> onComplete;
    };
    
    void addAnimation(const Animation& animation);
    void removeAnimation(const std::string& elementId);
    
    // UI-Feedback
    void playHapticFeedback(const std::string& elementId, float intensity);
    void playSoundFeedback(const std::string& elementId, const std::string& soundId);
    
private:
    VRUI() = default;
    ~VRUI() = default;
    
    // Singleton-Pattern
    VRUI(const VRUI&) = delete;
    VRUI& operator=(const VRUI&) = delete;
    
    // Interne Zustandsvariablen
    std::vector<UIElement> elements;
    Layout currentLayout;
    Theme currentTheme;
    std::vector<Animation> activeAnimations;
    
    // Interne Hilfsfunktionen
    void updateElementTransform(UIElement& element, float deltaTime);
    void handleElementInteraction(UIElement& element);
    void renderElement(const UIElement& element);
    bool isPointInElement(const glm::vec3& point, const UIElement& element);
    void updateAnimations(float deltaTime);
    void applyTheme(const Theme& theme);
};

} // namespace VR_DAW 