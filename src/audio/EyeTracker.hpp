#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace VR_DAW {

class EyeTracker {
public:
    static EyeTracker& getInstance();

    // Grundlegende Funktionen
    void initialize();
    void process();
    void optimize();
    void monitor();
    void adjust();
    void finalize();

    // Augenverfolgung
    void startTracking();
    void stopTracking();
    void pauseTracking();
    void resumeTracking();
    bool isTracking() const;

    // Kalibrierung
    void calibrate();
    void resetCalibration();
    void setCalibrationData(const std::vector<float>& data);
    std::vector<float> getCalibrationData() const;

    // Blickverfolgung
    void setGazeCallback(std::function<void(float, float)> callback);
    void removeGazeCallback();
    void processGaze(float x, float y);

    // Parameter
    void setSensitivity(float sensitivity);
    void setSmoothing(float smoothing);
    void setPrediction(bool enable);
    void setFiltering(bool enable);

private:
    EyeTracker() = default;
    ~EyeTracker() = default;

    EyeTracker(const EyeTracker&) = delete;
    EyeTracker& operator=(const EyeTracker&) = delete;

    // Augenverfolgung Komponenten
    std::unique_ptr<class EyeCalibrator> calibrator;
    std::unique_ptr<class EyePredictor> predictor;
    std::unique_ptr<class EyeFilter> filter;
    std::unique_ptr<class EyeSmoother> smoother;

    // Verarbeitungskomponenten
    std::unique_ptr<class EyeProcessor> processor;
    std::unique_ptr<class EyeExecutor> executor;
    std::unique_ptr<class EyeQueue> queue;
    std::unique_ptr<class EyeScheduler> scheduler;

    // Monitoring Komponenten
    std::unique_ptr<class EyeMonitor> monitor;
    std::unique_ptr<class EyeAnalyzer> analyzer;
    std::unique_ptr<class EyeLogger> logger;
    std::unique_ptr<class EyeReporter> reporter;

    // Interne Hilfsfunktionen
    void initializeComponents();
    void processEyeData();
    void optimizeTracking();
    void monitorPerformance();
    void adjustParameters();
    void finalizeComponents();

    // Augenverfolgungs-Datenstrukturen
    struct EyeData {
        float x;
        float y;
        float confidence;
        float timestamp;
    };

    struct TrackingParameters {
        float sensitivity;
        float smoothing;
        bool prediction;
        bool filtering;
    };

    std::vector<float> calibrationData;
    std::function<void(float, float)> gazeCallback;
    TrackingParameters parameters;
    bool isTrackingActive = false;
    bool isPaused = false;
    float currentSensitivity = 1.0f;
    float currentSmoothing = 0.3f;
    bool predictionEnabled = true;
    bool filteringEnabled = true;
};

} // namespace VR_DAW 