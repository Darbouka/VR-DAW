#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <openvr.h>
#include <GL/glew.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace VR_DAW {

class VRInterface {
public:
    VRInterface();
    ~VRInterface();
    
    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    
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
        GLuint vao;
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
        bool hapticFeedback = false;  // Standardmäßig deaktiviert
        bool handTracking = true;
        bool spatialAudio = true;
        bool adaptiveRendering = true;
        float renderScale = 1.0f;
        int msaaSamples = 4;
        bool asyncReprojection = true;
        bool motionSmoothing = true;
        bool foveatedRendering = false;  // Standardmäßig deaktiviert
        bool eyeTracking = false;  // Standardmäßig deaktiviert
        bool brainControl = false;  // Standardmäßig deaktiviert
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
    void mapMotionToAudioParameters(juce::AudioProcessor& processor);
    
    // Interaktion
    void handleControllerInput(const vr::VRControllerState_t& state);
    void handleHandGestures(const std::vector<glm::vec3>& handPositions);
    void handleBodyMovement(const MotionData& motionData);
    
    // Audio-Integration
    void updateSpatialAudio(const glm::vec3& listenerPosition, const glm::quat& listenerRotation);
    void mapMotionToEffects(juce::AudioProcessor& processor);
    
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

    // Erweiterte Rendering-Funktionen
    void initializeRendering();
    void shutdownRendering();
    void updateRendering();
    void setFoveatedRendering(bool enable);
    void setRenderScale(float scale);
    void setMSAASamples(int samples);
    void enableAsyncReprojection(bool enable);
    void enableMotionSmoothing(bool enable);

private:
    // VR-System
    vr::IVRSystem* vrSystem;
    vr::IVRCompositor* vrCompositor;
    vr::IVRInput* vrInput;
    
    // Raum und Fenster
    Room currentRoom;
    std::vector<Window> windows;
    std::map<std::string, int> windowToWallMap;
    
    // Bewegungs-Tracking
    MotionData currentMotion;
    bool isTrackingActive;
    std::vector<vr::TrackedDevicePose_t> devicePoses;
    
    // Rendering
    struct RenderState {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    } renderState;
    
    GLuint shaderProgram;
    GLuint textureShaderProgram;
    GLuint motionShaderProgram;
    
    // Shader-Uniforms
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint diffuseColorLoc;
    GLint specularColorLoc;
    GLint shininessLoc;
    GLint transparencyLoc;
    
    GLint textureModelLoc;
    GLint textureViewLoc;
    GLint textureProjectionLoc;
    GLint textureLoc;
    
    // Hilfsfunktionen
    void initializeVR();
    void initializeControllers();
    void initializeShaders();
    GLuint compileShaderProgram(const char* vertexSource, const char* fragmentSource);
    Mesh loadControllerModel(vr::ETrackedControllerRole role);
    void updateControllerStates();
    void processControllerInput();
    void updateWindowTransforms();
    void calculateSpatialAudio();

    VROptimizations currentVROptimizations;
    std::unique_ptr<class HapticController> hapticController;
    std::unique_ptr<class HandTracker> handTracker;
    std::unique_ptr<class SpatialAudioRenderer> spatialAudioRenderer;
    std::unique_ptr<class AdaptiveRenderer> adaptiveRenderer;
    bool foveatedRenderingEnabled = false;
    float renderScale = 1.0f;
    int msaaSamples = 4;
    bool asyncReprojectionEnabled = false;
    bool motionSmoothingEnabled = false;
};

class AdaptiveRenderer {
public:
    AdaptiveRenderer();
    ~AdaptiveRenderer();

    void initialize();
    void shutdown();
    void render();
    void enableFoveatedRendering(bool enable);
    void setFoveatedRadius(float radius);
    void setRenderScale(float scale);
    void setMSAASamples(int samples);
    void enableAsyncReprojection(bool enable);
    void enableMotionSmoothing(bool enable);

private:
    struct RenderTarget {
        GLuint framebuffer;
        GLuint colorTexture;
        GLuint depthTexture;
        glm::vec2 resolution;
    };

    struct FoveatedRendering {
        float radius;
        float falloff;
        int qualityLevels;
        std::vector<RenderTarget> qualityTargets;
    };

    FoveatedRendering foveatedRendering;
    float renderScale = 1.0f;
    int msaaSamples = 4;
    bool asyncReprojectionEnabled = false;
    bool motionSmoothingEnabled = false;
    bool isInitialized = false;
};

} // namespace VR_DAW 