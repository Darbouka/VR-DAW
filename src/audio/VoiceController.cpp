#include "VoiceController.hpp"
#include <algorithm>
#include <thread>
#include <chrono>

namespace VR_DAW {

VoiceController& VoiceController::getInstance() {
    static VoiceController instance;
    return instance;
}

void VoiceController::initialize() {
    initializeComponents();
}

void VoiceController::process() {
    processVoice();
}

void VoiceController::optimize() {
    optimizeRecognition();
}

void VoiceController::monitor() {
    monitorPerformance();
}

void VoiceController::adjust() {
    adjustParameters();
}

void VoiceController::finalize() {
    finalizeComponents();
}

// Spracherkennung Implementierung
void VoiceController::addCommand(const std::string& command, const std::string& action) {
    VoiceData data;
    data.command = command;
    data.action = action;
    data.sensitivity = currentSensitivity;
    data.confidenceThreshold = currentConfidenceThreshold;
    data.maxAlternatives = currentMaxAlternatives;
    data.continuousMode = continuousModeEnabled;
    
    commands[command] = data;
}

void VoiceController::removeCommand(const std::string& command) {
    commands.erase(command);
}

void VoiceController::updateCommand(const std::string& command, const std::string& action) {
    if (commands.find(command) != commands.end()) {
        commands[command].action = action;
    }
}

bool VoiceController::isCommandRecognized(const std::string& command) const {
    return commands.find(command) != commands.end();
}

// Sprachverarbeitung Implementierung
void VoiceController::setCommandCallback(const std::string& command, std::function<void()> callback) {
    if (commands.find(command) != commands.end()) {
        commands[command].callback = callback;
    }
}

void VoiceController::removeCommandCallback(const std::string& command) {
    if (commands.find(command) != commands.end()) {
        commands[command].callback = nullptr;
    }
}

void VoiceController::processCommand(const std::string& command) {
    if (commands.find(command) != commands.end() && commands[command].callback) {
        commands[command].callback();
    }
}

// Spracheinstellungen Implementierung
void VoiceController::setLanguage(const std::string& language) {
    currentLanguage = language;
    if (model) {
        model->setLanguage(language);
    }
}

void VoiceController::setVoiceModel(const std::string& modelName) {
    currentModel = modelName;
    if (model) {
        model->load(modelName);
    }
}

void VoiceController::setNoiseReduction(bool enable) {
    noiseReductionEnabled = enable;
    if (recognizer) {
        recognizer->setNoiseReduction(enable);
    }
}

void VoiceController::setAutoGainControl(bool enable) {
    autoGainControlEnabled = enable;
    if (recognizer) {
        recognizer->setAutoGainControl(enable);
    }
}

// Parameter Implementierung
void VoiceController::setSensitivity(float sensitivity) {
    currentSensitivity = sensitivity;
    for (auto& command : commands) {
        command.second.sensitivity = sensitivity;
    }
}

void VoiceController::setConfidenceThreshold(float threshold) {
    currentConfidenceThreshold = threshold;
    for (auto& command : commands) {
        command.second.confidenceThreshold = threshold;
    }
}

void VoiceController::setMaxAlternatives(int alternatives) {
    currentMaxAlternatives = alternatives;
    for (auto& command : commands) {
        command.second.maxAlternatives = alternatives;
    }
}

void VoiceController::setContinuousMode(bool enable) {
    continuousModeEnabled = enable;
    for (auto& command : commands) {
        command.second.continuousMode = enable;
    }
}

// Private Hilfsfunktionen
void VoiceController::initializeComponents() {
    recognizer = std::make_unique<VoiceRecognizer>();
    model = std::make_unique<VoiceModel>();
    predictor = std::make_unique<VoicePredictor>();
    calibrator = std::make_unique<VoiceCalibrator>();
    
    processor = std::make_unique<VoiceProcessor>();
    executor = std::make_unique<VoiceExecutor>();
    queue = std::make_unique<VoiceQueue>();
    scheduler = std::make_unique<VoiceScheduler>();
    
    monitor = std::make_unique<VoiceMonitor>();
    analyzer = std::make_unique<VoiceAnalyzer>();
    logger = std::make_unique<VoiceLogger>();
    reporter = std::make_unique<VoiceReporter>();
    
    // Initialisiere Standardeinstellungen
    model->setLanguage(currentLanguage);
    model->load(currentModel);
    recognizer->setNoiseReduction(noiseReductionEnabled);
    recognizer->setAutoGainControl(autoGainControlEnabled);
}

void VoiceController::processVoice() {
    if (recognizer && model) {
        for (const auto& command : commands) {
            if (recognizer->recognize(command.second.command)) {
                processCommand(command.first);
            }
        }
    }
}

void VoiceController::optimizeRecognition() {
    if (recognizer) {
        recognizer->optimize();
    }
    if (model) {
        model->optimize();
    }
    if (predictor) {
        predictor->optimize();
    }
}

void VoiceController::monitorPerformance() {
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

void VoiceController::adjustParameters() {
    if (recognizer) {
        recognizer->setSensitivity(currentSensitivity);
        recognizer->setConfidenceThreshold(currentConfidenceThreshold);
        recognizer->setMaxAlternatives(currentMaxAlternatives);
        recognizer->setContinuousMode(continuousModeEnabled);
    }
}

void VoiceController::finalizeComponents() {
    recognizer.reset();
    model.reset();
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