#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../collaboration/CollaborationManager.hpp"

namespace VR_DAW {

class CollaborationUI : public juce::Component,
                       public juce::ListBoxModel {
public:
    CollaborationUI();
    ~CollaborationUI() override;
    
    // UI-Initialisierung
    void initialize();
    void shutdown();
    
    // Projekt-Management UI
    void showProjectList();
    void showProjectDetails(const std::string& projectId);
    void showCreateProjectDialog();
    void showShareProjectDialog(const std::string& projectId);
    
    // Benutzer-Management UI
    void showUserList();
    void showUserDetails(const std::string& userId);
    void showInviteUserDialog();
    void showRoleManagementDialog(const std::string& userId);
    
    // Chat und Kommunikation UI
    void showChatWindow();
    void showAudioChatWindow();
    void showVideoChatWindow();
    
    // Versionierung UI
    void showVersionHistory();
    void showVersionComparison(const std::string& versionId1, const std::string& versionId2);
    void showCreateVersionDialog();
    
    // Cloud-Synchronisation UI
    void showSyncStatus();
    void showSyncSettings();
    void showCloudStorageDialog();
    
    // Berechtigungen UI
    void showPermissionManager();
    void showPermissionDialog(const std::string& userId);
    
    // Event-Handler
    void onProjectCreated(const CollaborationManager::CollaborationEvent& event);
    void onProjectJoined(const CollaborationManager::CollaborationEvent& event);
    void onProjectLeft(const CollaborationManager::CollaborationEvent& event);
    void onUserInvited(const CollaborationManager::CollaborationEvent& event);
    void onUserRemoved(const CollaborationManager::CollaborationEvent& event);
    void onMessageReceived(const CollaborationManager::CollaborationEvent& event);
    void onVersionCreated(const CollaborationManager::CollaborationEvent& event);
    void onSyncStarted(const CollaborationManager::CollaborationEvent& event);
    void onSyncCompleted(const CollaborationManager::CollaborationEvent& event);
    
    // ListBoxModel Interface
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    
private:
    // UI-Komponenten
    std::unique_ptr<juce::TabbedComponent> mainTabs;
    std::unique_ptr<juce::ListBox> projectList;
    std::unique_ptr<juce::ListBox> userList;
    std::unique_ptr<juce::TextEditor> chatWindow;
    std::unique_ptr<juce::ListBox> versionList;
    std::unique_ptr<juce::TextButton> createProjectButton;
    std::unique_ptr<juce::TextButton> inviteUserButton;
    std::unique_ptr<juce::TextButton> startChatButton;
    std::unique_ptr<juce::TextButton> syncButton;
    
    // Dialog-Komponenten
    std::unique_ptr<juce::DialogWindow> createProjectDialog;
    std::unique_ptr<juce::DialogWindow> inviteUserDialog;
    std::unique_ptr<juce::DialogWindow> roleManagementDialog;
    std::unique_ptr<juce::DialogWindow> versionDialog;
    std::unique_ptr<juce::DialogWindow> syncDialog;
    
    // Status-Anzeigen
    std::unique_ptr<juce::Label> connectionStatus;
    std::unique_ptr<juce::Label> syncStatus;
    std::unique_ptr<juce::Label> userCount;
    
    // Interne Hilfsfunktionen
    void setupUI();
    void setupEventHandlers();
    void updateProjectList();
    void updateUserList();
    void updateVersionList();
    void updateSyncStatus();
    void showNotification(const std::string& message);
    void handleError(const std::string& error);
    
    // Callback-Funktionen
    void onProjectSelected(int row);
    void onUserSelected(int row);
    void onVersionSelected(int row);
    void onCreateProjectClicked();
    void onInviteUserClicked();
    void onStartChatClicked();
    void onSyncClicked();
    
    // Event-Callbacks
    std::vector<std::function<void(const CollaborationManager::CollaborationEvent&)>> eventCallbacks;
};

} // namespace VR_DAW 