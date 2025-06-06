#include "VRSystem.hpp"
#include <chrono>

namespace VR_DAW {

VRSystem::VRSystem()
    : initialized(false)
    , running(false)
    , debugEnabled(false)
    , renderWidth(1920)
    , renderHeight(1080)
    , refreshRate(90.0f)
    , ipd(0.064f)
    , worldScale(1.0f)
    , qualityLevel(1)
    , frameTime(0.0f)
    , frameRate(0.0f)
    , latency(0.0f)
    , droppedFrames(0)
{
}

VRSystem::~VRSystem() {
    shutdown();
}

bool VRSystem::initialize() {
    if (initialized) return true;

    initializeComponents();
    initialized = true;
    running = true;
    return true;
}

void VRSystem::shutdown() {
    if (!initialized) return;

    running = false;
    shutdownComponents();
    initialized = false;
}

void VRSystem::update(float deltaTime) {
    if (!initialized || !running) return;

    updateComponents(deltaTime);
    updatePerformanceMetrics();
}

bool VRSystem::isInitialized() const {
    return initialized;
}

bool VRSystem::isRunning() const {
    return running;
}

std::string VRSystem::getStatus() const {
    if (!initialized) return "Nicht initialisiert";
    if (!running) return "Gestoppt";
    return "Läuft";
}

VRController& VRSystem::getController() {
    return *controller;
}

VRInterface& VRSystem::getInterface() {
    return *interface;
}

VRScene& VRSystem::getScene() {
    return *scene;
}

VRRenderer& VRSystem::getRenderer() {
    return *renderer;
}

VRInput& VRSystem::getInput() {
    return *input;
}

VRAudio& VRSystem::getAudio() {
    return *audio;
}

VRPhysics& VRSystem::getPhysics() {
    return *physics;
}

VRNetwork& VRSystem::getNetwork() {
    return *network;
}

VRUI& VRSystem::getUI() {
    return *ui;
}

void VRSystem::setRenderResolution(int width, int height) {
    renderWidth = width;
    renderHeight = height;
    
    if (renderer) {
        renderer->setViewport(0, 0, width, height);
    }
}

void VRSystem::setRefreshRate(float rate) {
    refreshRate = rate;
}

void VRSystem::setIPD(float newIpd) {
    ipd = newIpd;
}

void VRSystem::setWorldScale(float scale) {
    worldScale = scale;
}

void VRSystem::setQualitySettings(int quality) {
    qualityLevel = quality;
    
    if (renderer) {
        renderer->setRenderQuality(quality);
    }
}

float VRSystem::getFrameTime() const {
    return frameTime;
}

float VRSystem::getFrameRate() const {
    return frameRate;
}

float VRSystem::getLatency() const {
    return latency;
}

int VRSystem::getDroppedFrames() const {
    return droppedFrames;
}

void VRSystem::enableDebugMode(bool enable) {
    debugEnabled = enable;
    
    if (renderer) {
        renderer->enableDebugRendering(enable);
    }
}

void VRSystem::showDebugInfo() {
    if (!initialized || !debugEnabled) return;

    renderDebugInfo();
}

void VRSystem::toggleWireframe(bool enable) {
    if (renderer) {
        renderer->renderWireframe(enable);
    }
}

void VRSystem::initializeComponents() {
    controller = std::make_unique<VRController>();
    interface = std::make_unique<VRInterface>();
    scene = std::make_unique<VRScene>();
    renderer = std::make_unique<VRRenderer>();
    input = std::make_unique<VRInput>();
    audio = std::make_unique<VRAudio>();
    physics = std::make_unique<VRPhysics>();
    network = std::make_unique<VRNetwork>();
    ui = std::make_unique<VRUI>();

    controller->initialize();
    interface->initialize();
    scene->initialize();
    renderer->initialize();
    input->initialize();
    audio->initialize();
    physics->initialize();
    network->initialize();
    ui->initialize();
}

void VRSystem::shutdownComponents() {
    if (ui) ui->shutdown();
    if (network) network->shutdown();
    if (physics) physics->shutdown();
    if (audio) audio->shutdown();
    if (input) input->shutdown();
    if (renderer) renderer->shutdown();
    if (scene) scene->shutdown();
    if (interface) interface->shutdown();
    if (controller) controller->shutdown();

    ui.reset();
    network.reset();
    physics.reset();
    audio.reset();
    input.reset();
    renderer.reset();
    scene.reset();
    interface.reset();
    controller.reset();
}

void VRSystem::updateComponents(float deltaTime) {
    if (controller) controller->update();
    if (interface) interface->update();
    if (scene) scene->update();
    if (renderer) renderer->update();
    if (input) input->update();
    if (audio) audio->update();
    if (physics) physics->update(deltaTime);
    if (network) network->update();
    if (ui) ui->update();
}

void VRSystem::updatePerformanceMetrics() {
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    
    frameTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
    frameRate = 1.0f / frameTime;
    
    lastFrameTime = currentFrameTime;
}

void VRSystem::renderDebugInfo() {
    if (!initialized || !debugEnabled) return;

    // Hier würde das Rendering der Debug-Informationen erfolgen
    // z.B. mit OpenGL, Vulkan oder anderen Grafik-APIs
}

} // namespace VR_DAW 