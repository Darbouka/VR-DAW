#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace VR_DAW {

class PerformanceOptimizer {
public:
    static PerformanceOptimizer& getInstance();
    
    // Hauptfunktionen
    void optimize();
    void monitorPerformance();
    void adjustOptimization();
    void finalize();
    
    // Optimierungsfunktionen
    void optimizeGPU();
    void optimizeCPU();
    void optimizeMemory();
    void optimizeNetwork();
    void optimizeThreads();
    void optimizeCache();
    
private:
    PerformanceOptimizer() = default;
    ~PerformanceOptimizer() = default;
    
    PerformanceOptimizer(const PerformanceOptimizer&) = delete;
    PerformanceOptimizer& operator=(const PerformanceOptimizer&) = delete;
    
    // GPU-Komponenten
    std::unique_ptr<class GPUResources> gpuResources;
    std::unique_ptr<class ShaderOptimizer> shaderOptimizer;
    std::unique_ptr<class RenderOptimizer> renderOptimizer;
    
    // CPU-Komponenten
    std::unique_ptr<class CPULoadBalancer> cpuLoadBalancer;
    std::unique_ptr<class ProcessManager> processManager;
    std::unique_ptr<class ThreadScheduler> threadScheduler;
    
    // Speicher-Komponenten
    std::unique_ptr<class MemoryManager> memoryManager;
    std::unique_ptr<class MemoryPool> memoryPool;
    std::unique_ptr<class GarbageCollector> garbageCollector;
    
    // Netzwerk-Komponenten
    std::unique_ptr<class NetworkProtocol> networkProtocol;
    std::unique_ptr<class DataTransfer> dataTransfer;
    std::unique_ptr<class ConnectionManager> connectionManager;
    
    // Thread-Komponenten
    std::unique_ptr<class ThreadPool> threadPool;
    std::unique_ptr<class ThreadSynchronizer> threadSynchronizer;
    std::unique_ptr<class ThreadPrioritizer> threadPrioritizer;
    
    // Cache-Komponenten
    std::unique_ptr<class CacheStrategy> cacheStrategy;
    std::unique_ptr<class CacheInvalidator> cacheInvalidator;
    std::unique_ptr<class CachePrefetcher> cachePrefetcher;
    
    // Monitoring-Komponenten
    std::unique_ptr<class GPUMonitor> gpuMonitor;
    std::unique_ptr<class CPUMonitor> cpuMonitor;
    std::unique_ptr<class MemoryMonitor> memoryMonitor;
    std::unique_ptr<class NetworkMonitor> networkMonitor;
    
    // Metriken
    std::unique_ptr<class PerformanceMetrics> performanceMetrics;
};

} // namespace VR_DAW 