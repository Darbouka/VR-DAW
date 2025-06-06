#pragma once

#include <vector>
#include <string>

namespace VR_DAW {

class AIMasteringProcessor {
public:
    AIMasteringProcessor();
    ~AIMasteringProcessor();

    void initialize();
    void shutdown();
    void processBlock(std::vector<float>& buffer);
    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;

private:
    float threshold = -10.0f;
    float ratio = 2.0f;
    float attack = 0.01f;
    float release = 0.1f;
    float makeupGain = 1.0f;
    float limiterThreshold = -0.1f;
    float outputGain = 1.0f;
};

} // namespace VR_DAW 