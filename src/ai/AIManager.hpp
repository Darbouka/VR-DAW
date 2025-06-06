#pragma once

#include <string>
#include <vector>

namespace VR_DAW {

class AIManager {
public:
    AIManager();
    ~AIManager();

    void loadModel(const std::string& path);
    void processAudio(const std::vector<float>& input, std::vector<float>& output);
};

} // namespace VR_DAW 