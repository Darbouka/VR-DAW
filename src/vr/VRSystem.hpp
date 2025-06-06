
#pragma once

#include <memory>
#include <string>
#include "VRController.hpp"
#include "VRInterface.hpp"
#include "VRScene.hpp"
#include "VRRenderer.hpp"
#include "VRInput.hpp"
#include "VRAudio.hpp"
#include "VRPhysics.hpp"
#include "VRNetwork.hpp"
#include "VRUI.hpp"

namespace VR_DAW {

class VRSystem {
public:
    VRSystem();
    ~VRSystem();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update(float deltaTime);

    // System-Status
    bool isInitialized() const;
    bool isRunning() const;
    std::string getStatus() const;

    // Komponenten-Zugriff
    VRController& getController();
    VRInterface& getInterface();
    VRScene& getScene();
    VRRenderer& getRenderer();
    VRInput& getInput();
    VRAudio& getAudio();
    VRPhysics& getPhysics();
    VRNetwork& getNetwork();
    VRUI& getUI();

    // Konfiguration
    void setRenderResolution(int width, int height);
    void setRefreshRate(float rate);
    void setIPD(float ipd);
    void setWorldScale(float scale);
    void setQualitySettings(int quality);

    // Performance-Metriken
    float getFrameTime() const;
    float getFrameRate() const;
    float getLatency() const;
    int getDroppedFrames() const;

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void toggleWireframe(bool enable);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool running;
    bool debugEnabled;
    
    // Komponenten
    std::unique_ptr<VRController> controller;
    std::unique_ptr<VRInterface> interface;
    std::unique_ptr<VRScene> scene;
    std::unique_ptr<VRRenderer> renderer;
    std::unique_ptr<VRInput> input;
    std::unique_ptr<VRAudio> audio;
    std::unique_ptr<VRPhysics> physics;
    std::unique_ptr<VRNetwork> network;
    std::unique_ptr<VRUI> ui;
    
    // Konfiguration
    int renderWidth;
    int renderHeight;
    float refreshRate;
    float ipd;
    float worldScale;
    int qualityLevel;
    
    // Performance-Metriken
    float frameTime;
    float frameRate;
    float latency;
    int droppedFrames;
    
    void initializeComponents();
    void shutdownComponents();
    void updateComponents(float deltaTime);
    void updatePerformanceMetrics();
    void renderDebugInfo();
};

} // namespace VR_DAW 