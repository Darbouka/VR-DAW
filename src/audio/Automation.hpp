#pragma once

#include <vector>
#include <string>

namespace VR_DAW {

class Automation {
public:
    Automation();
    ~Automation();

    void initialize();
    void shutdown();
    void processBlock(std::vector<float>& buffer);
    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;

private:
    float automationValue = 0.0f;
};

} // namespace VR_DAW 