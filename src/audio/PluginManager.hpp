#pragma once

#include <string>
#include <vector>
#include <memory>

namespace VRDAW {

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    bool loadPlugin(const std::string& path);
    void unloadPlugin(const std::string& name);
    std::vector<std::string> getLoadedPlugins() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace VRDAW 