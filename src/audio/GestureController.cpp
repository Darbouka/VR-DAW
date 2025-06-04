#include "GestureController.hpp"
#include <algorithm>
#include <thread>
#include <chrono>

namespace VR_DAW {

GestureController& GestureController::getInstance() {
    static GestureController instance;
    return instance;
}

void GestureController::initialize() {
    initializeComponents();
}

void GestureController::process() {
    processGestures();
}

void GestureController::optimize() {
    optimizeRecognition();
}

void GestureController::monitor() {
    monitorPerformance();
}

void GestureController::adjust() {
    adjustParameters();
}

void GestureController::finalize() {
    finalizeComponents();
}

// Gestenerkennung Implementierung
void GestureController::addGesture(const std::string& name, const std::string& pattern) {
    GestureData data;
    data.name = name;
    data.pattern = pattern;
    data.sensitivity = currentSensitivity;
    data.threshold = currentThreshold;
    data.smoothing = currentSmoothing;
    data.prediction = predictionEnabled;
    
    gestures[name] = data;
}

void GestureController::removeGesture(const std::string& name) {
    gestures.erase(name);
}

void GestureController::updateGesture(const std::string& name, const std::string& pattern) {
    if (gestures.find(name) != gestures.end()) {
        gestures[name].pattern = pattern;
    }
}

bool GestureController::isGestureRecognized(const std::string& name) const {
    return gestures.find(name) != gestures.end();
}

// Gestenverarbeitung Implementierung
void GestureController::setGestureCallback(const std::string& name, std::function<void()> callback) {
    if (gestures.find(name) != gestures.end()) {
        gestures[name].callback = callback;
    }
}

void GestureController::removeGestureCallback(const std::string& name) {
    if (gestures.find(name) != gestures.end()) {
        gestures[name].callback = nullptr;
    }
}

void GestureController::processGesture(const std::string& name) {
    if (gestures.find(name) != gestures.end() && gestures[name].callback) {
        gestures[name].callback();
    }
}

// Kalibrierung Implementierung
void GestureController::calibrate() {
    if (calibrator) {
        calibrator->calibrate();
    }
}

void GestureController::resetCalibration() {
    calibrationData.clear();
}

void GestureController::setCalibrationData(const std::vector<float>& data) {
    calibrationData = data;
}

std::vector<float> GestureController::getCalibrationData() const {
    return calibrationData;
}

// Parameter Implementierung
void GestureController::setSensitivity(float sensitivity) {
    currentSensitivity = sensitivity;
    for (auto& gesture : gestures) {
        gesture.second.sensitivity = sensitivity;
    }
}

void GestureController::setThreshold(float threshold) {
    currentThreshold = threshold;
    for (auto& gesture : gestures) {
        gesture.second.threshold = threshold;
    }
}

void GestureController::setSmoothing(float smoothing) {
    currentSmoothing = smoothing;
    for (auto& gesture : gestures) {
        gesture.second.smoothing = smoothing;
    }
}

void GestureController::setPrediction(bool enable) {
    predictionEnabled = enable;
    for (auto& gesture : gestures) {
        gesture.second.prediction = enable;
    }
}

// Private Hilfsfunktionen
void GestureController::initializeComponents() {
    recognizer = std::make_unique<GestureRecognizer>();
    patternMatcher = std::make_unique<GesturePatternMatcher>();
    predictor = std::make_unique<GesturePredictor>();
    calibrator = std::make_unique<GestureCalibrator>();
    
    processor = std::make_unique<GestureProcessor>();
    executor = std::make_unique<GestureExecutor>();
    queue = std::make_unique<GestureQueue>();
    scheduler = std::make_unique<GestureScheduler>();
    
    monitor = std::make_unique<GestureMonitor>();
    analyzer = std::make_unique<GestureAnalyzer>();
    logger = std::make_unique<GestureLogger>();
    reporter = std::make_unique<GestureReporter>();
}

void GestureController::processGestures() {
    if (recognizer && patternMatcher) {
        for (const auto& gesture : gestures) {
            if (patternMatcher->match(gesture.second.pattern)) {
                processGesture(gesture.first);
            }
        }
    }
}

void GestureController::optimizeRecognition() {
    if (recognizer) {
        recognizer->optimize();
    }
    if (patternMatcher) {
        patternMatcher->optimize();
    }
    if (predictor) {
        predictor->optimize();
    }
}

void GestureController::monitorPerformance() {
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

void GestureController::adjustParameters() {
    if (recognizer) {
        recognizer->setSensitivity(currentSensitivity);
        recognizer->setThreshold(currentThreshold);
    }
    if (predictor) {
        predictor->setSmoothing(currentSmoothing);
        predictor->setPrediction(predictionEnabled);
    }
}

void GestureController::finalizeComponents() {
    recognizer.reset();
    patternMatcher.reset();
    predictor.reset();
    calibrator.reset();
    
    processor.reset();
    executor.reset();
    queue.reset();
    scheduler.reset();
    
    monitor.reset();
    analyzer.reset();
    logger.reset();
    reporter.reset();
}

} // namespace VR_DAW 