#include "PluginManager.hpp"

namespace VRDAW {

struct PluginManager::Impl {
    std::vector<std::string> loadedPlugins;
};

PluginManager::PluginManager() : pImpl(std::make_unique<Impl>()) {}
PluginManager::~PluginManager() = default;

bool PluginManager::loadPlugin(const std::string& path) {
    // TODO: Implement plugin loading
    return false;
}

void PluginManager::unloadPlugin(const std::string& name) {
    // TODO: Implement plugin unloading
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
    return pImpl->loadedPlugins;
}

} // namespace VRDAW 