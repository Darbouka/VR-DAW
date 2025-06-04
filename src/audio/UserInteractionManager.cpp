#include "UserInteractionManager.hpp"
#include "GestureController.hpp"
#include "VoiceController.hpp"
#include "EyeTracker.hpp"
#include "HapticController.hpp"
#include <algorithm>
#include <thread>
#include <chrono>

namespace VR_DAW {

UserInteractionManager& UserInteractionManager::getInstance() {
    static UserInteractionManager instance;
    return instance;
}

void UserInteractionManager::initialize() {
    // Initialisiere Gestensteuerung
    initializeGestureControl();
    
    // Initialisiere Sprachsteuerung
    initializeVoiceControl();
    
    // Initialisiere Augenverfolgung
    initializeEyeTracking();
    
    // Initialisiere Haptisches Feedback
    initializeHapticFeedback();
}

void UserInteractionManager::improve() {
    // Verbessere Gestensteuerung
    improveGestureControl();
    
    // Verbessere Sprachsteuerung
    improveVoiceControl();
    
    // Verbessere Augenverfolgung
    improveEyeTracking();
    
    // Verbessere Haptisches Feedback
    improveHapticFeedback();
}

void UserInteractionManager::handleInteraction() {
    // Verarbeite Gesten
    processGestures();
    
    // Verarbeite Sprache
    processVoice();
    
    // Verarbeite Augenverfolgung
    processEyeTracking();
    
    // Verarbeite Haptisches Feedback
    processHapticFeedback();
}

void UserInteractionManager::optimizeInteraction() {
    // Optimiere Gestensteuerung
    optimizeGestureControl();
    
    // Optimiere Sprachsteuerung
    optimizeVoiceControl();
    
    // Optimiere Augenverfolgung
    optimizeEyeTracking();
    
    // Optimiere Haptisches Feedback
    optimizeHapticFeedback();
}

void UserInteractionManager::monitorInteraction() {
    // Überwache Gestensteuerung
    monitorGestureControl();
    
    // Überwache Sprachsteuerung
    monitorVoiceControl();
    
    // Überwache Augenverfolgung
    monitorEyeTracking();
    
    // Überwache Haptisches Feedback
    monitorHapticFeedback();
}

void UserInteractionManager::adjustInteraction() {
    // Passe Gestensteuerung an
    adjustGestureControl();
    
    // Passe Sprachsteuerung an
    adjustVoiceControl();
    
    // Passe Augenverfolgung an
    adjustEyeTracking();
    
    // Passe Haptisches Feedback an
    adjustHapticFeedback();
}

void UserInteractionManager::finalize() {
    // Finalisiere Gestensteuerung
    finalizeGestureControl();
    
    // Finalisiere Sprachsteuerung
    finalizeVoiceControl();
    
    // Finalisiere Augenverfolgung
    finalizeEyeTracking();
    
    // Finalisiere Haptisches Feedback
    finalizeHapticFeedback();
}

// Gestensteuerung Implementierung
void UserInteractionManager::setGestureControlEnabled(bool enable) {
    if (gestureController) {
        if (enable) {
            gestureController->initialize();
        } else {
            gestureController->finalize();
        }
    }
}

void UserInteractionManager::setGestureSensitivity(float sensitivity) {
    if (gestureController) {
        gestureController->setSensitivity(sensitivity);
    }
}

void UserInteractionManager::addGesture(const std::string& name, const std::string& pattern) {
    if (gestureController) {
        gestureController->addGesture(name, pattern);
    }
}

void UserInteractionManager::removeGesture(const std::string& name) {
    if (gestureController) {
        gestureController->removeGesture(name);
    }
}

// Sprachsteuerung Implementierung
void UserInteractionManager::setVoiceControlEnabled(bool enable) {
    if (voiceController) {
        if (enable) {
            voiceController->initialize();
        } else {
            voiceController->finalize();
        }
    }
}

void UserInteractionManager::setVoiceRecognitionLanguage(const std::string& language) {
    if (voiceController) {
        voiceController->setLanguage(language);
    }
}

void UserInteractionManager::addVoiceCommand(const std::string& command, const std::string& action) {
    if (voiceController) {
        voiceController->addCommand(command, action);
    }
}

void UserInteractionManager::removeVoiceCommand(const std::string& command) {
    if (voiceController) {
        voiceController->removeCommand(command);
    }
}

// Augenverfolgung Implementierung
void UserInteractionManager::setEyeTrackingEnabled(bool enable) {
    if (eyeTracker) {
        if (enable) {
            eyeTracker->startTracking();
        } else {
            eyeTracker->stopTracking();
        }
    }
}

void UserInteractionManager::setEyeTrackingSensitivity(float sensitivity) {
    if (eyeTracker) {
        eyeTracker->setSensitivity(sensitivity);
    }
}

void UserInteractionManager::setEyeTrackingCalibration(const std::vector<float>& calibration) {
    if (eyeTracker) {
        eyeTracker->setCalibrationData(calibration);
    }
}

void UserInteractionManager::resetEyeTrackingCalibration() {
    if (eyeTracker) {
        eyeTracker->resetCalibration();
    }
}

// Haptisches Feedback Implementierung
void UserInteractionManager::setHapticFeedbackEnabled(bool enable) {
    if (hapticController) {
        if (enable) {
            hapticController->initialize();
        } else {
            hapticController->finalize();
        }
    }
}

void UserInteractionManager::setHapticIntensity(float intensity) {
    if (hapticController) {
        hapticController->setIntensity(intensity);
    }
}

void UserInteractionManager::setHapticPattern(const std::string& pattern) {
    if (hapticController) {
        std::vector<float> patternData = {1.0f, 0.5f, 0.0f}; // Beispiel-Pattern
        hapticController->addPattern(pattern, patternData);
    }
}

void UserInteractionManager::triggerHapticFeedback(const std::string& event) {
    if (hapticController) {
        hapticController->triggerFeedback(event);
    }
}

// Private Hilfsfunktionen
void UserInteractionManager::initializeGestureControl() {
    gestureController = std::make_unique<GestureController>();
    gestureController->initialize();
}

void UserInteractionManager::initializeVoiceControl() {
    voiceController = std::make_unique<VoiceController>();
    voiceController->initialize();
}

void UserInteractionManager::initializeEyeTracking() {
    eyeTracker = std::make_unique<EyeTracker>();
    eyeTracker->initialize();
}

void UserInteractionManager::initializeHapticFeedback() {
    hapticController = std::make_unique<HapticController>();
    hapticController->initialize();
}

void UserInteractionManager::improveGestureControl() {
    if (gestureController) {
        gestureController->optimize();
    }
}

void UserInteractionManager::improveVoiceControl() {
    if (voiceController) {
        voiceController->optimize();
    }
}

void UserInteractionManager::improveEyeTracking() {
    if (eyeTracker) {
        eyeTracker->optimize();
    }
}

void UserInteractionManager::improveHapticFeedback() {
    if (hapticController) {
        hapticController->optimize();
    }
}

void UserInteractionManager::processGestures() {
    if (gestureController) {
        gestureController->process();
    }
}

void UserInteractionManager::processVoice() {
    if (voiceController) {
        voiceController->process();
    }
}

void UserInteractionManager::processEyeTracking() {
    if (eyeTracker) {
        eyeTracker->process();
    }
}

void UserInteractionManager::processHapticFeedback() {
    if (hapticController) {
        hapticController->process();
    }
}

void UserInteractionManager::optimizeGestureControl() {
    if (gestureController) {
        gestureController->optimize();
    }
}

void UserInteractionManager::optimizeVoiceControl() {
    if (voiceController) {
        voiceController->optimize();
    }
}

void UserInteractionManager::optimizeEyeTracking() {
    if (eyeTracker) {
        eyeTracker->optimize();
    }
}

void UserInteractionManager::optimizeHapticFeedback() {
    if (hapticController) {
        hapticController->optimize();
    }
}

void UserInteractionManager::monitorGestureControl() {
    if (gestureController) {
        gestureController->monitor();
    }
}

void UserInteractionManager::monitorVoiceControl() {
    if (voiceController) {
        voiceController->monitor();
    }
}

void UserInteractionManager::monitorEyeTracking() {
    if (eyeTracker) {
        eyeTracker->monitor();
    }
}

void UserInteractionManager::monitorHapticFeedback() {
    if (hapticController) {
        hapticController->monitor();
    }
}

void UserInteractionManager::adjustGestureControl() {
    if (gestureController) {
        gestureController->adjust();
    }
}

void UserInteractionManager::adjustVoiceControl() {
    if (voiceController) {
        voiceController->adjust();
    }
}

void UserInteractionManager::adjustEyeTracking() {
    if (eyeTracker) {
        eyeTracker->adjust();
    }
}

void UserInteractionManager::adjustHapticFeedback() {
    if (hapticController) {
        hapticController->adjust();
    }
}

void UserInteractionManager::finalizeGestureControl() {
    if (gestureController) {
        gestureController->finalize();
    }
}

void UserInteractionManager::finalizeVoiceControl() {
    if (voiceController) {
        voiceController->finalize();
    }
}

void UserInteractionManager::finalizeEyeTracking() {
    if (eyeTracker) {
        eyeTracker->finalize();
    }
}

void UserInteractionManager::finalizeHapticFeedback() {
    if (hapticController) {
        hapticController->finalize();
    }
}

} // namespace VR_DAW 