#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "VRInterface.hpp"

namespace VR_DAW {

class VRScene {
public:
    VRScene();
    ~VRScene();

    // Initialisierung und Konfiguration
    bool initialize();
    void shutdown();
    void update();

    // Szenen-Management
    void loadScene(const std::string& sceneName);
    void unloadScene();
    void saveScene(const std::string& sceneName);

    // Objekt-Management
    void addObject(const std::string& name, const glm::vec3& position, const glm::quat& rotation);
    void removeObject(const std::string& name);
    void updateObject(const std::string& name, const glm::vec3& position, const glm::quat& rotation);
    void setObjectScale(const std::string& name, const glm::vec3& scale);

    // Interaktion
    void handleInteraction(const std::string& objectName, const std::string& interactionType);
    void processCollisions();
    void updatePhysics();

    // Rendering
    void render();
    void updateLights();
    void updateCamera();

    // Audio
    void setAudioSource(const std::string& objectName, const std::string& audioFile);
    void updateAudio();
    void setAudioListener(const glm::vec3& position, const glm::quat& rotation);

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void toggleWireframe();

    // Szene-Status
    bool isInitialized() const;
    bool isLoaded() const;
    std::string getStatus() const;

    // Szene-Objekte
    struct SceneObject {
        std::string id;
        std::string type;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        bool visible;
        std::string model;
        std::string material;
    };

    // Objekt-Management
    void createObject(const std::string& id, const std::string& type);
    void destroyObject(const std::string& id);
    void updateObject(const std::string& id, const SceneObject& object);
    SceneObject getObject(const std::string& id) const;

    // Transformation
    void setObjectPosition(const std::string& id, const glm::vec3& position);
    void setObjectRotation(const std::string& id, const glm::quat& rotation);
    void setObjectScale(const std::string& id, const glm::vec3& scale);
    void setObjectTransform(const std::string& id, const glm::mat4& transform);

    // Sichtbarkeit
    void setObjectVisible(const std::string& id, bool visible);
    bool isObjectVisible(const std::string& id) const;

    // Modell und Material
    void setObjectModel(const std::string& id, const std::string& model);
    void setObjectMaterial(const std::string& id, const std::string& material);

    // Beleuchtung
    struct Light {
        std::string id;
        std::string type;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float range;
        float spotAngle;
    };

    void addLight(const std::string& id, const Light& light);
    void removeLight(const std::string& id);
    void updateLight(const std::string& id, const Light& light);
    Light getLight(const std::string& id) const;

    // Kamera
    struct Camera {
        glm::vec3 position;
        glm::quat rotation;
        float fov;
        float nearPlane;
        float farPlane;
    };

    void setCamera(const Camera& camera);
    Camera getCamera() const;

    // Umgebung
    void setEnvironmentMap(const std::string& path);
    void setAmbientLight(const glm::vec3& color, float intensity);
    void setFog(const glm::vec3& color, float density, float start, float end);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool debugEnabled;
    
    // Szene-Status
    bool loaded;
    
    // Szene-Objekte
    std::map<std::string, SceneObject> objects;
    std::map<std::string, Light> lights;
    Camera camera;
    
    // Umgebung
    std::string environmentMap;
    glm::vec3 ambientColor;
    float ambientIntensity;
    glm::vec3 fogColor;
    float fogDensity;
    float fogStart;
    float fogEnd;
    
    void updateTransforms();
    void updateLights();
    void updateCamera();
    void renderDebugInfo();
};

} // namespace VR_DAW
