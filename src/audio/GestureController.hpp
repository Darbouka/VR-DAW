#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace VR_DAW {

class GestureController {
public:
    static GestureController& getInstance();

    // Grundlegende Funktionen
    void initialize();
    void process();
    void optimize();
    void monitor();
    void adjust();
    void finalize();

    // Gestenerkennung
    void addGesture(const std::string& name, const std::string& pattern);
    void removeGesture(const std::string& name);
    void updateGesture(const std::string& name, const std::string& pattern);
    bool isGestureRecognized(const std::string& name) const;

    // Gestenverarbeitung
    void setGestureCallback(const std::string& name, std::function<void()> callback);
    void removeGestureCallback(const std::string& name);
    void processGesture(const std::string& name);

    // Kalibrierung
    void calibrate();
    void resetCalibration();
    void setCalibrationData(const std::vector<float>& data);
    std::vector<float> getCalibrationData() const;

    // Parameter
    void setSensitivity(float sensitivity);
    void setThreshold(float threshold);
    void setSmoothing(float smoothing);
    void setPrediction(bool enable);

private:
    GestureController() = default;
    ~GestureController() = default;

    GestureController(const GestureController&) = delete;
    GestureController& operator=(const GestureController&) = delete;

    // Gestenerkennung Komponenten
    std::unique_ptr<class GestureRecognizer> recognizer;
    std::unique_ptr<class GesturePatternMatcher> patternMatcher;
    std::unique_ptr<class GesturePredictor> predictor;
    std::unique_ptr<class GestureCalibrator> calibrator;

    // Gestenverarbeitung Komponenten
    std::unique_ptr<class GestureProcessor> processor;
    std::unique_ptr<class GestureExecutor> executor;
    std::unique_ptr<class GestureQueue> queue;
    std::unique_ptr<class GestureScheduler> scheduler;

    // Monitoring Komponenten
    std::unique_ptr<class GestureMonitor> monitor;
    std::unique_ptr<class GestureAnalyzer> analyzer;
    std::unique_ptr<class GestureLogger> logger;
    std::unique_ptr<class GestureReporter> reporter;

    // Interne Hilfsfunktionen
    void initializeComponents();
    void processGestures();
    void optimizeRecognition();
    void monitorPerformance();
    void adjustParameters();
    void finalizeComponents();

    // Gesten-Datenstrukturen
    struct GestureData {
        std::string name;
        std::string pattern;
        std::function<void()> callback;
        float sensitivity;
        float threshold;
        float smoothing;
        bool prediction;
    };

    std::map<std::string, GestureData> gestures;
    std::vector<float> calibrationData;
    float currentSensitivity = 1.0f;
    float currentThreshold = 0.5f;
    float currentSmoothing = 0.3f;
    bool predictionEnabled = true;
};

} // namespace VR_DAW 