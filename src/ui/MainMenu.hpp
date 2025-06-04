#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace VR_DAW {

class MainMenu {
public:
    static MainMenu& getInstance();
    
    void render();
    
private:
    MainMenu() = default;
    ~MainMenu() = default;
    
    MainMenu(const MainMenu&) = delete;
    MainMenu& operator=(const MainMenu&) = delete;
    
    // KI-Assistent Funktionen
    void handleAIMastering();
    void handleAIMixing();
    void handleAISoundDesign();
    void handleAIModelSelection();
    void handleAIParameters();
    void handleAILearningMode();
    void handleAITemplates();
    
    // Sprachausgabe Funktionen
    void handleVoiceSelection(const std::string& voiceType);
    void handleVoiceGeneration();
    void handleVoiceCustomization();
    void handleVoiceManagement();
    void handleVoiceSettings();
    void handleVoiceTemplates();
    
    // Menü-Handler
    void handleFileMenu();
    void handleEditMenu();
    void handleAIMenu();
    void handleViewMenu();
    void handleHelpMenu();
    
    // KI-Einstellungen
    struct AISettings {
        std::string currentModel;
        bool learningMode;
        float sensitivity;
        std::vector<std::string> templates;
    };
    
    // Sprachausgabe-Einstellungen
    struct VoiceSettings {
        std::string currentVoice;
        std::string language;
        float pitch;
        float speed;
        float volume;
        bool useAIEnhancement;
        std::map<std::string, std::vector<std::string>> availableVoices;
    };
    
    AISettings aiSettings;
    VoiceSettings voiceSettings;
};

} // namespace VR_DAW 