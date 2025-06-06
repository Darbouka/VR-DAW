#include "Automation.hpp"
#include <vector>
#include <string>

namespace VR_DAW {

Automation::Automation() = default;
Automation::~Automation() = default;

void Automation::initialize() {}
void Automation::shutdown() {}
void Automation::processBlock(std::vector<float>& buffer) {
    // Dummy: keine echte Verarbeitung
}
void Automation::setParameter(const std::string& name, float value) {
    if (name == "automationValue") automationValue = value;
}
float Automation::getParameter(const std::string& name) const {
    if (name == "automationValue") return automationValue;
    return 0.0f;
}

} // namespace VR_DAW 