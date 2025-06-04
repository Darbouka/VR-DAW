#include "HapticController.hpp"
#include <algorithm>
#include <thread>
#include <chrono>

namespace VR_DAW {

HapticController& HapticController::getInstance() {
    static HapticController instance;
    return instance;
}

void HapticController::initialize() {
    initializeComponents();
}

void HapticController::process() {
    processHapticData();
}

void HapticController::optimize() {
    optimizeFeedback();
}

void HapticController::monitor() {
    monitorPerformance();
}

void HapticController::adjust() {
    adjustParameters();
}

void HapticController::finalize() {
    finalizeComponents();
}

// Haptisches Feedback Implementierung
void HapticController::triggerFeedback(const std::string& event) {
    if (isFeedbackActive && !isPaused) {
        HapticData data;
        data.event = event;
        data.intensity = currentIntensity;
        data.duration = currentDuration;
        data.frequency = currentFrequency;
        data.waveform = currentWaveform;
        
        if (patterns.find(event) != patterns.end()) {
            data.pattern = patterns[event];
        }
        
        if (processor) {
            processor->process(data);
        }
    }
}

void HapticController::stopFeedback() {
    isFeedbackActive = false;
    isPaused = false;
}

void HapticController::pauseFeedback() {
    if (isFeedbackActive) {
        isPaused = true;
    }
}

void HapticController::resumeFeedback() {
    if (isFeedbackActive) {
        isPaused = false;
    }
}

bool HapticController::isFeedbackActive() const {
    return isFeedbackActive && !isPaused;
}

// Muster-Generierung Implementierung
void HapticController::addPattern(const std::string& name, const std::vector<float>& pattern) {
    patterns[name] = pattern;
}

void HapticController::removePattern(const std::string& name) {
    patterns.erase(name);
}

void HapticController::updatePattern(const std::string& name, const std::vector<float>& pattern) {
    if (patterns.find(name) != patterns.end()) {
        patterns[name] = pattern;
    }
}

std::vector<float> HapticController::getPattern(const std::string& name) const {
    if (patterns.find(name) != patterns.end()) {
        return patterns[name];
    }
    return std::vector<float>();
}

// Parameter Implementierung
void HapticController::setIntensity(float intensity) {
    parameters.intensity = intensity;
    currentIntensity = intensity;
}

void HapticController::setDuration(float duration) {
    parameters.duration = duration;
    currentDuration = duration;
}

void HapticController::setFrequency(float frequency) {
    parameters.frequency = frequency;
    currentFrequency = frequency;
}

void HapticController::setWaveform(const std::string& waveform) {
    parameters.waveform = waveform;
    currentWaveform = waveform;
}

// Private Hilfsfunktionen
void HapticController::initializeComponents() {
    patternGenerator = std::make_unique<HapticPatternGenerator>();
    waveformGenerator = std::make_unique<HapticWaveformGenerator>();
    scheduler = std::make_unique<HapticScheduler>();
    calibrator = std::make_unique<HapticCalibrator>();
    
    processor = std::make_unique<HapticProcessor>();
    executor = std::make_unique<HapticExecutor>();
    queue = std::make_unique<HapticQueue>();
    manager = std::make_unique<HapticManager>();
    
    monitor = std::make_unique<HapticMonitor>();
    analyzer = std::make_unique<HapticAnalyzer>();
    logger = std::make_unique<HapticLogger>();
    reporter = std::make_unique<HapticReporter>();
    
    isFeedbackActive = true;
    isPaused = false;
}

void HapticController::processHapticData() {
    if (isFeedbackActive && !isPaused && processor) {
        // Generiere Wellenform
        if (waveformGenerator) {
            std::vector<float> waveform = waveformGenerator->generate(
                currentWaveform,
                currentFrequency,
                currentDuration
            );
            
            // Wende Intensität an
            for (float& sample : waveform) {
                sample *= currentIntensity;
            }
            
            // Verarbeite die Daten
            HapticData data;
            data.pattern = waveform;
            data.intensity = currentIntensity;
            data.duration = currentDuration;
            data.frequency = currentFrequency;
            data.waveform = currentWaveform;
            
            processor->process(data);
        }
    }
}

void HapticController::optimizeFeedback() {
    if (patternGenerator) {
        patternGenerator->optimize();
    }
    if (waveformGenerator) {
        waveformGenerator->optimize();
    }
    if (scheduler) {
        scheduler->optimize();
    }
}

void HapticController::monitorPerformance() {
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

void HapticController::adjustParameters() {
    if (patternGenerator) {
        patternGenerator->setIntensity(parameters.intensity);
        patternGenerator->setDuration(parameters.duration);
    }
    if (waveformGenerator) {
        waveformGenerator->setFrequency(parameters.frequency);
        waveformGenerator->setWaveform(parameters.waveform);
    }
}

void HapticController::finalizeComponents() {
    patternGenerator.reset();
    waveformGenerator.reset();
    scheduler.reset();
    calibrator.reset();
    
    processor.reset();
    executor.reset();
    queue.reset();
    manager.reset();
    
    monitor.reset();
    analyzer.reset();
    logger.reset();
    reporter.reset();
}

} // namespace VR_DAW 