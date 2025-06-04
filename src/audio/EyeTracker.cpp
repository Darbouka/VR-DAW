#include "EyeTracker.hpp"
#include <algorithm>
#include <thread>
#include <chrono>

namespace VR_DAW {

EyeTracker& EyeTracker::getInstance() {
    static EyeTracker instance;
    return instance;
}

void EyeTracker::initialize() {
    initializeComponents();
}

void EyeTracker::process() {
    processEyeData();
}

void EyeTracker::optimize() {
    optimizeTracking();
}

void EyeTracker::monitor() {
    monitorPerformance();
}

void EyeTracker::adjust() {
    adjustParameters();
}

void EyeTracker::finalize() {
    finalizeComponents();
}

// Augenverfolgung Implementierung
void EyeTracker::startTracking() {
    isTrackingActive = true;
    isPaused = false;
}

void EyeTracker::stopTracking() {
    isTrackingActive = false;
    isPaused = false;
}

void EyeTracker::pauseTracking() {
    if (isTrackingActive) {
        isPaused = true;
    }
}

void EyeTracker::resumeTracking() {
    if (isTrackingActive) {
        isPaused = false;
    }
}

bool EyeTracker::isTracking() const {
    return isTrackingActive && !isPaused;
}

// Kalibrierung Implementierung
void EyeTracker::calibrate() {
    if (calibrator) {
        calibrator->calibrate();
    }
}

void EyeTracker::resetCalibration() {
    calibrationData.clear();
}

void EyeTracker::setCalibrationData(const std::vector<float>& data) {
    calibrationData = data;
}

std::vector<float> EyeTracker::getCalibrationData() const {
    return calibrationData;
}

// Blickverfolgung Implementierung
void EyeTracker::setGazeCallback(std::function<void(float, float)> callback) {
    gazeCallback = callback;
}

void EyeTracker::removeGazeCallback() {
    gazeCallback = nullptr;
}

void EyeTracker::processGaze(float x, float y) {
    if (gazeCallback && isTracking()) {
        // Wende Kalibrierung an
        if (!calibrationData.empty()) {
            x = applyCalibration(x, calibrationData);
            y = applyCalibration(y, calibrationData);
        }
        
        // Wende Glättung an
        if (parameters.smoothing > 0.0f) {
            x = smoothValue(x, parameters.smoothing);
            y = smoothValue(y, parameters.smoothing);
        }
        
        // Wende Filterung an
        if (parameters.filtering) {
            x = filterValue(x);
            y = filterValue(y);
        }
        
        gazeCallback(x, y);
    }
}

// Parameter Implementierung
void EyeTracker::setSensitivity(float sensitivity) {
    parameters.sensitivity = sensitivity;
    currentSensitivity = sensitivity;
}

void EyeTracker::setSmoothing(float smoothing) {
    parameters.smoothing = smoothing;
    currentSmoothing = smoothing;
}

void EyeTracker::setPrediction(bool enable) {
    parameters.prediction = enable;
    predictionEnabled = enable;
}

void EyeTracker::setFiltering(bool enable) {
    parameters.filtering = enable;
    filteringEnabled = enable;
}

// Private Hilfsfunktionen
void EyeTracker::initializeComponents() {
    calibrator = std::make_unique<EyeCalibrator>();
    predictor = std::make_unique<EyePredictor>();
    filter = std::make_unique<EyeFilter>();
    smoother = std::make_unique<EyeSmoother>();
    
    processor = std::make_unique<EyeProcessor>();
    executor = std::make_unique<EyeExecutor>();
    queue = std::make_unique<EyeQueue>();
    scheduler = std::make_unique<EyeScheduler>();
    
    monitor = std::make_unique<EyeMonitor>();
    analyzer = std::make_unique<EyeAnalyzer>();
    logger = std::make_unique<EyeLogger>();
    reporter = std::make_unique<EyeReporter>();
}

void EyeTracker::processEyeData() {
    if (isTracking() && processor) {
        EyeData data;
        // Hole aktuelle Augenposition
        if (predictor) {
            data = predictor->predict();
        }
        
        // Verarbeite die Daten
        if (processor) {
            processor->process(data);
        }
        
        // Aktualisiere die Position
        processGaze(data.x, data.y);
    }
}

void EyeTracker::optimizeTracking() {
    if (predictor) {
        predictor->optimize();
    }
    if (filter) {
        filter->optimize();
    }
    if (smoother) {
        smoother->optimize();
    }
}

void EyeTracker::monitorPerformance() {
    if (monitor) {
        monitor->monitor();
    }
    if (analyzer) {
        analyzer->analyze();
    }
    if (logger) {
        logger->log();
    }
    if (reporter) {
        reporter->report();
    }
}

void EyeTracker::adjustParameters() {
    if (predictor) {
        predictor->setSensitivity(parameters.sensitivity);
        predictor->setPrediction(parameters.prediction);
    }
    if (smoother) {
        smoother->setSmoothing(parameters.smoothing);
    }
    if (filter) {
        filter->setEnabled(parameters.filtering);
    }
}

void EyeTracker::finalizeComponents() {
    calibrator.reset();
    predictor.reset();
    filter.reset();
    smoother.reset();
    
    processor.reset();
    executor.reset();
    queue.reset();
    scheduler.reset();
    
    monitor.reset();
    analyzer.reset();
    logger.reset();
    reporter.reset();
}

float EyeTracker::applyCalibration(float value, const std::vector<float>& calibration) {
    // Einfache lineare Kalibrierung
    if (calibration.size() >= 2) {
        return value * calibration[0] + calibration[1];
    }
    return value;
}

float EyeTracker::smoothValue(float value, float smoothing) {
    // Exponentieller gleitender Durchschnitt
    static float lastValue = value;
    lastValue = lastValue * (1.0f - smoothing) + value * smoothing;
    return lastValue;
}

float EyeTracker::filterValue(float value) {
    // Einfacher Tiefpassfilter
    static float lastValue = value;
    static const float alpha = 0.1f;
    lastValue = lastValue * (1.0f - alpha) + value * alpha;
    return lastValue;
}

} // namespace VR_DAW 