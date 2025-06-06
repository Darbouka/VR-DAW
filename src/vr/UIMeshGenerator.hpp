#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "VRRenderer.hpp"

namespace VR_DAW {

class UIMeshGenerator {
public:
    // Grundlegende UI-Elemente
    static VRRenderer::Mesh createQuad(float width, float height);
    static VRRenderer::Mesh createCircle(float radius, int segments);
    static VRRenderer::Mesh createRoundedRect(float width, float height, float radius, int segments);
    
    // Komplexe UI-Elemente
    static VRRenderer::Mesh createButton(float width, float height, float depth);
    static VRRenderer::Mesh createSlider(float width, float height, float depth);
    static VRRenderer::Mesh createKnob(float radius, float depth, int segments);
    static VRRenderer::Mesh createWaveform(float width, float height, float depth, const std::vector<float>& samples);
    
    // Text-Meshes
    static VRRenderer::Mesh createTextMesh(const std::string& text, float fontSize, float depth);
    
private:
    // Hilfsfunktionen
    static void addVertex(std::vector<float>& vertices, const glm::vec3& pos, const glm::vec2& texCoord, const glm::vec3& normal);
    static void addTriangle(std::vector<unsigned int>& indices, unsigned int a, unsigned int b, unsigned int c);
    static void addQuad(std::vector<unsigned int>& indices, unsigned int a, unsigned int b, unsigned int c, unsigned int d);
    static glm::vec3 calculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
};

} // namespace VR_DAW 