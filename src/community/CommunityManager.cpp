#include "CommunityManager.hpp"
#include <sqlite3.h>
#include <curl/curl.h>
#include <json/json.h>
#include <thread>
#include <mutex>
#include <queue>
#include <filesystem>

namespace VR_DAW {

class CommunityManager::Impl {
public:
    sqlite3* db;
    CURL* curl;
    std::thread updateThread;
    std::mutex mutex;
    std::queue<CommunityEvent> eventQueue;
    bool isInitialized;
    std::string dbPath;
    std::string apiUrl;
    std::vector<CommunityUser> users;
    std::vector<CommunityProject> projects;
    
    Impl() : db(nullptr), curl(nullptr), isInitialized(false) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }
    
    ~Impl() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        
        if (db) {
            sqlite3_close(db);
        }
    }
    
    bool initializeDatabase() {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            return false;
        }
        
        // Tabellen erstellen
        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id TEXT PRIMARY KEY,
                username TEXT NOT NULL,
                email TEXT UNIQUE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            
            CREATE TABLE IF NOT EXISTS projects (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                owner_id TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (owner_id) REFERENCES users(id)
            );
            
            CREATE TABLE IF NOT EXISTS collaborations (
                project_id TEXT,
                user_id TEXT,
                role TEXT,
                joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                PRIMARY KEY (project_id, user_id),
                FOREIGN KEY (project_id) REFERENCES projects(id),
                FOREIGN KEY (user_id) REFERENCES users(id)
            );
        )";
        
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            return false;
        }
        
        return true;
    }
    
    bool syncWithServer() {
        if (!curl) return false;
        
        try {
            // Benutzer synchronisieren
            std::string userUrl = apiUrl + "/users";
            curl_easy_setopt(curl, CURLOPT_URL, userUrl.c_str());
            
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) return false;
            
            // JSON parsen
            Json::Value root;
            Json::Reader reader;
            if (reader.parse(response, root)) {
                std::lock_guard<std::mutex> lock(mutex);
                users.clear();
                
                for (const auto& user : root) {
                    CommunityUser communityUser;
                    communityUser.id = user["id"].asString();
                    communityUser.username = user["username"].asString();
                    communityUser.email = user["email"].asString();
                    users.push_back(communityUser);
                }
            }
            
            // Projekte synchronisieren
            std::string projectUrl = apiUrl + "/projects";
            curl_easy_setopt(curl, CURLOPT_URL, projectUrl.c_str());
            
            response.clear();
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) return false;
            
            if (reader.parse(response, root)) {
                std::lock_guard<std::mutex> lock(mutex);
                projects.clear();
                
                for (const auto& project : root) {
                    CommunityProject communityProject;
                    communityProject.id = project["id"].asString();
                    communityProject.name = project["name"].asString();
                    communityProject.ownerId = project["owner_id"].asString();
                    projects.push_back(communityProject);
                }
            }
            
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

CommunityManager::CommunityManager() : pImpl(std::make_unique<Impl>()) {}

CommunityManager::~CommunityManager() = default;

bool CommunityManager::initialize(const std::string& dbPath, const std::string& apiUrl) {
    pImpl->dbPath = dbPath;
    pImpl->apiUrl = apiUrl;
    
    if (!pImpl->initializeDatabase()) {
        return false;
    }
    
    pImpl->isInitialized = true;
    
    // Synchronisations-Thread starten
    pImpl->updateThread = std::thread([this]() {
        while (pImpl->isInitialized) {
            pImpl->syncWithServer();
            std::this_thread::sleep_for(std::chrono::minutes(5));
        }
    });
    
    return true;
}

std::vector<CommunityUser> CommunityManager::getUsers() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->users;
}

std::vector<CommunityProject> CommunityManager::getProjects() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->projects;
}

bool CommunityManager::createProject(const std::string& name, const std::string& ownerId) {
    if (!pImpl->isInitialized) return false;
    
    try {
        // Projekt in Datenbank speichern
        const char* sql = "INSERT INTO projects (id, name, owner_id) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        std::string projectId = generateUUID();
        sqlite3_bind_text(stmt, 1, projectId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, ownerId.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
        
        sqlite3_finalize(stmt);
        
        // Projekt zum Server hochladen
        Json::Value root;
        root["id"] = projectId;
        root["name"] = name;
        root["owner_id"] = ownerId;
        
        Json::FastWriter writer;
        std::string jsonStr = writer.write(root);
        
        curl_easy_setopt(pImpl->curl, CURLOPT_URL, (pImpl->apiUrl + "/projects").c_str());
        curl_easy_setopt(pImpl->curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        
        CURLcode res = curl_easy_perform(pImpl->curl);
        if (res != CURLE_OK) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool CommunityManager::joinProject(const std::string& projectId, const std::string& userId, const std::string& role) {
    if (!pImpl->isInitialized) return false;
    
    try {
        // Kollaboration in Datenbank speichern
        const char* sql = "INSERT INTO collaborations (project_id, user_id, role) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, projectId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
        
        sqlite3_finalize(stmt);
        
        // Kollaboration zum Server hochladen
        Json::Value root;
        root["project_id"] = projectId;
        root["user_id"] = userId;
        root["role"] = role;
        
        Json::FastWriter writer;
        std::string jsonStr = writer.write(root);
        
        curl_easy_setopt(pImpl->curl, CURLOPT_URL, (pImpl->apiUrl + "/collaborations").c_str());
        curl_easy_setopt(pImpl->curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        
        CURLcode res = curl_easy_perform(pImpl->curl);
        if (res != CURLE_OK) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string CommunityManager::generateUUID() {
    // Einfache UUID-Generierung
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";
    
    std::string uuid;
    uuid.reserve(36);
    
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid += '-';
        } else {
            uuid += hex[dis(gen)];
        }
    }
    
    return uuid;
}

void CommunityManager::shutdown() {
    if (pImpl->isInitialized) {
        pImpl->isInitialized = false;
        if (pImpl->updateThread.joinable()) {
            pImpl->updateThread.join();
        }
    }
}

} // namespace VR_DAW 