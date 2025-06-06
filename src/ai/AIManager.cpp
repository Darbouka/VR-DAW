#include "AIManager.hpp"
#include <torch/torch.h>
#include <torch/script.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

namespace VR_DAW {

class AIManager::Impl {
public:
    torch::jit::script::Module model;
    bool isModelLoaded;
    std::string modelPath;
    std::vector<float> audioBuffer;
    std::vector<float> featureBuffer;
    std::map<std::string, torch::jit::script::Module> additionalModels;
    
    Impl() : isModelLoaded(false) {
        audioBuffer.reserve(44100); // 1 Sekunde Audio bei 44.1kHz
        featureBuffer.reserve(1024);
    }
    
    bool loadModel(const std::string& path) {
        try {
            model = torch::jit::load(path);
            model.eval();
            isModelLoaded = true;
            modelPath = path;
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    std::vector<float> extractFeatures(const std::vector<float>& audio) {
        if (!isModelLoaded) return {};
        
        try {
            // Audio in Tensor konvertieren
            auto input = torch::from_blob(const_cast<float*>(audio.data()),
                                        {1, static_cast<long>(audio.size())},
                                        torch::kFloat32);
            
            // Features extrahieren
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(input);
            auto output = model.forward(inputs).toTensor();
            
            // Tensor in Vektor konvertieren
            std::vector<float> features(output.data_ptr<float>(),
                                      output.data_ptr<float>() + output.numel());
            return features;
        } catch (const std::exception& e) {
            return {};
        }
    }
    
    std::vector<float> generateAudio(const std::vector<float>& features) {
        if (!isModelLoaded) return {};
        
        try {
            // Features in Tensor konvertieren
            auto input = torch::from_blob(const_cast<float*>(features.data()),
                                        {1, static_cast<long>(features.size())},
                                        torch::kFloat32);
            
            // Audio generieren
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(input);
            auto output = model.forward(inputs).toTensor();
            
            // Tensor in Vektor konvertieren
            std::vector<float> audio(output.data_ptr<float>(),
                                   output.data_ptr<float>() + output.numel());
            return audio;
        } catch (const std::exception& e) {
            return {};
        }
    }
    
    std::vector<float> processAudio(const std::vector<float>& audio) {
        if (!isModelLoaded) return audio;
        
        try {
            // Features extrahieren
            auto features = extractFeatures(audio);
            if (features.empty()) return audio;
            
            // Audio generieren
            auto processedAudio = generateAudio(features);
            if (processedAudio.empty()) return audio;
            
            return processedAudio;
        } catch (const std::exception& e) {
            return audio;
        }
    }
};

AIManager::AIManager() : pImpl(std::make_unique<Impl>()) {}

AIManager::~AIManager() = default;

void AIManager::loadModel(const std::string& path) {
    try {
        pImpl->model = torch::jit::load(path);
        pImpl->model.eval();
        pImpl->isModelLoaded = true;
        pImpl->modelPath = path;
    } catch (const c10::Error& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
    }
}

void AIManager::processAudio(const std::vector<float>& input, std::vector<float>& output) {
    if (!pImpl->isModelLoaded) {
        output = input;
        return;
    }

    try {
        // Features extrahieren
        auto features = pImpl->extractFeatures(input);
        if (features.empty()) {
            output = input;
            return;
        }
        
        // Audio generieren
        auto processedAudio = pImpl->generateAudio(features);
        if (processedAudio.empty()) {
            output = input;
            return;
        }
        
        output = processedAudio;
    } catch (const std::exception& e) {
        output = input;
    }
}

bool AIManager::trainModel(const std::vector<std::vector<float>>& trainingData, const std::vector<std::vector<float>>& labels) {
    if (!pImpl->isModelLoaded) return false;

    try {
        torch::optim::Adam optimizer(pImpl->model.parameters(), torch::optim::AdamOptions(0.001));
        torch::nn::MSELoss loss;

        for (size_t i = 0; i < trainingData.size(); ++i) {
            torch::Tensor input = torch::from_blob(const_cast<float*>(trainingData[i].data()), {1, static_cast<long>(trainingData[i].size())}, torch::kFloat32);
            torch::Tensor target = torch::from_blob(const_cast<float*>(labels[i].data()), {1, static_cast<long>(labels[i].size())}, torch::kFloat32);

            optimizer.zero_grad();
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(input);
            auto output = pImpl->model.forward(inputs).toTensor();
            auto loss_val = loss(output, target);
            loss_val.backward();
            optimizer.step();
        }
        return true;
    } catch (const c10::Error& e) {
        std::cerr << "Error training model: " << e.what() << std::endl;
        return false;
    }
}

bool AIManager::loadAdditionalModel(const std::string& name, const std::string& path) {
    try {
        pImpl->additionalModels[name] = torch::jit::load(path);
        pImpl->additionalModels[name].eval();
        return true;
    } catch (const c10::Error& e) {
        std::cerr << "Error loading additional model: " << e.what() << std::endl;
        return false;
    }
}

std::vector<float> AIManager::processWithAdditionalModel(const std::string& modelName, const std::vector<float>& inputData) {
    if (pImpl->additionalModels.find(modelName) == pImpl->additionalModels.end()) return inputData;

    try {
        torch::Tensor input = torch::from_blob(const_cast<float*>(inputData.data()), {1, static_cast<long>(inputData.size())}, torch::kFloat32);
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(input);
        auto output = pImpl->additionalModels[modelName].forward(inputs).toTensor();
        std::vector<float> result(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());
        return result;
    } catch (const c10::Error& e) {
        std::cerr << "Error processing with additional model: " << e.what() << std::endl;
        return inputData;
    }
}

bool AIManager::isModelLoaded() const {
    return pImpl->isModelLoaded;
}

std::string AIManager::getModelPath() const {
    return pImpl->modelPath;
}

void AIManager::saveModel(const std::string& path) {
    if (pImpl->isModelLoaded) {
        pImpl->model.save(path);
    }
}

} // namespace VR_DAW 