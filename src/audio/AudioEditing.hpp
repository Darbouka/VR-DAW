#pragma once

#include <vector>
#include <string>

namespace VR_DAW {

class AudioEditing {
public:
    AudioEditing();
    ~AudioEditing();

    void initialize();
    void shutdown();
    void processBlock(std::vector<float>& buffer);
    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;

private:
    float gain = 1.0f;
    float pan = 0.0f;
    float fadeIn = 0.0f;
    float fadeOut = 0.0f;
};

} // namespace VR_DAW 