#include "ProjectManager.hpp"
#include <iostream>
#include <vector>

namespace VR_DAW {

struct ProjectManager::Impl {
    std::string projectName;
    std::vector<std::string> audioFiles;
};

ProjectManager::ProjectManager() : pImpl(new Impl) {}
ProjectManager::~ProjectManager() { delete pImpl; }

void ProjectManager::newProject(const std::string& name) {
    pImpl->projectName = name;
    pImpl->audioFiles.clear();
    std::cout << "Neues Projekt erstellt: " << name << std::endl;
}

bool ProjectManager::loadProject(const std::string& path) {
    // TODO: Projekt aus Datei laden
    std::cout << "Projekt geladen: " << path << std::endl;
    return true;
}

bool ProjectManager::saveProject(const std::string& path) {
    // TODO: Projekt in Datei speichern
    std::cout << "Projekt gespeichert: " << path << std::endl;
    return true;
}

void ProjectManager::addAudioFile(const std::string& filePath) {
    pImpl->audioFiles.push_back(filePath);
    std::cout << "Audio-Datei hinzugefügt: " << filePath << std::endl;
}

void ProjectManager::removeAudioFile(const std::string& filePath) {
    auto& files = pImpl->audioFiles;
    files.erase(std::remove(files.begin(), files.end(), filePath), files.end());
    std::cout << "Audio-Datei entfernt: " << filePath << std::endl;
}

} // namespace VR_DAW 