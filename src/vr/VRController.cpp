#include "VRController.hpp"
#include <algorithm>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace VR_DAW {

VRController::VRController()
    : initialized(false)
    , debugEnabled(false)
    , scale(1.0f)
    , offset(0.0f)
    , rotation(1.0f, 0.0f, 0.0f, 0.0f)
{
    controllerStates.resize(2); // Standardmäßig 2 Controller
    hapticActive.resize(2, false);
}

VRController::~VRController() {
    shutdown();
}

bool VRController::initialize() {
    if (initialized) return true;

    // Hier würde die Initialisierung der VR-Controller erfolgen
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
    
    initialized = true;
    return true;
}

void VRController::shutdown() {
    if (!initialized) return;

    // Hier würde das Herunterfahren der VR-Controller erfolgen
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
    
    controllerStates.clear();
    hapticActive.clear();
    
    initialized = false;
}

void VRController::update() {
    if (!initialized) return;

    updateControllerStates();
    updateHapticFeedback();
}

bool VRController::isInitialized() const {
    return initialized;
}

bool VRController::isConnected() const {
    return !controllerStates.empty() && 
           std::any_of(controllerStates.begin(), controllerStates.end(),
                      [](const ControllerState& state) { return state.isTracking; });
}

std::string VRController::getStatus() const {
    if (!initialized) return "Nicht initialisiert";
    if (!isConnected()) return "Nicht verbunden";
    return "Verbunden";
}

void VRController::setControllerModel(const std::string& path) {
    modelPath = path;
}

void VRController::setControllerScale(float newScale) {
    scale = std::max(0.1f, newScale);
}

void VRController::setControllerOffset(const glm::vec3& newOffset) {
    offset = newOffset;
}

void VRController::setControllerRotation(const glm::quat& newRotation) {
    rotation = newRotation;
}

VRController::ControllerState VRController::getControllerState(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return ControllerState();
    }
    return controllerStates[controllerIndex];
}

glm::vec3 VRController::getPosition(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return glm::vec3(0.0f);
    }
    return controllerStates[controllerIndex].position;
}

glm::quat VRController::getRotation(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    return controllerStates[controllerIndex].rotation;
}

glm::vec3 VRController::getVelocity(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return glm::vec3(0.0f);
    }
    return controllerStates[controllerIndex].velocity;
}

glm::vec3 VRController::getAngularVelocity(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return glm::vec3(0.0f);
    }
    return controllerStates[controllerIndex].angularVelocity;
}

bool VRController::isTracking(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return false;
    }
    return controllerStates[controllerIndex].isTracking;
}

void VRController::triggerHapticPulse(int controllerIndex, float duration, float frequency, float amplitude) {
    if (controllerIndex < 0 || controllerIndex >= hapticActive.size()) return;

    // Hier würde das haptische Feedback ausgelöst werden
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
    
    hapticActive[controllerIndex] = true;
}

void VRController::stopHapticPulse(int controllerIndex) {
    if (controllerIndex < 0 || controllerIndex >= hapticActive.size()) return;

    // Hier würde das haptische Feedback gestoppt werden
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
    
    hapticActive[controllerIndex] = false;
}

float VRController::getBatteryLevel(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return 0.0f;
    }
    return controllerStates[controllerIndex].batteryLevel;
}

bool VRController::isCharging(int controllerIndex) const {
    if (controllerIndex < 0 || controllerIndex >= controllerStates.size()) {
        return false;
    }
    return controllerStates[controllerIndex].isCharging;
}

void VRController::enableDebugMode(bool enable) {
    debugEnabled = enable;
}

void VRController::showDebugInfo() {
    if (!initialized || !debugEnabled) return;

    renderDebugInfo();
}

void VRController::updateControllerStates() {
    if (!initialized) return;

    // Hier würde die Aktualisierung der Controller-Zustände erfolgen
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
}

void VRController::updateHapticFeedback() {
    if (!initialized) return;

    // Hier würde die Aktualisierung des haptischen Feedbacks erfolgen
    // z.B. mit OpenVR, Oculus SDK oder anderen VR-APIs
}

void VRController::renderDebugInfo() {
    if (!initialized || !debugEnabled) return;

    // Hier würde das Rendering der Debug-Informationen erfolgen
    // z.B. mit OpenGL, Vulkan oder anderen Grafik-APIs
}

} // namespace VR_DAW
