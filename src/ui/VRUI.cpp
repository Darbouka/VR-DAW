#define GLM_ENABLE_EXPERIMENTAL
#include "VRUI.hpp"
#include <memory>

namespace VR_DAW {

struct VRUI::Impl {
    bool isInitialized = false;
    float scale = 1.0f;
};

VRUI::VRUI() : pImpl(std::make_unique<Impl>()) {}
VRUI::~VRUI() = default;

VRUI& VRUI::getInstance() {
    static VRUI instance;
    return instance;
}

bool VRUI::initialize() {
    pImpl->isInitialized = true;
    return true;
}

void VRUI::shutdown() {
    pImpl->isInitialized = false;
}

void VRUI::render() {
    if (!pImpl->isInitialized) return;
    // TODO: Rendering-Code
}

void VRUI::handleInput() {
    if (!pImpl->isInitialized) return;
    // TODO: Input-Code
}

void VRUI::setScale(float scale) {
    pImpl->scale = scale;
}

} // namespace VR_DAW 