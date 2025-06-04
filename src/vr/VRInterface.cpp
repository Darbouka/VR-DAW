#include "VRInterface.hpp"
#include <openxr/openxr_platform_defines.h>
#include <openxr/openxr_reflection.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb_image.h>
#include <glm/gtc/type_ptr.hpp>

namespace VR_DAW {

VRInterface::VRInterface() {
    initialize();
}

VRInterface::~VRInterface() {
    shutdown();
}

VRInterface& VRInterface::getInstance() {
    static VRInterface instance;
    return instance;
}

bool VRInterface::initialize() {
    // OpenVR initialisieren
    vr::EVRInitError error;
    vrSystem = vr::VR_Init(&error, vr::VRApplication_Scene);
    
    if (error != vr::VRInitError_None) {
        return false;
    }
    
    // Compositor initialisieren
    vrCompositor = vr::VRCompositor();
    if (!vrCompositor) {
        return false;
    }
    
    // Input-System initialisieren
    vrInput = vr::VRInput();
    if (!vrInput) {
        return false;
    }
    
    // Shader initialisieren
    initializeShaders();
    
    // Controller-Modelle laden
    leftControllerModel = loadControllerModel(vr::TrackedControllerRole_LeftHand);
    rightControllerModel = loadControllerModel(vr::TrackedControllerRole_RightHand);
    
    // Adaptive Renderer initialisieren
    adaptiveRenderer = std::make_unique<AdaptiveRenderer>();
    adaptiveRenderer->initialize();
    
    // Haptic Controller nur initialisieren, wenn verfügbar
    if (currentVROptimizations.hapticFeedback) {
        hapticController = std::make_unique<HapticController>();
        hapticController->initialize();
    }
    
    // Hand Tracker initialisieren
    handTracker = std::make_unique<HandTracker>();
    handTracker->initialize();
    
    // Spatial Audio Renderer initialisieren
    spatialAudioRenderer = std::make_unique<SpatialAudioRenderer>();
    spatialAudioRenderer->initialize();
    
    return true;
}

void VRInterface::shutdown() {
    if (vrSystem) {
        vr::VR_Shutdown();
        vrSystem = nullptr;
    }
    
    adaptiveRenderer.reset();
    hapticController.reset();
    handTracker.reset();
    spatialAudioRenderer.reset();
}

void VRInterface::render() {
    if (!vrSystem || !vrCompositor) return;
    
    // Aktualisiere Tracking
    updateTracking();
    
    // Rendere für beide Augen
    for (vr::EVREye eye : {vr::Eye_Left, vr::Eye_Right}) {
        // Hole Eye-Matrix
        glm::mat4 eyeMatrix = glm::make_mat4(vrSystem->GetEyeToHeadTransform(eye).m);
        glm::mat4 projectionMatrix = glm::make_mat4(vrSystem->GetProjectionMatrix(eye, 0.1f, 1000.0f).m);
        
        // Setze Viewport
        int width, height;
        vrSystem->GetRecommendedRenderTargetSize(&width, &height);
        glViewport(0, 0, width, height);
        
        // Rendere Szene
        renderScene(eyeMatrix, projectionMatrix);
        
        // Submit an Compositor
        vr::Texture_t texture = { (void*)(uintptr_t)renderTargets[eye].colorTexture, 
                                 vr::TextureType_OpenGL, 
                                 vr::ColorSpace_Gamma };
        vrCompositor->Submit(eye, &texture);
    }
    
    // Present
    vrCompositor->PostPresentHandoff();
}

void VRInterface::updateTracking() {
    if (!vrSystem) return;
    
    // Aktualisiere Device-Posen
    vr::VRCompositor()->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    
    // Verarbeite Controller-Input
    processControllerInput();
    
    // Aktualisiere Hand-Tracking
    if (handTracker) {
        handTracker->update(devicePoses);
    }
    
    // Aktualisiere Haptic Feedback
    if (hapticController) {
        hapticController->update();
    }
}

void VRInterface::processControllerInput() {
    if (!vrInput) return;
    
    // Verarbeite Controller-Events
    vr::VREvent_t event;
    while (vrSystem->PollNextEvent(&event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_ButtonPress:
                handleButtonPress(event);
                break;
            case vr::VREvent_ButtonUnpress:
                handleButtonUnpress(event);
                break;
            case vr::VREvent_TouchPadMove:
                handleTouchPadMove(event);
                break;
        }
    }
}

void VRInterface::renderScene(const glm::mat4& eyeMatrix, const glm::mat4& projectionMatrix) {
    // Aktualisiere Shader-Uniforms
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(eyeMatrix));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    
    // Rendere Raum
    renderRoom();
    
