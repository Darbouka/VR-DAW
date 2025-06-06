#pragma once
#include <string>

namespace VR_DAW {
class NeuralNetwork {
public:
    NeuralNetwork() = default;
    ~NeuralNetwork() = default;
    std::string generateResponse(const std::string& input) { return "Antwort: " + input; }
    void load(const std::string&) {}
    void save(const std::string&) {}
};
} 