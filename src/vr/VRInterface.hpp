#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "VRController.hpp"

namespace VR_DAW {

class VRInterface {
public:
    VRInterface();
    ~VRInterface();
    
    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update();
    void render();
    bool isConnected() const;
    
    // Raum-Management
    struct Room {
        glm::vec3 dimensions;
        std::vector<glm::vec3> wallPositions;
        std::vector<glm::quat> wallRotations;
        float windowSize;
    };
    
    // Fenster-Management
    struct Window {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec2 size;
        bool isResizable;
        bool isMovable;
        std::string content;
    };
    
    // Bewegungs-Tracking
    struct MotionData {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        std::vector<glm::vec3> jointPositions;
        std::vector<glm::quat> jointRotations;
    };
    
    // Material
    struct Material {
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        float transparency;
    };
    
    // Mesh
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };
    
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int vao;
    };
    
    // Audio-Parameter
    struct AudioParameters {
        float volume;
        float pan;
        float reverb;
        float delay;
        float filterCutoff;
        float filterResonance;
    };
    
    // Neue VR-Optimierungen
    struct VROptimizations {
        bool hapticFeedback = false;
        bool handTracking = true;
        bool spatialAudio = true;
        bool adaptiveRendering = true;
        float renderScale = 1.0f;
        int msaaSamples = 4;
        bool asyncReprojection = true;
        bool motionSmoothing = true;
        bool foveatedRendering = false;
        bool eyeTracking = false;
        bool brainControl = false;
    };
    
    // Raum-Funktionen
    void createRoom(const Room& room);
    void addWindow(const Window& window);
    void updateWindowPosition(const std::string& windowId, const glm::vec3& position);
    void updateWindowSize(const std::string& windowId, const glm::vec2& size);
    void attachWindowToWall(const std::string& windowId, int wallIndex);
    
    // Bewegungs-Tracking
    void startMotionTracking();
    void updateMotionData();
    
    // Interaktion
    void handleControllerInput(const std::vector<float>& state);
    void handleHandGestures(const std::vector<glm::vec3>& handPositions);
    void handleBodyMovement(const MotionData& motionData);
    
    // Audio-Integration
    void updateSpatialAudio(const glm::vec3& listenerPosition, const glm::quat& listenerRotation);
    
    // Rendering
    void renderRoom();
    void renderWindows();
    void renderMotionData();
    void renderMesh(const Mesh& mesh, const glm::mat4& model, const Material& material);
    void renderVector(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
    void renderWindowContent(const Window& window);
    
    void setVROptimizations(const VROptimizations& optimizations);
    VROptimizations getVROptimizations() const;
    void enableHapticFeedback(bool enable);
    void enableHandTracking(bool enable);
    void enableSpatialAudio(bool enable);
    void setRenderScale(float scale);
    void setMSAASamples(int samples);
    void enableAsyncReprojection(bool enable);
    void enableMotionSmoothing(bool enable);
    void enableFoveatedRendering(bool enable);

    // VR-System-Status
    bool isInitialized() const;
    bool isRunning() const;
    void setRunning(bool running);

    // Controller-Management
    std::shared_ptr<VRController> getLeftController();
    std::shared_ptr<VRController> getRightController();
    void updateControllers();

    // VR-Rendering
    void beginFrame();
    void endFrame();
    void submitFrame();

    // VR-Interaktion
    bool handleInput();
    void processEvents();
    void updateTracking();

    // VR-Konfiguration
    void setRenderResolution(int width, int height);
    void setRefreshRate(float rate);
    void setIPD(float ipd);
    void setWorldScale(float scale);

    // VR-Statistiken
    float getFrameTime() const;
    float getFrameRate() const;
    float getLatency() const;
    int getDroppedFrames() const;

    // Interface-Status
    bool isVisible() const;
    std::string getStatus() const;

    // Interface-Elemente
    struct InterfaceElement {
        std::string id;
        std::string type;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec2 size;
        bool visible;
        bool interactive;
        std::string text;
        std::string texture;
        glm::vec4 color;
    };

    // Interface-Management
    void createElement(const std::string& id, const std::string& type);
    void destroyElement(const std::string& id);
    void updateElement(const std::string& id, const InterfaceElement& element);
    InterfaceElement getElement(const std::string& id) const;

    // Layout-Management
    void setLayout(const std::string& layoutType);
    void updateLayout();
    void setElementPosition(const std::string& id, const glm::vec3& position);
    void setElementRotation(const std::string& id, const glm::quat& rotation);
    void setElementSize(const std::string& id, const glm::vec2& size);

    // Interaktion
    void handleInteraction(const glm::vec3& position, const glm::quat& rotation);
    bool isElementHovered(const std::string& id) const;
    bool isElementClicked(const std::string& id) const;
    void registerInteractionCallback(const std::string& id, std::function<void()> callback);

    // Styling
    void setElementColor(const std::string& id, const glm::vec4& color);
    void setElementTexture(const std::string& id, const std::string& texture);
    void setElementText(const std::string& id, const std::string& text);
    void setElementFont(const std::string& id, const std::string& font);
    void setElementFontSize(const std::string& id, float size);

    // Animation
    void animateElement(const std::string& id, const glm::vec3& targetPosition, float duration);
    void animateElementRotation(const std::string& id, const glm::quat& targetRotation, float duration);
    void animateElementColor(const std::string& id, const glm::vec4& targetColor, float duration);
    void stopAnimation(const std::string& id);

    // Rendering
    void renderInterface();
    void setRenderQuality(int quality);
    void enableDebugRendering(bool enable);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool running;
    bool debugEnabled;
    std::shared_ptr<VRController> leftController;
    std::shared_ptr<VRController> rightController;
    
    // Rendering-Parameter
    int renderWidth;
    int renderHeight;
    float refreshRate;
    float ipd;
    float worldScale;
    
    // Performance-Metriken
    float frameTime;
    float frameRate;
    float latency;
    int droppedFrames;
    
    // Interface-Status
    std::string currentLayout;
    float renderScale;
    int renderQuality;
    
    // Interface-Elemente
    std::map<std::string, InterfaceElement> elements;
    std::map<std::string, std::vector<std::function<void()>>> interactionCallbacks;
    
    void initializeVRSystem();
    void shutdownVRSystem();
    void updatePerformanceMetrics();
    void updateElementTransforms();
    void processInteractions();
    void updateAnimations();
    void renderElement(const InterfaceElement& element);
    void renderDebugInfo();
};

} // namespace VR_DAW 