    // Rendere Fenster
    renderWindows();
    
    // Rendere Controller
    renderControllers();
    
    // Rendere UI
    renderUI();
}

void VRInterface::initializeShaders() {
    // Vertex Shader
    const char* vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aTexCoord;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    // Fragment Shader
    const char* fragmentSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        
        uniform vec3 diffuseColor;
        uniform vec3 specularColor;
        uniform float shininess;
        uniform float transparency;
        
        out vec4 FragColor;
        
        void main() {
            vec3 normal = normalize(Normal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            vec3 viewDir = normalize(-FragPos);
            
            // Diffuse
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * diffuseColor;
            
            // Specular
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            vec3 specular = spec * specularColor;
            
            // Ambient
            vec3 ambient = 0.1 * diffuseColor;
            
            vec3 result = ambient + diffuse + specular;
            FragColor = vec4(result, transparency);
        }
    )";
    
    // Kompiliere Shader
    shaderProgram = compileShaderProgram(vertexSource, fragmentSource);
    
    // Hole Uniform-Locations
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
}

GLuint VRInterface::compileShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Shader Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
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

void VRInterface::createRoom(const glm::vec3& dimensions) {
    currentRoom.dimensions = dimensions;
    
    // Wände erstellen
    // Vordere Wand
    addWall(glm::vec3(0.0f, dimensions.y/2, dimensions.z/2), 
            glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
    
    // Hintere Wand
    addWall(glm::vec3(0.0f, dimensions.y/2, -dimensions.z/2), 
            glm::quat(glm::vec3(0.0f, glm::pi<float>(), 0.0f)));
    
    // Linke Wand
    addWall(glm::vec3(-dimensions.x/2, dimensions.y/2, 0.0f), 
            glm::quat(glm::vec3(0.0f, glm::pi<float>()/2, 0.0f)));
    
    // Rechte Wand
    addWall(glm::vec3(dimensions.x/2, dimensions.y/2, 0.0f), 
            glm::quat(glm::vec3(0.0f, -glm::pi<float>()/2, 0.0f)));
    
    // Decke
    addWall(glm::vec3(0.0f, dimensions.y, 0.0f), 
            glm::quat(glm::vec3(glm::pi<float>()/2, 0.0f, 0.0f)));
    
    // Boden
    addWall(glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::quat(glm::vec3(-glm::pi<float>()/2, 0.0f, 0.0f)));
}

void VRInterface::addWall(const glm::vec3& position, const glm::quat& rotation) {
    currentRoom.wallPositions.push_back(position);
    currentRoom.wallRotations.push_back(rotation);
}

void VRInterface::createWindow(const std::string& id, const std::string& title,
                             const glm::vec3& position, const glm::vec2& size) {
    Window window;
    window.id = id;
    window.title = title;
    window.position = position;
    window.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    window.size = size;
    window.isResizable = true;
    window.isMovable = true;
    
    windows.push_back(window);
    currentRoom.windowPositions[id] = position;
    currentRoom.windowSizes[id] = size;
}

void VRInterface::moveWindow(const std::string& id, const glm::vec3& newPosition) {
    for (auto& window : windows) {
        if (window.id == id) {
            window.position = newPosition;
            currentRoom.windowPositions[id] = newPosition;
            break;
        }
    }
}

void VRInterface::resizeWindow(const std::string& id, const glm::vec2& newSize) {
    for (auto& window : windows) {
        if (window.id == id) {
            window.size = newSize;
            currentRoom.windowSizes[id] = newSize;
            break;
        }
    }
}

void VRInterface::attachWindowToWall(const std::string& id, size_t wallIndex) {
    if (wallIndex >= currentRoom.wallPositions.size()) return;
    
    for (auto& window : windows) {
        if (window.id == id) {
            // Fenster an Wand ausrichten
            window.position = currentRoom.wallPositions[wallIndex];
            window.rotation = currentRoom.wallRotations[wallIndex];
            currentRoom.windowPositions[id] = window.position;
            break;
        }
    }
}

void VRInterface::startMotionTracking() {
    isTrackingActive = true;
    
    // Motion-Tracking initialisieren
    vr::VRInput()->SetActionManifestPath("motion_manifest.json");
    
    // Tracking-Aktionen definieren
    vr::VRActionSetHandle_t actionSet;
    vr::VRInput()->GetActionSetHandle("/actions/motion", &actionSet);
    
    // Tracking starten
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
}

void VRInterface::updateMotionData() {
    if (!isTrackingActive) return;
    
    // Controller-Positionen aktualisieren
    for (auto& [role, state] : controllerStates) {
        vr::TrackedDevicePose_t pose;
        vr::VRSystem()->GetControllerStateWithPose(
            vr::TrackingUniverseStanding,
            vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(role),
            &state.buttonStates[0],
            sizeof(state.buttonStates),
            &pose
        );
        
        if (pose.bPoseIsValid) {
            state.position = glm::vec3(
                pose.mDeviceToAbsoluteTracking.m[0][3],
                pose.mDeviceToAbsoluteTracking.m[1][3],
                pose.mDeviceToAbsoluteTracking.m[2][3]
            );
            
            state.rotation = glm::quat_cast(glm::mat4(
                pose.mDeviceToAbsoluteTracking.m[0][0], pose.mDeviceToAbsoluteTracking.m[0][1], pose.mDeviceToAbsoluteTracking.m[0][2], pose.mDeviceToAbsoluteTracking.m[0][3],
                pose.mDeviceToAbsoluteTracking.m[1][0], pose.mDeviceToAbsoluteTracking.m[1][1], pose.mDeviceToAbsoluteTracking.m[1][2], pose.mDeviceToAbsoluteTracking.m[1][3],
                pose.mDeviceToAbsoluteTracking.m[2][0], pose.mDeviceToAbsoluteTracking.m[2][1], pose.mDeviceToAbsoluteTracking.m[2][2], pose.mDeviceToAbsoluteTracking.m[2][3],
                pose.mDeviceToAbsoluteTracking.m[3][0], pose.mDeviceToAbsoluteTracking.m[3][1], pose.mDeviceToAbsoluteTracking.m[3][2], pose.mDeviceToAbsoluteTracking.m[3][3]
            ));
        }
    }
    
    // Bewegungsdaten aktualisieren
    currentMotion.position = (controllerStates[vr::TrackedControllerRole_LeftHand].position + 
                            controllerStates[vr::TrackedControllerRole_RightHand].position) * 0.5f;
    
    // Geschwindigkeit und Beschleunigung berechnen
    static glm::vec3 lastPosition = currentMotion.position;
    static float lastUpdateTime = 0.0f;
    float currentTime = static_cast<float>(glm::clock::now().time_since_epoch().count()) / 1e9f;
    
    if (lastUpdateTime > 0.0f) {
        float deltaTime = currentTime - lastUpdateTime;
        currentMotion.velocity = (currentMotion.position - lastPosition) / deltaTime;
        currentMotion.acceleration = (currentMotion.velocity - currentMotion.velocity) / deltaTime;
    }
    
    lastPosition = currentMotion.position;
    lastUpdateTime = currentTime;
}

void VRInterface::mapMotionToAudioParameters(const MotionData& motion) {
    // Bewegungsdaten auf Audio-Parameter mappen
    float intensity = glm::length(motion.velocity);
    float direction = glm::atan(motion.velocity.x, motion.velocity.z);
    float height = motion.position.y;
    
    // Parameter an Audio-Engine übergeben
    AudioParameters params;
    
    // Geschwindigkeitsbasierte Parameter
    params.volume = glm::clamp(intensity * 2.0f, 0.0f, 1.0f);
    params.pitch = glm::clamp(1.0f + (intensity * 0.5f), 0.5f, 2.0f);
    params.reverb = glm::clamp(intensity * 1.5f, 0.0f, 1.0f);
    
    // Richtungsbasierte Parameter
    params.pan = glm::clamp(direction / glm::pi<float>(), -1.0f, 1.0f);
    params.stereoWidth = glm::clamp(std::abs(direction) / glm::pi<float>(), 0.0f, 1.0f);
    
    // Höhenbasierte Parameter
    params.filterCutoff = glm::clamp(height / 2.0f, 20.0f, 20000.0f);
    params.filterResonance = glm::clamp(1.0f - (height / 3.0f), 0.0f, 1.0f);
    
    // Beschleunigungsbasierte Parameter
    float acceleration = glm::length(motion.acceleration);
    params.distortion = glm::clamp(acceleration * 0.5f, 0.0f, 1.0f);
    params.compression = glm::clamp(acceleration * 0.3f, 0.0f, 1.0f);
    
    // Rotationseffekte
    glm::vec3 eulerAngles = glm::eulerAngles(motion.rotation);
    params.phaserRate = glm::clamp(std::abs(eulerAngles.x) / glm::pi<float>(), 0.0f, 1.0f);
    params.flangerRate = glm::clamp(std::abs(eulerAngles.y) / glm::pi<float>(), 0.0f, 1.0f);
    params.chorusRate = glm::clamp(std::abs(eulerAngles.z) / glm::pi<float>(), 0.0f, 1.0f);
    
    // Parameter an Audio-Engine übergeben
    audioEngine->updateParameters(params);
}

void VRInterface::renderRoom() {
    // Raum rendern
    for (size_t i = 0; i < currentRoom.wallPositions.size(); ++i) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), currentRoom.wallPositions[i]);
        model *= glm::toMat4(currentRoom.wallRotations[i]);
        
        // Wand-Material und Textur laden
        Material wallMaterial;
        wallMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
        wallMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
        wallMaterial.shininess = 32.0f;
        
        // Wand-Mesh erstellen
        Mesh wallMesh;
        wallMesh.vertices = {
            // Vorderseite
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
        };
        
        wallMesh.indices = {0, 1, 2, 2, 3, 0};
        
        // Wand rendern
        renderMesh(wallMesh, model, wallMaterial);
    }
}

