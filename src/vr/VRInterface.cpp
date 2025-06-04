#include "VRInterface.hpp"
#include <openxr/openxr_platform_defines.h>
#include <openxr/openxr_reflection.h>

namespace VR_DAW {

VRInterface& VRInterface::getInstance() {
    static VRInterface instance;
    return instance;
}

bool VRInterface::initialize() {
    // Standardmäßig im Desktop-Modus starten
    currentMode = OperationMode::Desktop;
    
    // Desktop-Viewport initialisieren
    desktopState.viewportWidth = 1280;
    desktopState.viewportHeight = 720;
    desktopState.cameraPosition = glm::vec3(0.0f, 1.6f, 2.0f);  // Standard-Kameraposition
    desktopState.cameraRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // Keine Rotation
    desktopState.scale = 1.0f;
    
    // Wenn VR-Modus aktiviert ist, OpenXR initialisieren
    if (currentMode == OperationMode::VR || currentMode == OperationMode::Hybrid) {
        XrInstanceCreateInfo createInfo{};
        createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
        createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
        strcpy(createInfo.applicationInfo.applicationName, "VR DAW");
        strcpy(createInfo.applicationInfo.engineName, "VR DAW Engine");
        
        if (xrCreateInstance(&createInfo, &xrInstance) != XR_SUCCESS) {
            return false;
        }
        
        // Erweiterungen aktivieren
        uint32_t extensionCount;
        xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
        std::vector<XrExtensionProperties> extensions(extensionCount);
        xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensions.data());
        
        // Hand-Tracking-Erweiterung aktivieren
        bool handTrackingSupported = false;
        for (const auto& extension : extensions) {
            if (strcmp(extension.extensionName, XR_EXT_HAND_TRACKING_EXTENSION_NAME) == 0) {
                handTrackingSupported = true;
                break;
            }
        }
        
        if (handTrackingSupported) {
            for (int i = 0; i < 2; ++i) {
                HandTracker tracker;
                XrHandTrackerCreateInfoEXT createInfo{};
                createInfo.type = XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT;
                createInfo.hand = i == 0 ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
                createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
                
                if (xrCreateHandTrackerEXT(xrInstance, &createInfo, &tracker.tracker) == XR_SUCCESS) {
                    handTrackers.push_back(std::move(tracker));
                }
            }
        }
    }
    
    return true;
}

void VRInterface::setOperationMode(OperationMode mode) {
    if (currentMode == mode) {
        return;
    }
    
    // Wenn von VR zu Desktop wechseln
    if (currentMode == OperationMode::VR && (mode == OperationMode::Desktop || mode == OperationMode::Hybrid)) {
        endSession();
    }
    
    // Wenn von Desktop zu VR wechseln
    if ((currentMode == OperationMode::Desktop || currentMode == OperationMode::Hybrid) && mode == OperationMode::VR) {
        startSession();
    }
    
    currentMode = mode;
}

VRInterface::OperationMode VRInterface::getOperationMode() const {
    return currentMode;
}

void VRInterface::setDesktopViewport(int width, int height) {
    desktopState.viewportWidth = width;
    desktopState.viewportHeight = height;
}

void VRInterface::setDesktopCamera(const glm::vec3& position, const glm::quat& rotation) {
    desktopState.cameraPosition = position;
    desktopState.cameraRotation = rotation;
}

void VRInterface::setDesktopScale(float scale) {
    desktopState.scale = scale;
}

void VRInterface::renderFrame() {
    switch (currentMode) {
        case OperationMode::VR:
            renderVRFrame();
            break;
        case OperationMode::Desktop:
            renderDesktopFrame();
            break;
        case OperationMode::Hybrid:
            renderVRFrame();
            renderDesktopFrame();
            break;
    }
}

