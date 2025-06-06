#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VRScene.hpp"

namespace VR_DAW {

class VRRenderer {
public:
    struct RenderConfig {
        bool useVulkan;
        int width;
        int height;
        float renderScale;
        int msaaSamples;
    };

    struct ShaderProgram {
        unsigned int id;
        std::string vertexPath;
        std::string fragmentPath;
    };

    struct Mesh {
        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
        size_t indexCount;
    };

    struct Texture {
        unsigned int id;
        int width;
        int height;
        int channels;
    };

    static VRRenderer& getInstance();

    VRRenderer();
    ~VRRenderer();

    // Initialisierung und Shutdown
    bool initialize(const RenderConfig& config);
    void shutdown();
    void render();

    // Rendering-Status
    bool isInitialized() const;
    bool isRendering() const;
    std::string getStatus() const;

    // Rendering-Konfiguration
    void setRenderResolution(int width, int height);
    void setRenderScale(float scale);
    void setRenderQuality(int quality);
    void setMSAASamples(int samples);
    void setRefreshRate(float rate);
    void setIPD(float ipd);
    void setWorldScale(float scale);

    // Rendering-Pipeline
    void beginFrame();
    void endFrame();
    void submitFrame();
    void renderScene(const VRScene& scene);
    void renderUI(const VRInterface& interface);

    // Rendering-Funktionen
    void renderUI(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void renderSynthesizer(const glm::mat4& modelMatrix);
    void renderWaveform(const std::vector<float>& data, const glm::mat4& modelMatrix);

    // Shader-Management
    ShaderProgram createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
    void useShaderProgram(const ShaderProgram& program);
    void deleteShaderProgram(ShaderProgram& program);

    // Mesh-Management
    Mesh createMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    void renderMesh(const Mesh& mesh, const glm::mat4& modelMatrix);
    void deleteMesh(Mesh& mesh);

    // Texture-Management
    Texture loadTexture(const std::string& path);
    void bindTexture(const Texture& texture, unsigned int unit = 0);
    void deleteTexture(Texture& texture);

    // Post-Processing
    struct PostProcessEffect {
        std::string name;
        bool enabled;
        float intensity;
        std::vector<float> parameters;
    };

    void addPostProcessEffect(const std::string& name, const PostProcessEffect& effect);
    void removePostProcessEffect(const std::string& name);
    void updatePostProcessEffect(const std::string& name, const PostProcessEffect& effect);
    void enablePostProcessEffect(const std::string& name, bool enable);

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void toggleWireframe();
    void renderDebugShapes();
    void renderDebugText(const std::string& text, const glm::vec2& position);
    void renderDebugInfo(const std::string& text, const glm::vec3& position);
    void renderElementBounds(const glm::vec3& position, const glm::vec3& size);

    // Performance-Metriken
    float getFrameTime() const;
    float getFrameRate() const;
    float getLatency() const;
    int getDroppedFrames() const;

    // Uniform-Setter
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const glm::vec2& value);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::vec4& value);
    void setUniform(const std::string& name, const glm::mat4& value);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool rendering;
    bool debugEnabled;
    bool wireframeMode;

    // Rendering-Parameter
    int renderWidth;
    int renderHeight;
    float renderScale;
    int renderQuality;
    int msaaSamples;
    float refreshRate;
    float ipd;
    float worldScale;

    // Performance-Metriken
    float frameTime;
    float frameRate;
    float latency;
    int droppedFrames;

    // Shader und Material
    std::map<std::string, ShaderProgram> shaderPrograms;
    std::map<std::string, PostProcessEffect> postProcessEffects;

    // OpenGL/Vulkan-spezifische Implementierung
    #ifdef USE_OPENGL
    void initializeOpenGL();
    #endif
    
    #ifdef USE_VULKAN
    void initializeVulkan();
    void initializeRendering();
    void shutdownRendering();
    void updatePerformanceMetrics();
    void renderPostProcess();
    void renderDebugInfo();
    #endif
};

} // namespace VR_DAW 