#include "CommunityManager.hpp"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace VR_DAW {

struct CommunityManager::Impl {
    std::string currentUser;
    std::string authToken;
    std::string baseUrl = "http://localhost:8080/api";
    std::string uploadDir = "uploads";
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

CommunityManager::CommunityManager() : pImpl(std::make_unique<Impl>()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::filesystem::create_directories(pImpl->uploadDir);
}

CommunityManager::~CommunityManager() {
    curl_global_cleanup();
}

std::string CommunityManager::makeRequest(const std::string& endpoint, const std::string& method, const std::string& data) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = pImpl->baseUrl + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        }

        if (!pImpl->authToken.empty()) {
            std::string authHeader = "Authorization: Bearer " + pImpl->authToken;
            curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, authHeader.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

bool CommunityManager::login(const std::string& username, const std::string& password) {
    json loginData = {
        {"username", username},
        {"password", password}
    };

    std::string response = makeRequest("/auth/login", "POST", loginData.dump());
    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("access_token")) {
            pImpl->authToken = responseJson["access_token"];
            pImpl->currentUser = username;
            std::cout << "User eingeloggt: " << username << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Login-Fehler: " << e.what() << std::endl;
    }
    return false;
}

bool CommunityManager::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    json registerData = {
        {"username", username},
        {"password", password},
        {"email", email}
    };

    std::string response = makeRequest("/auth/register", "POST", registerData.dump());
    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("message")) {
            std::cout << "User registriert: " << username << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Registrierungs-Fehler: " << e.what() << std::endl;
    }
    return false;
}

std::vector<Project> CommunityManager::getProjects() {
    std::vector<Project> result;
    std::string response = makeRequest("/projects", "GET");
    
    try {
        json projectsJson = json::parse(response);
        for (const auto& projectJson : projectsJson) {
            Project project;
            project.id = projectJson["id"];
            project.name = projectJson["name"];
            for (const auto& file : projectJson["files"]) {
                project.files.push_back(file["name"]);
            }
            result.push_back(project);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Laden der Projekte: " << e.what() << std::endl;
    }
    
    return result;
}

bool CommunityManager::createProject(const std::string& name) {
    json projectData = {
        {"name", name}
    };

    std::string response = makeRequest("/projects", "POST", projectData.dump());
    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("id")) {
            std::cout << "Projekt erstellt: " << name << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Erstellen des Projekts: " << e.what() << std::endl;
    }
    return false;
}

bool CommunityManager::uploadFile(int projectId, const std::string& filePath) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string response;
    struct curl_httppost* formpost = nullptr;
    struct curl_httppost* lastptr = nullptr;

    // Datei zum Formular hinzufügen
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "file",
        CURLFORM_FILE, filePath.c_str(),
        CURLFORM_END);

    std::string url = pImpl->baseUrl + "/projects/" + std::to_string(projectId) + "/files";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (!pImpl->authToken.empty()) {
        std::string authHeader = "Authorization: Bearer " + pImpl->authToken;
        curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    curl_formfree(formpost);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Upload-Fehler: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("id")) {
            std::cout << "Datei hochgeladen: " << filePath << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Hochladen der Datei: " << e.what() << std::endl;
    }
    return false;
}

bool CommunityManager::uploadProject(const std::string& projectPath) {
    // TODO: Implementiere Projekt-Upload
    std::cout << "Projekt hochgeladen: " << projectPath << std::endl;
    return true;
}

bool CommunityManager::downloadProject(const std::string& projectId) {
    // TODO: Implementiere Projekt-Download
    std::cout << "Projekt heruntergeladen: " << projectId << std::endl;
    return true;
}

bool CommunityManager::startCollaboration(int projectId) {
    std::string response = makeRequest("/collaboration/" + std::to_string(projectId), "POST");
    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("session_id")) {
            std::cout << "Kollaboration gestartet für Projekt: " << projectId << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Starten der Kollaboration: " << e.what() << std::endl;
    }
    return false;
}

bool CommunityManager::inviteCollaborator(int projectId, const std::string& username, const std::string& role) {
    json inviteData = {
        {"username", username},
        {"role", role}
    };

    std::string response = makeRequest("/collaboration/" + std::to_string(projectId) + "/invite", "POST", inviteData.dump());
    try {
        json responseJson = json::parse(response);
        if (responseJson.contains("message")) {
            std::cout << "Benutzer eingeladen: " << username << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fehler beim Einladen des Benutzers: " << e.what() << std::endl;
    }
    return false;
}

} // namespace VR_DAW 