void VRInterface::renderDesktopFrame() {
    // Desktop-View aktualisieren
    updateDesktopView();
    
    // UI-Elemente im Desktop-Modus rendern
    for (const auto& element : uiElements) {
        // Element-Position relativ zur Kamera berechnen
        glm::vec3 worldPos(element.position[0], element.position[1], element.position[2]);
        glm::quat worldRot(element.rotation[0], element.rotation[1], element.rotation[2], element.rotation[3]);
        glm::vec3 worldScale(element.scale[0], element.scale[1], element.scale[2]);
        
        // Kamera-Transformation anwenden
        glm::vec3 viewPos = glm::inverse(desktopState.cameraRotation) * (worldPos - desktopState.cameraPosition);
        glm::quat viewRot = glm::inverse(desktopState.cameraRotation) * worldRot;
        
        // Perspektivische Projektion
        float aspectRatio = static_cast<float>(desktopState.viewportWidth) / desktopState.viewportHeight;
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
        
        // Element rendern
        // Hier würde die Integration mit dem Rendering-System erfolgen
    }
}

void VRInterface::updateDesktopView() {
    // View-Matrix für Desktop-Modus berechnen
    glm::mat4 viewMatrix = glm::lookAt(
        desktopState.cameraPosition,
        desktopState.cameraPosition + glm::vec3(0.0f, 0.0f, -1.0f) * desktopState.cameraRotation,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    // View-Matrix aktualisieren
    std::copy(&viewMatrix[0][0], &viewMatrix[0][0] + 16, renderState.viewMatrix);
    
    // Projektions-Matrix für Desktop-Modus berechnen
    float aspectRatio = static_cast<float>(desktopState.viewportWidth) / desktopState.viewportHeight;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
    
    // Projektions-Matrix aktualisieren
    std::copy(&projectionMatrix[0][0], &projectionMatrix[0][0] + 16, renderState.projectionMatrix);
}

void VRInterface::shutdown() {
    // Hand-Tracker beenden
    for (auto& tracker : handTrackers) {
        xrDestroyHandTrackerEXT(tracker.tracker);
    }
    handTrackers.clear();
    
    // OpenXR-Instanz beenden
    if (xrInstance != XR_NULL_HANDLE) {
        xrDestroyInstance(xrInstance);
        xrInstance = XR_NULL_HANDLE;
    }
}

void VRInterface::startSession() {
    XrSessionCreateInfo createInfo{};
    createInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    createInfo.systemId = getSystemId();  // System-ID muss implementiert werden
    
    if (xrCreateSession(xrInstance, &createInfo, &xrSession) != XR_SUCCESS) {
        throw std::runtime_error("Fehler beim Erstellen der OpenXR-Session");
    }
    
    // Referenzraum erstellen
    XrReferenceSpaceCreateInfo spaceInfo{};
    spaceInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
    spaceInfo.poseInReferenceSpace = XrPosef{};
    
    if (xrCreateReferenceSpace(xrSession, &spaceInfo, &xrSpace) != XR_SUCCESS) {
        throw std::runtime_error("Fehler beim Erstellen des Referenzraums");
    }
    
    // Controller-Aktionen initialisieren
    initializeControllers();
}

void VRInterface::endSession() {
    if (xrSession != XR_NULL_HANDLE) {
        xrDestroySession(xrSession);
        xrSession = XR_NULL_HANDLE;
    }
    
    if (xrSpace != XR_NULL_HANDLE) {
        xrDestroySpace(xrSpace);
        xrSpace = XR_NULL_HANDLE;
    }
}

void VRInterface::handleControllerInput() {
    for (auto& controller : controllers) {
        XrActionStateGetInfo getInfo{};
        getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
        getInfo.action = controller.gripAction;
        
        XrActionStateFloat gripState{};
        gripState.type = XR_TYPE_ACTION_STATE_FLOAT;
        xrGetActionStateFloat(xrSession, &getInfo, &gripState);
        
        // Weitere Controller-Inputs hier verarbeiten
    }
}

void VRInterface::updateControllerState() {
    for (auto& controller : controllers) {
        XrSpaceLocation location{};
        location.type = XR_TYPE_SPACE_LOCATION;
        xrLocateSpace(controller.space, xrSpace, XrTime(), &location);
        
        // Controller-Position und -Rotation aktualisieren
    }
}

void VRInterface::createUIElement(const std::string& type, const std::string& id) {
    UIElement element;
    element.id = id;
    element.type = type;
    element.data = nullptr;  // UI-Element-Daten müssen implementiert werden
    
    // Standard-Position und -Skalierung
    element.position[0] = 0.0f;
    element.position[1] = 0.0f;
    element.position[2] = 0.0f;
    
    element.rotation[0] = 0.0f;
    element.rotation[1] = 0.0f;
    element.rotation[2] = 0.0f;
    element.rotation[3] = 1.0f;
    
    element.scale[0] = 1.0f;
    element.scale[1] = 1.0f;
    element.scale[2] = 1.0f;
    
    uiElements.push_back(std::move(element));
}

void VRInterface::updateUIElement(const std::string& id, const void* data) {
    auto it = std::find_if(uiElements.begin(), uiElements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != uiElements.end()) {
        it->data = const_cast<void*>(data);
    }
}

void VRInterface::removeUIElement(const std::string& id) {
    auto it = std::find_if(uiElements.begin(), uiElements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != uiElements.end()) {
        uiElements.erase(it);
    }
}

void VRInterface::setObjectPosition(const std::string& id, const float* position) {
    auto it = std::find_if(uiElements.begin(), uiElements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != uiElements.end()) {
        std::copy(position, position + 3, it->position);
    }
}

void VRInterface::setObjectRotation(const std::string& id, const float* rotation) {
    auto it = std::find_if(uiElements.begin(), uiElements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != uiElements.end()) {
        std::copy(rotation, rotation + 4, it->rotation);
    }
}

void VRInterface::setObjectScale(const std::string& id, const float* scale) {
    auto it = std::find_if(uiElements.begin(), uiElements.end(),
        [&id](const auto& element) {
            return element.id == id;
        });
    
    if (it != uiElements.end()) {
        std::copy(scale, scale + 3, it->scale);
    }
}

void VRInterface::enableHandTracking(bool enable) {
    if (enable) {
        for (auto& tracker : handTrackers) {
            XrHandJointsMotionRangeInfoEXT motionRangeInfo{};
            motionRangeInfo.type = XR_TYPE_HAND_JOINTS_MOTION_RANGE_INFO_EXT;
            motionRangeInfo.handJointsMotionRange = XR_HAND_JOINTS_MOTION_RANGE_UNOBSTRUCTED_EXT;
            
            xrUpdateHandJointLocationsEXT(tracker.tracker, &motionRangeInfo);
        }
    }
}

void VRInterface::updateHandPoses() {
    for (auto& tracker : handTrackers) {
        XrHandJointLocationsEXT locations{};
        locations.type = XR_TYPE_HAND_JOINT_LOCATIONS_EXT;
        locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
        locations.jointLocations = tracker.jointLocations;
        
        xrUpdateHandJointLocationsEXT(tracker.tracker, &locations);
    }
}

void VRInterface::enableRoomTracking(bool enable) {
    roomSpace.isTracking = enable;
}

void VRInterface::updateRoomSpace() {
    if (roomSpace.isTracking) {
        XrExtent2Df bounds;
        XrResult result = xrGetReferenceSpaceBoundsRect(xrSession, XR_REFERENCE_SPACE_TYPE_STAGE, &bounds);
        
        if (result == XR_SUCCESS) {
            roomSpace.bounds[0] = -bounds.width / 2.0f;   // minX
            roomSpace.bounds[1] = -bounds.height / 2.0f;  // minY
            roomSpace.bounds[2] = -1.0f;                  // minZ
            roomSpace.bounds[3] = bounds.width / 2.0f;    // maxX
            roomSpace.bounds[4] = bounds.height / 2.0f;   // maxY
            roomSpace.bounds[5] = 1.0f;                   // maxZ
        }
    }
}

void VRInterface::renderVRFrame() {
    // Frame-Rendering implementieren
    // Hier würde die Logik für das Rendering der VR-Szene implementiert
}

void VRInterface::updateViewMatrix(const float* viewMatrix) {
    std::copy(viewMatrix, viewMatrix + 16, renderState.viewMatrix);
}

void VRInterface::updateProjectionMatrix(const float* projectionMatrix) {
    std::copy(projectionMatrix, projectionMatrix + 16, renderState.projectionMatrix);
}

} // namespace VR_DAW 