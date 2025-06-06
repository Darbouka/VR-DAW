#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "VRController.hpp"

namespace VR_DAW {

class VRInput {
public:
    VRInput();
    ~VRInput();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update();

    // Eingabe-Status
    bool isInitialized() const;
    bool isConnected() const;
    std::string getStatus() const;

    // Controller-Management
    void setControllerModel(const std::string& model);
    void setControllerScale(float scale);
    void setControllerOffset(const glm::vec3& offset);
    void setControllerRotation(const glm::quat& rotation);

    // Controller-Tracking
    glm::vec3 getControllerPosition(int controllerIndex) const;
    glm::quat getControllerRotation(int controllerIndex) const;
    glm::vec3 getControllerVelocity(int controllerIndex) const;
    glm::vec3 getControllerAngularVelocity(int controllerIndex) const;
    bool isControllerTracked(int controllerIndex) const;

    // Controller-Eingaben
    bool isButtonPressed(int controllerIndex, int button) const;
    bool isButtonTouched(int controllerIndex, int button) const;
    float getTriggerValue(int controllerIndex) const;
    float getGripValue(int controllerIndex) const;
    glm::vec2 getThumbstickValue(int controllerIndex) const;

    // Haptisches Feedback
    void triggerHapticPulse(int controllerIndex, float duration, float frequency, float amplitude);
    void stopHapticPulse(int controllerIndex);

    // Hand-Tracking
    struct HandJoint {
        glm::vec3 position;
        glm::quat rotation;
        float confidence;
    };

    struct Hand {
        std::vector<HandJoint> joints;
        bool isTracked;
        float confidence;
    };

    void enableHandTracking(bool enable);
    bool isHandTrackingEnabled() const;
    Hand getHand(int handIndex) const;
    bool isHandTracked(int handIndex) const;

    // Gesten-Erkennung
    struct Gesture {
        std::string name;
        float confidence;
        std::vector<glm::vec3> positions;
        std::vector<glm::quat> rotations;
    };

    void registerGesture(const std::string& name, std::function<void(const Gesture&)> callback);
    void unregisterGesture(const std::string& name);
    bool isGestureRecognized(const std::string& name) const;

    // Bewegungs-Tracking
    struct MotionData {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        std::vector<glm::vec3> jointPositions;
        std::vector<glm::quat> jointRotations;
    };

    void startMotionTracking();
    void stopMotionTracking();
    bool isMotionTrackingEnabled() const;
    MotionData getMotionData() const;

    // Event-System
    struct InputEvent {
        std::string type;
        int controllerIndex;
        int button;
        float value;
        glm::vec3 position;
        glm::quat rotation;
    };

    void registerEventHandler(const std::string& eventType, std::function<void(const InputEvent&)> handler);
    void unregisterEventHandler(const std::string& eventType);
    void processEvents();

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void renderDebugShapes();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool connected;
    bool debugEnabled;
    bool handTrackingEnabled;
    bool motionTrackingEnabled;

    // Controller-Status
    std::vector<VRController::ControllerState> controllerStates;
    std::vector<Hand> hands;
    MotionData motionData;

    // Event-Handler
    std::map<std::string, std::vector<std::function<void(const InputEvent&)>>> eventHandlers;
    std::map<std::string, std::function<void(const Gesture&)>> gestureCallbacks;

    void updateControllers();
    void updateHands();
    void updateMotion();
    void processGestures();
    void renderDebugInfo();
};

} // namespace VR_DAW 