void VRInterface::renderWindows() {
    // Fenster rendern
    for (const auto& window : windows) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), window.position);
        model *= glm::toMat4(window.rotation);
        model = glm::scale(model, glm::vec3(window.size.x, window.size.y, 1.0f));
        
        // Fenster-Material
        Material windowMaterial;
        windowMaterial.diffuseColor = glm::vec3(0.9f, 0.9f, 0.9f);
        windowMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
        windowMaterial.shininess = 64.0f;
        windowMaterial.transparency = 0.8f;
        
        // Fenster-Mesh
        Mesh windowMesh;
        windowMesh.vertices = {
            // Vorderseite
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
        };
        
        windowMesh.indices = {0, 1, 2, 2, 3, 0};
        
        // Fenster rendern
        renderMesh(windowMesh, model, windowMaterial);
        
        // Fenster-Inhalt rendern
        renderWindowContent(window);
    }
}

void VRInterface::renderMotionData() {
    if (!isTrackingActive) return;
    
    // Controller rendern
    for (const auto& [role, state] : controllerStates) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), state.position);
        model *= glm::toMat4(state.rotation);
        
        // Controller-Material
        Material controllerMaterial;
        controllerMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
        controllerMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
        controllerMaterial.shininess = 32.0f;
        
        // Controller-Mesh laden
        Mesh controllerMesh = loadControllerModel(role);
        
        // Controller rendern
        renderMesh(controllerMesh, model, controllerMaterial);
    }
    
    // Bewegungsvektoren rendern
    glm::mat4 motionModel = glm::translate(glm::mat4(1.0f), currentMotion.position);
    
    // Geschwindigkeitsvektor
    glm::vec3 velocityEnd = currentMotion.position + currentMotion.velocity;
    renderVector(currentMotion.position, velocityEnd, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Beschleunigungsvektor
    glm::vec3 accelerationEnd = currentMotion.position + currentMotion.acceleration;
    renderVector(currentMotion.position, accelerationEnd, glm::vec3(1.0f, 0.0f, 0.0f));
}

