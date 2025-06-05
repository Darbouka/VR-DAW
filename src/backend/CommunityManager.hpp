#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <filesystem>

namespace VR_DAW {

struct Project {
    int id;
    std::string name;
    std::vector<std::string> files;
};

class CommunityManager {
public:
    CommunityManager();
    ~CommunityManager();

    bool login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password, const std::string& email);
    bool uploadProject(const std::string& projectPath);
    bool downloadProject(const std::string& projectId);
    bool startCollaboration(int projectId);
    bool inviteCollaborator(int projectId, const std::string& username, const std::string& role = "viewer");
    std::vector<Project> getProjects();
    bool createProject(const std::string& name);
    bool uploadFile(int projectId, const std::string& filePath);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    std::string makeRequest(const std::string& endpoint, const std::string& method, const std::string& data = "");
    std::string getAuthToken() const;
};

} // namespace VR_DAW 