#pragma once
#include <string>
#include <vector>

namespace VR_DAW {

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    void newProject(const std::string& name);
    bool loadProject(const std::string& path);
    bool saveProject(const std::string& path);
    void addAudioFile(const std::string& filePath);
    void removeAudioFile(const std::string& filePath);

private:
    struct Impl;
    Impl* pImpl;
};

} // namespace VR_DAW 