#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>

namespace VR_DAW {

class VRController {
public:
    VRController();
    ~VRController();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update();

    // Controller-Status
    bool isInitialized() const;
    bool isConnected() const;
    std::string getStatus() const;

    // Controller-Eigenschaften
    struct ControllerState {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 velocity;
        glm::vec3 angularVelocity;
        bool isTracking;
        float batteryLevel;
        bool isCharging;
    };

    // Controller-Management
    void setControllerModel(const std::string& modelPath);
    void setControllerScale(float scale);
    void setControllerOffset(const glm::vec3& offset);
    void setControllerRotation(const glm::quat& rotation);

    // Tracking
    ControllerState getControllerState(int controllerIndex) const;
    glm::vec3 getPosition(int controllerIndex) const;
    glm::quat getRotation(int controllerIndex) const;
    glm::vec3 getVelocity(int controllerIndex) const;
    glm::vec3 getAngularVelocity(int controllerIndex) const;
    bool isTracking(int controllerIndex) const;

    // Haptisches Feedback
    void triggerHapticPulse(int controllerIndex, float duration, float frequency, float amplitude);
    void stopHapticPulse(int controllerIndex);

    // Batterie-Status
    float getBatteryLevel(int controllerIndex) const;
    bool isCharging(int controllerIndex) const;

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();

    // Interaktion
    bool isGrabbing() const;
    bool isPointing() const;
    glm::vec3 getPointDirection() const;

    // Callbacks
    using ButtonCallback = std::function<void(int button, bool pressed)>;
    using TriggerCallback = std::function<void(float value)>;
    using ThumbstickCallback = std::function<void(const glm::vec2& value)>;
    
    void setButtonCallback(ButtonCallback callback);
    void setTriggerCallback(TriggerCallback callback);
    void setThumbstickCallback(ThumbstickCallback callback);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool debugEnabled;
    
    // Controller-Status
    std::vector<ControllerState> controllerStates;
    std::vector<bool> hapticActive;
    
    // Controller-Modelle
    std::string modelPath;
    float scale;
    glm::vec3 offset;
    glm::quat rotation;
    
    void updateControllerStates();
    void updateHapticFeedback();
    void renderDebugInfo();

    // OpenVR-spezifische Implementierung
    #ifdef USE_OPENVR
    void initializeOpenVR();
    void updateOpenVR();
    #endif
};

} // namespace VR_DAW
