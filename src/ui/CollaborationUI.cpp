#include "CollaborationUI.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

namespace VR_DAW {

CollaborationUI::CollaborationUI() {
    setupUI();
    setupEventHandlers();
}

CollaborationUI::~CollaborationUI() {
    shutdown();
}

void CollaborationUI::initialize() {
    // Event-Handler registrieren
    auto& manager = CollaborationManager::getInstance();
    manager.registerEventCallback("project_created", [this](const auto& e) { onProjectCreated(e); });
    manager.registerEventCallback("project_joined", [this](const auto& e) { onProjectJoined(e); });
    manager.registerEventCallback("project_left", [this](const auto& e) { onProjectLeft(e); });
    manager.registerEventCallback("user_invited", [this](const auto& e) { onUserInvited(e); });
    manager.registerEventCallback("user_removed", [this](const auto& e) { onUserRemoved(e); });
    manager.registerEventCallback("message", [this](const auto& e) { onMessageReceived(e); });
    manager.registerEventCallback("version_created", [this](const auto& e) { onVersionCreated(e); });
    manager.registerEventCallback("cloud_sync_started", [this](const auto& e) { onSyncStarted(e); });
    manager.registerEventCallback("cloud_sync_completed", [this](const auto& e) { onSyncCompleted(e); });
}

void CollaborationUI::shutdown() {
    // Event-Handler deregistrieren
    auto& manager = CollaborationManager::getInstance();
    manager.unregisterEventCallback("project_created");
    manager.unregisterEventCallback("project_joined");
    manager.unregisterEventCallback("project_left");
    manager.unregisterEventCallback("user_invited");
    manager.unregisterEventCallback("user_removed");
    manager.unregisterEventCallback("message");
    manager.unregisterEventCallback("version_created");
    manager.unregisterEventCallback("cloud_sync_started");
    manager.unregisterEventCallback("cloud_sync_completed");
}

void CollaborationUI::setupUI() {
    // Haupt-Tabs erstellen
    mainTabs = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(mainTabs.get());
    
    // Projekt-Liste
    projectList = std::make_unique<juce::ListBox>();
    projectList->setRowHeight(30);
    projectList->setModel(this);
    mainTabs->addTab("Projekte", juce::Colours::lightgrey, projectList.get(), false);
    
    // Benutzer-Liste
    userList = std::make_unique<juce::ListBox>();
    userList->setRowHeight(30);
    userList->setModel(this);
    mainTabs->addTab("Benutzer", juce::Colours::lightgrey, userList.get(), false);
    
    // Chat-Fenster
    chatWindow = std::make_unique<juce::TextEditor>();
    chatWindow->setMultiLine(true);
    chatWindow->setReadOnly(true);
    mainTabs->addTab("Chat", juce::Colours::lightgrey, chatWindow.get(), false);
    
    // Versions-Liste
    versionList = std::make_unique<juce::ListBox>();
    versionList->setRowHeight(30);
    versionList->setModel(this);
    mainTabs->addTab("Versionen", juce::Colours::lightgrey, versionList.get(), false);
    
    // Buttons
    createProjectButton = std::make_unique<juce::TextButton>("Neues Projekt");
    inviteUserButton = std::make_unique<juce::TextButton>("Benutzer einladen");
    startChatButton = std::make_unique<juce::TextButton>("Chat starten");
    syncButton = std::make_unique<juce::TextButton>("Synchronisieren");
    
    addAndMakeVisible(createProjectButton.get());
    addAndMakeVisible(inviteUserButton.get());
    addAndMakeVisible(startChatButton.get());
    addAndMakeVisible(syncButton.get());
    
    // Status-Labels
    connectionStatus = std::make_unique<juce::Label>("connectionStatus", "Nicht verbunden");
    syncStatus = std::make_unique<juce::Label>("syncStatus", "Nicht synchronisiert");
    userCount = std::make_unique<juce::Label>("userCount", "0 Benutzer online");
    
    addAndMakeVisible(connectionStatus.get());
    addAndMakeVisible(syncStatus.get());
    addAndMakeVisible(userCount.get());
}

void CollaborationUI::setupEventHandlers() {
    createProjectButton->onClick = [this] { onCreateProjectClicked(); };
    inviteUserButton->onClick = [this] { onInviteUserClicked(); };
    startChatButton->onClick = [this] { onStartChatClicked(); };
    syncButton->onClick = [this] { onSyncClicked(); };
    
    projectList->onRowSelected = [this](int row) { onProjectSelected(row); };
    userList->onRowSelected = [this](int row) { onUserSelected(row); };
    versionList->onRowSelected = [this](int row) { onVersionSelected(row); };
}

void CollaborationUI::showProjectList() {
    mainTabs->setCurrentTabIndex(0);
    updateProjectList();
}

void CollaborationUI::showProjectDetails(const std::string& projectId) {
    auto& manager = CollaborationManager::getInstance();
    auto project = manager.getProject(projectId);
    
    auto* dialog = new juce::DialogWindow(
        "Projekt Details: " + project.name,
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* nameLabel = new juce::Label("nameLabel", "Name: " + project.name);
    auto* ownerLabel = new juce::Label("ownerLabel", "Besitzer: " + project.owner);
    auto* createdLabel = new juce::Label("createdLabel", "Erstellt: " + project.createdAt);
    auto* statusLabel = new juce::Label("statusLabel", "Status: " + project.status);
    
    auto* shareButton = new juce::TextButton("Teilen");
    auto* leaveButton = new juce::TextButton("Verlassen");
    
    content->addAndMakeVisible(nameLabel);
    content->addAndMakeVisible(ownerLabel);
    content->addAndMakeVisible(createdLabel);
    content->addAndMakeVisible(statusLabel);
    content->addAndMakeVisible(shareButton);
    content->addAndMakeVisible(leaveButton);
    
    shareButton->onClick = [this, projectId] {
        showShareProjectDialog(projectId);
    };
    
    leaveButton->onClick = [this, projectId, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.leaveProject(projectId);
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 300);
    dialog->showModal();
}

void CollaborationUI::showCreateProjectDialog() {
    createProjectDialog = std::make_unique<juce::DialogWindow>(
        "Neues Projekt erstellen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* nameLabel = new juce::Label("nameLabel", "Projektname:");
    auto* nameEditor = new juce::TextEditor();
    auto* createButton = new juce::TextButton("Erstellen");
    
    content->addAndMakeVisible(nameLabel);
    content->addAndMakeVisible(nameEditor);
    content->addAndMakeVisible(createButton);
    
    createButton->onClick = [this, nameEditor] {
        auto& manager = CollaborationManager::getInstance();
        manager.createProject(nameEditor->getText().toStdString());
        createProjectDialog->exitModalState(0);
    };
    
    createProjectDialog->setContentOwned(content, true);
    createProjectDialog->centreWithSize(300, 150);
    createProjectDialog->showModal();
}

void CollaborationUI::showShareProjectDialog(const std::string& projectId) {
    auto* dialog = new juce::DialogWindow(
        "Projekt teilen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* emailLabel = new juce::Label("emailLabel", "E-Mail:");
    auto* emailEditor = new juce::TextEditor();
    auto* roleLabel = new juce::Label("roleLabel", "Rolle:");
    auto* roleCombo = new juce::ComboBox();
    roleCombo->addItem("Leser", 1);
    roleCombo->addItem("Bearbeiter", 2);
    roleCombo->addItem("Administrator", 3);
    
    auto* shareButton = new juce::TextButton("Teilen");
    
    content->addAndMakeVisible(emailLabel);
    content->addAndMakeVisible(emailEditor);
    content->addAndMakeVisible(roleLabel);
    content->addAndMakeVisible(roleCombo);
    content->addAndMakeVisible(shareButton);
    
    shareButton->onClick = [this, projectId, emailEditor, roleCombo, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.shareProject(projectId, emailEditor->getText().toStdString(), 
                           static_cast<CollaborationManager::UserRole>(roleCombo->getSelectedId()));
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(300, 200);
    dialog->showModal();
}

void CollaborationUI::showUserList() {
    mainTabs->setCurrentTabIndex(1);
    updateUserList();
}

void CollaborationUI::showUserDetails(const std::string& userId) {
    auto& manager = CollaborationManager::getInstance();
    auto user = manager.getUser(userId);
    
    auto* dialog = new juce::DialogWindow(
        "Benutzer Details: " + user.name,
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* nameLabel = new juce::Label("nameLabel", "Name: " + user.name);
    auto* emailLabel = new juce::Label("emailLabel", "E-Mail: " + user.email);
    auto* roleLabel = new juce::Label("roleLabel", "Rolle: " + user.role);
    auto* statusLabel = new juce::Label("statusLabel", "Status: " + user.status);
    
    auto* manageButton = new juce::TextButton("Rollen verwalten");
    auto* removeButton = new juce::TextButton("Entfernen");
    
    content->addAndMakeVisible(nameLabel);
    content->addAndMakeVisible(emailLabel);
    content->addAndMakeVisible(roleLabel);
    content->addAndMakeVisible(statusLabel);
    content->addAndMakeVisible(manageButton);
    content->addAndMakeVisible(removeButton);
    
    manageButton->onClick = [this, userId] {
        showRoleManagementDialog(userId);
    };
    
    removeButton->onClick = [this, userId, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.removeUser(userId);
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 300);
    dialog->showModal();
}

void CollaborationUI::showInviteUserDialog() {
    inviteUserDialog = std::make_unique<juce::DialogWindow>(
        "Benutzer einladen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* emailLabel = new juce::Label("emailLabel", "E-Mail:");
    auto* emailEditor = new juce::TextEditor();
    auto* inviteButton = new juce::TextButton("Einladen");
    
    content->addAndMakeVisible(emailLabel);
    content->addAndMakeVisible(emailEditor);
    content->addAndMakeVisible(inviteButton);
    
    inviteButton->onClick = [this, emailEditor] {
        auto& manager = CollaborationManager::getInstance();
        manager.inviteUser(emailEditor->getText().toStdString());
        inviteUserDialog->exitModalState(0);
    };
    
    inviteUserDialog->setContentOwned(content, true);
    inviteUserDialog->centreWithSize(300, 150);
    inviteUserDialog->showModal();
}

void CollaborationUI::showRoleManagementDialog(const std::string& userId) {
    auto* dialog = new juce::DialogWindow(
        "Rollen verwalten",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* roleLabel = new juce::Label("roleLabel", "Neue Rolle:");
    auto* roleCombo = new juce::ComboBox();
    roleCombo->addItem("Leser", 1);
    roleCombo->addItem("Bearbeiter", 2);
    roleCombo->addItem("Administrator", 3);
    
    auto* saveButton = new juce::TextButton("Speichern");
    
    content->addAndMakeVisible(roleLabel);
    content->addAndMakeVisible(roleCombo);
    content->addAndMakeVisible(saveButton);
    
    saveButton->onClick = [this, userId, roleCombo, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.setUserRole(userId, static_cast<CollaborationManager::UserRole>(roleCombo->getSelectedId()));
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(300, 150);
    dialog->showModal();
}

void CollaborationUI::showChatWindow() {
    mainTabs->setCurrentTabIndex(2);
}

void CollaborationUI::showAudioChatWindow() {
    auto* dialog = new juce::DialogWindow(
        "Audio Chat",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* statusLabel = new juce::Label("statusLabel", "Status: Verbindung wird hergestellt...");
    auto* muteButton = new juce::TextButton("Stummschalten");
    auto* endButton = new juce::TextButton("Beenden");
    
    content->addAndMakeVisible(statusLabel);
    content->addAndMakeVisible(muteButton);
    content->addAndMakeVisible(endButton);
    
    muteButton->onClick = [muteButton, statusLabel] {
        static bool muted = false;
        muted = !muted;
        muteButton->setButtonText(muted ? "Stummschaltung aufheben" : "Stummschalten");
        statusLabel->setText("Status: " + std::string(muted ? "Stummgeschaltet" : "Aktiv"), 
                           juce::dontSendNotification);
    };
    
    endButton->onClick = [dialog] {
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 200);
    dialog->showModal();
}

void CollaborationUI::showVideoChatWindow() {
    auto* dialog = new juce::DialogWindow(
        "Video Chat",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* statusLabel = new juce::Label("statusLabel", "Status: Verbindung wird hergestellt...");
    auto* videoButton = new juce::TextButton("Video ausschalten");
    auto* audioButton = new juce::TextButton("Audio ausschalten");
    auto* endButton = new juce::TextButton("Beenden");
    
    content->addAndMakeVisible(statusLabel);
    content->addAndMakeVisible(videoButton);
    content->addAndMakeVisible(audioButton);
    content->addAndMakeVisible(endButton);
    
    videoButton->onClick = [videoButton, statusLabel] {
        static bool videoOff = false;
        videoOff = !videoOff;
        videoButton->setButtonText(videoOff ? "Video einschalten" : "Video ausschalten");
        statusLabel->setText("Status: " + std::string(videoOff ? "Video aus" : "Video an"), 
                           juce::dontSendNotification);
    };
    
    audioButton->onClick = [audioButton, statusLabel] {
        static bool audioOff = false;
        audioOff = !audioOff;
        audioButton->setButtonText(audioOff ? "Audio einschalten" : "Audio ausschalten");
        statusLabel->setText("Status: " + std::string(audioOff ? "Audio aus" : "Audio an"), 
                           juce::dontSendNotification);
    };
    
    endButton->onClick = [dialog] {
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 200);
    dialog->showModal();
}

void CollaborationUI::showVersionHistory() {
    mainTabs->setCurrentTabIndex(3);
    updateVersionList();
}

void CollaborationUI::showVersionComparison(const std::string& versionId1, const std::string& versionId2) {
    auto& manager = CollaborationManager::getInstance();
    auto version1 = manager.getVersion(versionId1);
    auto version2 = manager.getVersion(versionId2);
    
    auto* dialog = new juce::DialogWindow(
        "Versionsvergleich",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* version1Label = new juce::Label("version1Label", "Version 1: " + version1.name);
    auto* version2Label = new juce::Label("version2Label", "Version 2: " + version2.name);
    auto* changesLabel = new juce::Label("changesLabel", "Änderungen:");
    auto* changesList = new juce::TextEditor();
    changesList->setMultiLine(true);
    changesList->setReadOnly(true);
    
    auto* restoreButton = new juce::TextButton("Version wiederherstellen");
    auto* closeButton = new juce::TextButton("Schließen");
    
    content->addAndMakeVisible(version1Label);
    content->addAndMakeVisible(version2Label);
    content->addAndMakeVisible(changesLabel);
    content->addAndMakeVisible(changesList);
    content->addAndMakeVisible(restoreButton);
    content->addAndMakeVisible(closeButton);
    
    restoreButton->onClick = [this, versionId1, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.restoreVersion(versionId1);
        dialog->exitModalState(0);
    };
    
    closeButton->onClick = [dialog] {
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(600, 400);
    dialog->showModal();
}

void CollaborationUI::showCreateVersionDialog() {
    versionDialog = std::make_unique<juce::DialogWindow>(
        "Neue Version erstellen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* nameLabel = new juce::Label("nameLabel", "Versionsname:");
    auto* nameEditor = new juce::TextEditor();
    auto* createButton = new juce::TextButton("Erstellen");
    
    content->addAndMakeVisible(nameLabel);
    content->addAndMakeVisible(nameEditor);
    content->addAndMakeVisible(createButton);
    
    createButton->onClick = [this, nameEditor] {
        auto& manager = CollaborationManager::getInstance();
        manager.createVersion(manager.getCurrentProjectId(), nameEditor->getText().toStdString());
        versionDialog->exitModalState(0);
    };
    
    versionDialog->setContentOwned(content, true);
    versionDialog->centreWithSize(300, 150);
    versionDialog->showModal();
}

void CollaborationUI::showSyncStatus() {
    auto* dialog = new juce::DialogWindow(
        "Synchronisationsstatus",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* statusLabel = new juce::Label("statusLabel", "Status: Synchronisierung läuft...");
    auto* progressBar = new juce::ProgressBar(0.0);
    auto* detailsLabel = new juce::Label("detailsLabel", "Details:");
    auto* detailsList = new juce::TextEditor();
    detailsList->setMultiLine(true);
    detailsList->setReadOnly(true);
    
    content->addAndMakeVisible(statusLabel);
    content->addAndMakeVisible(progressBar);
    content->addAndMakeVisible(detailsLabel);
    content->addAndMakeVisible(detailsList);
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 300);
    dialog->showModal();
}

void CollaborationUI::showSyncSettings() {
    auto* dialog = new juce::DialogWindow(
        "Synchronisationseinstellungen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* autoSyncLabel = new juce::Label("autoSyncLabel", "Automatische Synchronisation:");
    auto* autoSyncToggle = new juce::ToggleButton("Aktiviert");
    auto* intervalLabel = new juce::Label("intervalLabel", "Synchronisationsintervall (Minuten):");
    auto* intervalEditor = new juce::TextEditor();
    auto* saveButton = new juce::TextButton("Speichern");
    
    content->addAndMakeVisible(autoSyncLabel);
    content->addAndMakeVisible(autoSyncToggle);
    content->addAndMakeVisible(intervalLabel);
    content->addAndMakeVisible(intervalEditor);
    content->addAndMakeVisible(saveButton);
    
    saveButton->onClick = [this, autoSyncToggle, intervalEditor, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.setAutoSync(autoSyncToggle->getToggleState());
        manager.setSyncInterval(std::stoi(intervalEditor->getText().toStdString()));
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(400, 200);
    dialog->showModal();
}

void CollaborationUI::showCloudStorageDialog() {
    auto* dialog = new juce::DialogWindow(
        "Cloud-Speicher",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* spaceLabel = new juce::Label("spaceLabel", "Verfügbarer Speicherplatz:");
    auto* spaceBar = new juce::ProgressBar(0.75);
    auto* filesLabel = new juce::Label("filesLabel", "Gespeicherte Dateien:");
    auto* filesList = new juce::ListBox();
    auto* uploadButton = new juce::TextButton("Datei hochladen");
    auto* downloadButton = new juce::TextButton("Datei herunterladen");
    
    content->addAndMakeVisible(spaceLabel);
    content->addAndMakeVisible(spaceBar);
    content->addAndMakeVisible(filesLabel);
    content->addAndMakeVisible(filesList);
    content->addAndMakeVisible(uploadButton);
    content->addAndMakeVisible(downloadButton);
    
    uploadButton->onClick = [this] {
        auto& manager = CollaborationManager::getInstance();
        manager.uploadToCloud();
    };
    
    downloadButton->onClick = [this] {
        auto& manager = CollaborationManager::getInstance();
        manager.downloadFromCloud();
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(500, 400);
    dialog->showModal();
}

void CollaborationUI::showPermissionManager() {
    auto* dialog = new juce::DialogWindow(
        "Berechtigungs-Manager",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* usersLabel = new juce::Label("usersLabel", "Benutzer:");
    auto* usersList = new juce::ListBox();
    auto* permissionsLabel = new juce::Label("permissionsLabel", "Berechtigungen:");
    auto* permissionsList = new juce::ListBox();
    auto* saveButton = new juce::TextButton("Speichern");
    
    content->addAndMakeVisible(usersLabel);
    content->addAndMakeVisible(usersList);
    content->addAndMakeVisible(permissionsLabel);
    content->addAndMakeVisible(permissionsList);
    content->addAndMakeVisible(saveButton);
    
    saveButton->onClick = [this, usersList, permissionsList, dialog] {
        auto& manager = CollaborationManager::getInstance();
        // Berechtigungen speichern
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(600, 400);
    dialog->showModal();
}

void CollaborationUI::showPermissionDialog(const std::string& userId) {
    auto* dialog = new juce::DialogWindow(
        "Berechtigungen",
        juce::Colours::lightgrey,
        true);
    
    auto* content = new juce::Component();
    auto* readToggle = new juce::ToggleButton("Lesen");
    auto* writeToggle = new juce::ToggleButton("Schreiben");
    auto* adminToggle = new juce::ToggleButton("Administrator");
    auto* saveButton = new juce::TextButton("Speichern");
    
    content->addAndMakeVisible(readToggle);
    content->addAndMakeVisible(writeToggle);
    content->addAndMakeVisible(adminToggle);
    content->addAndMakeVisible(saveButton);
    
    saveButton->onClick = [this, userId, readToggle, writeToggle, adminToggle, dialog] {
        auto& manager = CollaborationManager::getInstance();
        manager.setPermission(userId, "read", readToggle->getToggleState());
        manager.setPermission(userId, "write", writeToggle->getToggleState());
        manager.setPermission(userId, "admin", adminToggle->getToggleState());
        dialog->exitModalState(0);
    };
    
    dialog->setContentOwned(content, true);
    dialog->centreWithSize(300, 200);
    dialog->showModal();
}

// Event-Handler
void CollaborationUI::onProjectCreated(const CollaborationManager::CollaborationEvent& event) {
    updateProjectList();
    showNotification("Neues Projekt erstellt: " + event.projectId);
}

void CollaborationUI::onProjectJoined(const CollaborationManager::CollaborationEvent& event) {
    updateProjectList();
    showNotification("Projekt beigetreten: " + event.projectId);
}

void CollaborationUI::onProjectLeft(const CollaborationManager::CollaborationEvent& event) {
    updateProjectList();
    showNotification("Projekt verlassen: " + event.projectId);
}

void CollaborationUI::onUserInvited(const CollaborationManager::CollaborationEvent& event) {
    updateUserList();
    showNotification("Benutzer eingeladen: " + event.userId);
}

void CollaborationUI::onUserRemoved(const CollaborationManager::CollaborationEvent& event) {
    updateUserList();
    showNotification("Benutzer entfernt: " + event.userId);
}

void CollaborationUI::onMessageReceived(const CollaborationManager::CollaborationEvent& event) {
    chatWindow->insertTextAtCaret(event.data + "\n");
}

void CollaborationUI::onVersionCreated(const CollaborationManager::CollaborationEvent& event) {
    updateVersionList();
    showNotification("Neue Version erstellt: " + event.data);
}

void CollaborationUI::onSyncStarted(const CollaborationManager::CollaborationEvent& event) {
    syncStatus->setText("Synchronisierung läuft...", juce::dontSendNotification);
}

void CollaborationUI::onSyncCompleted(const CollaborationManager::CollaborationEvent& event) {
    syncStatus->setText("Synchronisiert", juce::dontSendNotification);
    showNotification("Synchronisierung abgeschlossen");
}

// Interne Hilfsfunktionen
void CollaborationUI::updateProjectList() {
    projectList->updateContent();
}

void CollaborationUI::updateUserList() {
    userList->updateContent();
}

void CollaborationUI::updateVersionList() {
    versionList->updateContent();
}

void CollaborationUI::updateSyncStatus() {
    auto& manager = CollaborationManager::getInstance();
    if (manager.isConnected()) {
        connectionStatus->setText("Verbunden", juce::dontSendNotification);
    } else {
        connectionStatus->setText("Nicht verbunden", juce::dontSendNotification);
    }
}

void CollaborationUI::showNotification(const std::string& message) {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Benachrichtigung",
        message);
}

void CollaborationUI::handleError(const std::string& error) {
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::WarningIcon,
        "Fehler",
        error);
}

// Callback-Funktionen
void CollaborationUI::onProjectSelected(int row) {
    auto& manager = CollaborationManager::getInstance();
    auto projects = manager.getProjects();
    if (row >= 0 && row < projects.size()) {
        showProjectDetails(projects[row].id);
    }
}

void CollaborationUI::onUserSelected(int row) {
    auto& manager = CollaborationManager::getInstance();
    auto users = manager.getUsers();
    if (row >= 0 && row < users.size()) {
        showUserDetails(users[row].id);
    }
}

void CollaborationUI::onVersionSelected(int row) {
    auto& manager = CollaborationManager::getInstance();
    auto versions = manager.getVersions(manager.getCurrentProjectId());
    if (row >= 0 && row < versions.size()) {
        if (row > 0) {
            showVersionComparison(versions[row-1].id, versions[row].id);
        }
    }
}

// ListBox-Model Implementierung
int CollaborationUI::getNumRows() {
    auto& manager = CollaborationManager::getInstance();
    if (mainTabs->getCurrentTabIndex() == 0) {
        return manager.getProjects().size();
    } else if (mainTabs->getCurrentTabIndex() == 1) {
        return manager.getUsers().size();
    } else if (mainTabs->getCurrentTabIndex() == 3) {
        return manager.getVersions(manager.getCurrentProjectId()).size();
    }
    return 0;
}

void CollaborationUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    auto& manager = CollaborationManager::getInstance();
    
    if (rowIsSelected) {
        g.fillAll(juce::Colours::lightblue);
    }
    
    g.setColour(juce::Colours::black);
    g.setFont(height * 0.7f);
    
    if (mainTabs->getCurrentTabIndex() == 0) {
        auto projects = manager.getProjects();
        if (rowNumber < projects.size()) {
            g.drawText(projects[rowNumber].name, 5, 0, width - 10, height, juce::Justification::centredLeft);
        }
    } else if (mainTabs->getCurrentTabIndex() == 1) {
        auto users = manager.getUsers();
        if (rowNumber < users.size()) {
            g.drawText(users[rowNumber].name, 5, 0, width - 10, height, juce::Justification::centredLeft);
        }
    } else if (mainTabs->getCurrentTabIndex() == 3) {
        auto versions = manager.getVersions(manager.getCurrentProjectId());
        if (rowNumber < versions.size()) {
            g.drawText(versions[rowNumber].name, 5, 0, width - 10, height, juce::Justification::centredLeft);
        }
    }
}

void CollaborationUI::onCreateProjectClicked() {
    showCreateProjectDialog();
}

void CollaborationUI::onInviteUserClicked() {
    showInviteUserDialog();
}

void CollaborationUI::onStartChatClicked() {
    showChatWindow();
}

void CollaborationUI::onSyncClicked() {
    auto& manager = CollaborationManager::getInstance();
    manager.syncProject(manager.getCurrentProjectId());
}

} // namespace VR_DAW 