#include "UIMeshGenerator.hpp"
#include <cmath>

namespace VR_DAW {

void UIMeshGenerator::addVertex(std::vector<float>& vertices, const glm::vec3& pos, const glm::vec2& texCoord, const glm::vec3& normal) {
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);
    vertices.push_back(texCoord.x);
    vertices.push_back(texCoord.y);
    vertices.push_back(normal.x);
    vertices.push_back(normal.y);
    vertices.push_back(normal.z);
}

void UIMeshGenerator::addTriangle(std::vector<unsigned int>& indices, unsigned int a, unsigned int b, unsigned int c) {
    indices.push_back(a);
    indices.push_back(b);
    indices.push_back(c);
}

void UIMeshGenerator::addQuad(std::vector<unsigned int>& indices, unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
    addTriangle(indices, a, b, c);
    addTriangle(indices, a, c, d);
}

glm::vec3 UIMeshGenerator::calculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    return glm::normalize(glm::cross(edge1, edge2));
}

VRRenderer::Mesh UIMeshGenerator::createQuad(float width, float height) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Vertices
    addVertex(vertices, glm::vec3(-halfWidth, -halfHeight, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(halfWidth, -halfHeight, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(halfWidth, halfHeight, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(-halfWidth, halfHeight, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Indices
    addQuad(indices, 0, 1, 2, 3);
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createCircle(float radius, int segments) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Mittelpunkt
    addVertex(vertices, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Kreis-Punkte
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * M_PI;
        float x = std::cos(angle) * radius;
        float y = std::sin(angle) * radius;
        float u = (x / radius + 1.0f) * 0.5f;
        float v = (y / radius + 1.0f) * 0.5f;
        
        addVertex(vertices, glm::vec3(x, y, 0.0f), glm::vec2(u, v), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // Indices
    for (int i = 1; i < segments; ++i) {
        addTriangle(indices, 0, i, i + 1);
    }
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createRoundedRect(float width, float height, float radius, int segments) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Eckpunkte
    glm::vec3 corners[4] = {
        glm::vec3(-halfWidth + radius, -halfHeight, 0.0f),
        glm::vec3(halfWidth - radius, -halfHeight, 0.0f),
        glm::vec3(halfWidth - radius, halfHeight, 0.0f),
        glm::vec3(-halfWidth + radius, halfHeight, 0.0f)
    };
    
    // Mittelpunkt
    addVertex(vertices, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Eckpunkte hinzufügen
    for (int i = 0; i < 4; ++i) {
        addVertex(vertices, corners[i], glm::vec2(0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // Abgerundete Ecken
    for (int corner = 0; corner < 4; ++corner) {
        float startAngle = corner * M_PI_2;
        glm::vec3 center = corners[corner];
        
        for (int i = 0; i <= segments; ++i) {
            float angle = startAngle + (float)i / segments * M_PI_2;
            float x = center.x + std::cos(angle) * radius;
            float y = center.y + std::sin(angle) * radius;
            float u = (x / width + 0.5f);
            float v = (y / height + 0.5f);
            
            addVertex(vertices, glm::vec3(x, y, 0.0f), glm::vec2(u, v), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
    
    // Indices für die Flächen
    unsigned int centerIndex = 0;
    unsigned int cornerIndices[4] = {1, 2, 3, 4};
    unsigned int segmentStartIndex = 5;
    
    for (int corner = 0; corner < 4; ++corner) {
        unsigned int startIndex = segmentStartIndex + corner * (segments + 1);
        
        // Dreiecke zur Mitte
        for (int i = 0; i < segments; ++i) {
            addTriangle(indices, centerIndex, startIndex + i, startIndex + i + 1);
        }
        
        // Dreiecke zu den Ecken
        addTriangle(indices, centerIndex, startIndex + segments, cornerIndices[corner]);
    }
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createButton(float width, float height, float depth) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Vorderseite
    addVertex(vertices, glm::vec3(-halfWidth, -halfHeight, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(halfWidth, -halfHeight, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(halfWidth, halfHeight, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(-halfWidth, halfHeight, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Rückseite
    addVertex(vertices, glm::vec3(-halfWidth, -halfHeight, -depth), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    addVertex(vertices, glm::vec3(halfWidth, -halfHeight, -depth), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    addVertex(vertices, glm::vec3(halfWidth, halfHeight, -depth), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    addVertex(vertices, glm::vec3(-halfWidth, halfHeight, -depth), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    
    // Indices
    // Vorderseite
    addQuad(indices, 0, 1, 2, 3);
    // Rückseite
    addQuad(indices, 4, 5, 6, 7);
    // Seiten
    addQuad(indices, 0, 4, 7, 3); // Links
    addQuad(indices, 1, 5, 6, 2); // Rechts
    addQuad(indices, 0, 1, 5, 4); // Unten
    addQuad(indices, 3, 2, 6, 7); // Oben
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createSlider(float width, float height, float depth) {
    return createButton(width, height, depth); // Slider verwendet das gleiche Mesh wie Button
}

VRRenderer::Mesh UIMeshGenerator::createKnob(float radius, float depth, int segments) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Oberfläche
    for (int i = 0; i <= segments; ++i) {
        float phi = (float)i / segments * M_PI;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        for (int j = 0; j <= segments; ++j) {
            float theta = (float)j / segments * 2.0f * M_PI;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            
            float x = radius * sinPhi * cosTheta;
            float y = radius * sinPhi * sinTheta;
            float z = radius * cosPhi;
            
            float u = (float)j / segments;
            float v = (float)i / segments;
            
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            addVertex(vertices, glm::vec3(x, y, z), glm::vec2(u, v), normal);
        }
    }
    
    // Indices
    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int current = i * (segments + 1) + j;
            int next = current + 1;
            int bottom = current + segments + 1;
            int bottomNext = bottom + 1;
            
            addTriangle(indices, current, bottom, next);
            addTriangle(indices, next, bottom, bottomNext);
        }
    }
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createWaveform(float width, float height, float depth, const std::vector<float>& samples) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Basis-Vertices
    addVertex(vertices, glm::vec3(-halfWidth, -halfHeight, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    addVertex(vertices, glm::vec3(halfWidth, -halfHeight, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Sample-Vertices
    for (size_t i = 0; i < samples.size(); ++i) {
        float x = -halfWidth + (float)i / (samples.size() - 1) * width;
        float y = samples[i] * halfHeight;
        float u = (float)i / (samples.size() - 1);
        
        addVertex(vertices, glm::vec3(x, y, 0.0f), glm::vec2(u, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // Indices für die Wellenform
    for (size_t i = 0; i < samples.size() - 1; ++i) {
        addTriangle(indices, 0, i + 2, i + 3);
        addTriangle(indices, 1, i + 3, i + 2);
    }
    
    return VRRenderer::getInstance().createMesh(vertices, indices);
}

VRRenderer::Mesh UIMeshGenerator::createTextMesh(const std::string& text, float fontSize, float depth) {
    // TODO: Implementierung der Text-Mesh-Generierung
    // Dies erfordert eine Font-Rendering-Bibliothek wie FreeType
    return VRRenderer::Mesh();
}

} // namespace VR_DAW 