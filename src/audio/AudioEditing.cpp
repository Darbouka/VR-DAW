#include "AudioEditing.hpp"
#include <vector>
#include <string>

namespace VR_DAW {

AudioEditing::AudioEditing() = default;
AudioEditing::~AudioEditing() = default;

void AudioEditing::initialize() {}
void AudioEditing::shutdown() {}
void AudioEditing::processBlock(std::vector<float>& buffer) {
    // Dummy: keine echte Verarbeitung
}
void AudioEditing::setParameter(const std::string& name, float value) {
    if (name == "gain") gain = value;
    else if (name == "pan") pan = value;
    else if (name == "fadeIn") fadeIn = value;
    else if (name == "fadeOut") fadeOut = value;
}
float AudioEditing::getParameter(const std::string& name) const {
    if (name == "gain") return gain;
    else if (name == "pan") return pan;
    else if (name == "fadeIn") return fadeIn;
    else if (name == "fadeOut") return fadeOut;
    return 0.0f;
}

} // namespace VR_DAW 