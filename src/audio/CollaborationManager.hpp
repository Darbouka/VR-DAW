#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace VR_DAW {

class CollaborationManager {
public:
    static CollaborationManager& getInstance();
    
    // Hauptfunktionen
    void initialize();
    void enhance();
    void handleCollaboration();
    void manageResources();
    void handleConflicts();
    void monitorCollaboration();
    void optimizeCollaboration();
    void finalize();
    
private:
    CollaborationManager() = default;
    ~CollaborationManager() = default;
    
    CollaborationManager(const CollaborationManager&) = delete;
    CollaborationManager& operator=(const CollaborationManager&) = delete;
    
    // Kollaborations-Komponenten
    std::unique_ptr<class RealTimeCollaboration> realTimeCollaboration;
    std::unique_ptr<class SessionSharing> sessionSharing;
    std::unique_ptr<class RemoteControl> remoteControl;
    std::unique_ptr<class VersionControl> versionControl;
    
    // Ressourcen-Management
    std::unique_ptr<class ResourceManager> resourceManager;
    std::unique_ptr<class ConnectionManager> connectionManager;
    std::unique_ptr<class SynchronizationManager> synchronizationManager;
    
    // Konflikt-Lösung
    std::unique_ptr<class ConflictResolver> conflictResolver;
    std::unique_ptr<class MergeManager> mergeManager;
    std::unique_ptr<class VersionResolver> versionResolver;
    
    // Monitoring
    std::unique_ptr<class CollaborationMonitor> collaborationMonitor;
    std::unique_ptr<class PerformanceMonitor> performanceMonitor;
    std::unique_ptr<class ResourceMonitor> resourceMonitor;
    
    // Optimierung
    std::unique_ptr<class CollaborationOptimizer> collaborationOptimizer;
    std::unique_ptr<class PerformanceOptimizer> performanceOptimizer;
    std::unique_ptr<class ResourceOptimizer> resourceOptimizer;
};

} // namespace VR_DAW 