void VRInterface::renderMesh(const Mesh& mesh, const glm::mat4& model, const Material& material) {
    // Shader-Programm aktivieren
    glUseProgram(shaderProgram);
    
    // Uniforms setzen
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderState.viewMatrix));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderState.projectionMatrix));
    
    // Material-Eigenschaften setzen
    glUniform3fv(diffuseColorLoc, 1, glm::value_ptr(material.diffuseColor));
    glUniform3fv(specularColorLoc, 1, glm::value_ptr(material.specularColor));
    glUniform1f(shininessLoc, material.shininess);
    glUniform1f(transparencyLoc, material.transparency);
    
    // Vertex Array Object binden
    glBindVertexArray(mesh.vao);
    
    // Zeichnen
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    
    // Aufräumen
    glBindVertexArray(0);
    glUseProgram(0);
}

void VRInterface::renderVector(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    // Vektor-Material
    Material vectorMaterial;
    vectorMaterial.diffuseColor = color;
    vectorMaterial.specularColor = glm::vec3(1.0f);
    vectorMaterial.shininess = 32.0f;
    
    // Vektor-Mesh erstellen
    Mesh vectorMesh;
    vectorMesh.vertices = {
        {{start.x, start.y, start.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{end.x, end.y, end.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
    };
    
    vectorMesh.indices = {0, 1};
    
    // Vektor rendern
    renderMesh(vectorMesh, glm::mat4(1.0f), vectorMaterial);
}

void VRInterface::renderWindowContent(const Window& window) {
    // Fenster-Inhalt rendern
    if (window.content.empty()) return;
    
    // Textur erstellen
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Textur-Daten laden
    int width, height, channels;
    unsigned char* data = stbi_load(window.content.c_str(), &width, &height, &channels, 4);
    
    if (data) {
        // Textur-Daten hochladen
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Textur-Parameter setzen
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Textur rendern
        glUseProgram(textureShaderProgram);
        
        // Uniforms setzen
        glUniform1i(textureLoc, 0);
        glUniformMatrix4fv(textureModelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glUniformMatrix4fv(textureViewLoc, 1, GL_FALSE, glm::value_ptr(renderState.viewMatrix));
        glUniformMatrix4fv(textureProjectionLoc, 1, GL_FALSE, glm::value_ptr(renderState.projectionMatrix));
        
        // Textur-Mesh erstellen
        Mesh textureMesh;
        textureMesh.vertices = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
        };
        
        textureMesh.indices = {0, 1, 2, 2, 3, 0};
        
        // Vertex Array Object erstellen
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        // Vertex Buffer Object erstellen
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, textureMesh.vertices.size() * sizeof(Vertex), textureMesh.vertices.data(), GL_STATIC_DRAW);
        
        // Element Buffer Object erstellen
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, textureMesh.indices.size() * sizeof(unsigned int), textureMesh.indices.data(), GL_STATIC_DRAW);
        
        // Vertex-Attribute setzen
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);
        
        // Textur binden
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // Zeichnen
        glDrawElements(GL_TRIANGLES, textureMesh.indices.size(), GL_UNSIGNED_INT, 0);
        
        // Aufräumen
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &texture);
        glUseProgram(0);
        
        // Speicher freigeben
        stbi_image_free(data);
    }
}

Mesh VRInterface::loadControllerModel(vr::ETrackedControllerRole role) {
    // Controller-Modell basierend auf Rolle laden
    Mesh controllerMesh;
    
    // Standard-Controller-Mesh
    controllerMesh.vertices = {
        // Controller-Körper
        {{-0.05f, -0.05f, -0.1f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.05f, -0.05f, -0.1f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.05f,  0.05f, -0.1f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.05f,  0.05f, -0.1f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    controllerMesh.indices = {0, 1, 2, 2, 3, 0};
    
    return controllerMesh;
}

} // namespace VR_DAW 