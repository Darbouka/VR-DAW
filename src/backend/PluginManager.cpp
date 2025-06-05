#include "PluginManager.hpp"
#include <iostream>
#include <vector>

namespace VR_DAW {

struct PluginManager::Impl {
    std::vector<std::string> plugins;
};

PluginManager::PluginManager() : pImpl(new Impl) {}
PluginManager::~PluginManager() { delete pImpl; }

bool PluginManager::loadPlugin(const std::string& path) {
    pImpl->plugins.push_back(path);
    std::cout << "Plugin geladen: " << path << std::endl;
    return true;
}

void PluginManager::unloadPlugin(int pluginId) {
    if (pluginId >= 0 && pluginId < static_cast<int>(pImpl->plugins.size())) {
        std::cout << "Plugin entfernt: " << pImpl->plugins[pluginId] << std::endl;
        pImpl->plugins.erase(pImpl->plugins.begin() + pluginId);
    }
}

void PluginManager::setParameter(int pluginId, const std::string& param, float value) {
    if (pluginId >= 0 && pluginId < static_cast<int>(pImpl->plugins.size())) {
        std::cout << "Parameter gesetzt: Plugin " << pluginId << ", " << param << " = " << value << std::endl;
    }
}

} // namespace VR_DAW 