#include "VRRenderer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace VR_DAW {

struct VRRenderer::Impl {
    RenderConfig config;
    GLFWwindow* window;
    ShaderProgram currentShader;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    bool isInitialized;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    glm::vec4 clearColor;
    int viewportX, viewportY, viewportWidth, viewportHeight;
};

VRRenderer& VRRenderer::getInstance() {
    static VRRenderer instance;
    return instance;
}

bool VRRenderer::initialize(const RenderConfig& config) {
    pImpl = std::make_unique<Impl>();
    pImpl->config = config;
    pImpl->isInitialized = false;
    pImpl->clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    pImpl->viewportX = 0;
    pImpl->viewportY = 0;
    pImpl->viewportWidth = config.width;
    pImpl->viewportHeight = config.height;

    if (!glfwInit()) {
        std::cerr << "Fehler bei der GLFW-Initialisierung" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    pImpl->window = glfwCreateWindow(config.width, config.height, "VR DAW", nullptr, nullptr);
    if (!pImpl->window) {
        std::cerr << "Fehler beim Erstellen des GLFW-Fensters" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(pImpl->window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fehler beim Laden von GLAD" << std::endl;
        return false;
    }

    // OpenGL-Einstellungen
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    // Standard-Shader erstellen
    std::string vertexShader = R"(
        #version 410 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec3 aNormal;

        out vec2 TexCoord;
        out vec3 Normal;
        out vec3 FragPos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aTexCoord;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    std::string fragmentShader = R"(
        #version 410 core
        in vec2 TexCoord;
        in vec3 Normal;
        in vec3 FragPos;

        out vec4 FragColor;

        uniform sampler2D texture1;
        uniform vec3 lightPos;
        uniform vec3 viewPos;
        uniform vec3 lightColor;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = spec * lightColor;
            
            vec3 ambient = 0.1 * lightColor;
            
            vec4 texColor = texture(texture1, TexCoord);
            vec3 result = (ambient + diffuse + specular) * texColor.rgb;
            FragColor = vec4(result, texColor.a);
        }
    )";

    pImpl->currentShader = createShaderProgram(vertexShader, fragmentShader);
    useShaderProgram(pImpl->currentShader);

    pImpl->isInitialized = true;
    return true;
}

void VRRenderer::shutdown() {
    if (!pImpl->isInitialized) return;

    // Ressourcen freigeben
    for (auto& mesh : pImpl->meshes) {
        deleteMesh(mesh);
    }
    for (auto& texture : pImpl->textures) {
        deleteTexture(texture);
    }
    deleteShaderProgram(pImpl->currentShader);

    glfwDestroyWindow(pImpl->window);
    glfwTerminate();
    pImpl->isInitialized = false;
}

ShaderProgram VRRenderer::createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    ShaderProgram program;
    
    // Vertex-Shader kompilieren
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vShaderCode = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    // Fragment-Shader kompilieren
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fShaderCode = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);

    // Shader-Programm erstellen
    program.id = glCreateProgram();
    glAttachShader(program.id, vertexShader);
    glAttachShader(program.id, fragmentShader);
    glLinkProgram(program.id);

    // Shader löschen
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void VRRenderer::useShaderProgram(const ShaderProgram& program) {
    glUseProgram(program.id);
    pImpl->currentShader = program;
}

void VRRenderer::deleteShaderProgram(ShaderProgram& program) {
    glDeleteProgram(program.id);
    program.id = 0;
}

Mesh VRRenderer::createMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    Mesh mesh;
    
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // TexCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    mesh.indexCount = indices.size();
    pImpl->meshes.push_back(mesh);
    return mesh;
}

void VRRenderer::renderMesh(const Mesh& mesh, const glm::mat4& modelMatrix) {
    useShaderProgram(pImpl->currentShader);
    setUniform("model", modelMatrix);
    setUniform("view", pImpl->viewMatrix);
    setUniform("projection", pImpl->projectionMatrix);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void VRRenderer::deleteMesh(Mesh& mesh) {
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
    mesh.vao = mesh.vbo = mesh.ebo = 0;
    mesh.indexCount = 0;
}

Texture VRRenderer::loadTexture(const std::string& path) {
    Texture texture;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &texture.width, &texture.height, &texture.channels, 0);
    if (!data) {
        std::cerr << "Fehler beim Laden der Textur: " << path << std::endl;
        return texture;
    }

    GLenum format = GL_RGB;
    if (texture.channels == 4) {
        format = GL_RGBA;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    pImpl->textures.push_back(texture);
    return texture;
}

void VRRenderer::bindTexture(const Texture& texture, unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void VRRenderer::deleteTexture(Texture& texture) {
    glDeleteTextures(1, &texture.id);
    texture.id = 0;
}

void VRRenderer::beginFrame() {
    glClearColor(pImpl->clearColor.r, pImpl->clearColor.g, pImpl->clearColor.b, pImpl->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VRRenderer::endFrame() {
    glfwSwapBuffers(pImpl->window);
    glfwPollEvents();
}

void VRRenderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VRRenderer::setViewport(int x, int y, int width, int height) {
    pImpl->viewportX = x;
    pImpl->viewportY = y;
    pImpl->viewportWidth = width;
    pImpl->viewportHeight = height;
    glViewport(x, y, width, height);
}

void VRRenderer::setClearColor(const glm::vec4& color) {
    pImpl->clearColor = color;
}

void VRRenderer::setUniform(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(pImpl->currentShader.id, name.c_str()), value);
}

void VRRenderer::setUniform(const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(pImpl->currentShader.id, name.c_str()), 1, glm::value_ptr(value));
}

void VRRenderer::setUniform(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(pImpl->currentShader.id, name.c_str()), 1, glm::value_ptr(value));
}

void VRRenderer::setUniform(const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(pImpl->currentShader.id, name.c_str()), 1, glm::value_ptr(value));
}

void VRRenderer::setUniform(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(pImpl->currentShader.id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace VR_DAW 