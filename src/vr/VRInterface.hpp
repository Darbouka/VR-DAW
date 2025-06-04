#pragma once

#include <memory>
#include <vector>
#include <string>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <glm/glm.hpp>

namespace VR_DAW {

class VRInterface {
public:
    static VRInterface& getInstance();

    // Initialisierung und Konfiguration
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Betriebsmodus
    enum class OperationMode {
        VR,         // VR-Modus mit Brille
        Desktop,    // Desktop-Modus ohne Brille
        Hybrid      // Kombinierter Modus
    };
    
    void setOperationMode(OperationMode mode);
    OperationMode getOperationMode() const;
    
    // Desktop-Modus-spezifische Funktionen
    void setDesktopViewport(int width, int height);
    void setDesktopCamera(const glm::vec3& position, const glm::quat& rotation);
    void setDesktopScale(float scale);
    
    // VR-Session-Management
    void startSession();
    void endSession();
    
    // Controller-Interaktion
    void handleControllerInput();
    void updateControllerState();
    
    // VR-UI-Elemente
    void createUIElement(const std::string& type, const std::string& id);
    void updateUIElement(const std::string& id, const void* data);
    void removeUIElement(const std::string& id);
    
    // 3D-Raum-Interaktion
    void setObjectPosition(const std::string& id, const float* position);
    void setObjectRotation(const std::string& id, const float* rotation);
    void setObjectScale(const std::string& id, const float* scale);
    
    // Hand-Tracking
    void enableHandTracking(bool enable);
    void updateHandPoses();
    
    // Raum-Tracking
    void enableRoomTracking(bool enable);
    void updateRoomSpace();
    
    // Rendering
    void renderFrame();
    void updateViewMatrix(const float* viewMatrix);
    void updateProjectionMatrix(const float* projectionMatrix);

private:
    VRInterface() = default;
    ~VRInterface() = default;
    
    VRInterface(const VRInterface&) = delete;
    VRInterface& operator=(const VRInterface&) = delete;
    
    // OpenXR Komponenten
    XrInstance xrInstance = XR_NULL_HANDLE;
    XrSession xrSession = XR_NULL_HANDLE;
    XrSpace xrSpace = XR_NULL_HANDLE;
    
    // Controller
    struct Controller {
        XrActionSet actionSet;
        XrAction gripAction;
        XrAction triggerAction;
        XrAction thumbstickAction;
        XrAction buttonAction;
        XrSpace space;
    };
    std::vector<Controller> controllers;
    
    // Hand-Tracking
    struct HandTracker {
        XrHandTrackerEXT tracker;
        XrHandJointLocationEXT jointLocations[XR_HAND_JOINT_COUNT_EXT];
    };
    std::vector<HandTracker> handTrackers;
    
    // UI-Elemente
    struct UIElement {
        std::string id;
        std::string type;
        void* data;
        float position[3];
        float rotation[4];
        float scale[3];
    };
    std::vector<UIElement> uiElements;
    
    // Rendering
    struct RenderState {
        float viewMatrix[16];
        float projectionMatrix[16];
        std::vector<void*> renderables;
    };
    RenderState renderState;
    
    // Raum-Tracking
    struct RoomSpace {
        float bounds[6];  // minX, minY, minZ, maxX, maxY, maxZ
        bool isTracking;
    };
    RoomSpace roomSpace;
    
    // Desktop-Modus-Komponenten
    struct DesktopState {
        int viewportWidth;
        int viewportHeight;
        glm::vec3 cameraPosition;
        glm::quat cameraRotation;
        float scale;
    } desktopState;
    
    // Betriebsmodus
    OperationMode currentMode = OperationMode::Desktop;
    
    // Interne Hilfsfunktionen
    void initializeControllers();
    XrSystemId getSystemId();
    void updateDesktopView();
    void renderDesktopFrame();
    void renderVRFrame();
}; 