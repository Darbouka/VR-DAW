#pragma once

#include <string>
#include <vector>
#include <memory>
#include <random>

namespace VR_DAW {

struct CommunityUser {
    std::string id;
    std::string username;
    std::string email;
};

struct CommunityProject {
    std::string id;
    std::string name;
    std::string ownerId;
};

struct CommunityEvent {
    enum class Type {
        UserJoined,
        UserLeft,
        ProjectCreated,
        ProjectUpdated,
        CollaborationAdded,
        CollaborationRemoved
    };
    
    Type type;
    std::string userId;
    std::string projectId;
    std::string data;
};

class CommunityManager {
public:
    CommunityManager();
    ~CommunityManager();
    
    bool initialize(const std::string& dbPath, const std::string& apiUrl);
    void shutdown();
    
    std::vector<CommunityUser> getUsers() const;
    std::vector<CommunityProject> getProjects() const;
    
    bool createProject(const std::string& name, const std::string& ownerId);
    bool joinProject(const std::string& projectId, const std::string& userId, const std::string& role);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    
    std::string generateUUID();
};

} // namespace VR_DAW 