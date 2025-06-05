#pragma once
#include <string>
#include <vector>

namespace VR_DAW {

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    bool loadPlugin(const std::string& path);
    void unloadPlugin(int pluginId);
    void setParameter(int pluginId, const std::string& param, float value);

private:
    struct Impl;
    Impl* pImpl;
};

} // namespace VR_DAW 