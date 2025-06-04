#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace VR_DAW {

class UserInteractionManager {
public:
    static UserInteractionManager& getInstance();

    // Grundlegende Interaktionsfunktionen
    void initialize();
    void improve();
    void handleInteraction();
    void optimizeInteraction();
    void monitorInteraction();
    void adjustInteraction();
    void finalize();

    // Gestensteuerung
    void setGestureControlEnabled(bool enable);
    void setGestureSensitivity(float sensitivity);
    void addGesture(const std::string& name, const std::string& pattern);
    void removeGesture(const std::string& name);

    // Sprachsteuerung
    void setVoiceControlEnabled(bool enable);
    void setVoiceRecognitionLanguage(const std::string& language);
    void addVoiceCommand(const std::string& command, const std::string& action);
    void removeVoiceCommand(const std::string& command);

    // Augenverfolgung
    void setEyeTrackingEnabled(bool enable);
    void setEyeTrackingSensitivity(float sensitivity);
    void setEyeTrackingCalibration(const std::vector<float>& calibration);
    void resetEyeTrackingCalibration();

    // Haptisches Feedback
    void setHapticFeedbackEnabled(bool enable);
    void setHapticIntensity(float intensity);
    void setHapticPattern(const std::string& pattern);
    void triggerHapticFeedback(const std::string& event);

private:
    UserInteractionManager() = default;
    ~UserInteractionManager() = default;

    UserInteractionManager(const UserInteractionManager&) = delete;
    UserInteractionManager& operator=(const UserInteractionManager&) = delete;

    // Gestensteuerung Komponenten
    std::unique_ptr<class GestureController> gestureController;
    std::unique_ptr<class GestureRecognizer> gestureRecognizer;
    std::unique_ptr<class GestureProcessor> gestureProcessor;
    std::unique_ptr<class GestureMonitor> gestureMonitor;

    // Sprachsteuerung Komponenten
    std::unique_ptr<class VoiceController> voiceController;
    std::unique_ptr<class VoiceRecognizer> voiceRecognizer;
    std::unique_ptr<class VoiceProcessor> voiceProcessor;
    std::unique_ptr<class VoiceMonitor> voiceMonitor;

    // Augenverfolgung Komponenten
    std::unique_ptr<class EyeTracker> eyeTracker;
    std::unique_ptr<class EyeCalibrator> eyeCalibrator;
    std::unique_ptr<class EyeProcessor> eyeProcessor;
    std::unique_ptr<class EyeMonitor> eyeMonitor;

    // Haptisches Feedback Komponenten
    std::unique_ptr<class HapticController> hapticController;
    std::unique_ptr<class HapticPatternGenerator> hapticPatternGenerator;
    std::unique_ptr<class HapticProcessor> hapticProcessor;
    std::unique_ptr<class HapticMonitor> hapticMonitor;

    // Interne Hilfsfunktionen
    void initializeGestureControl();
    void initializeVoiceControl();
    void initializeEyeTracking();
    void initializeHapticFeedback();

    void improveGestureControl();
    void improveVoiceControl();
    void improveEyeTracking();
    void improveHapticFeedback();

    void processGestures();
    void processVoice();
    void processEyeTracking();
    void processHapticFeedback();

    void optimizeGestureControl();
    void optimizeVoiceControl();
    void optimizeEyeTracking();
    void optimizeHapticFeedback();

    void monitorGestureControl();
    void monitorVoiceControl();
    void monitorEyeTracking();
    void monitorHapticFeedback();

    void adjustGestureControl();
    void adjustVoiceControl();
    void adjustEyeTracking();
    void adjustHapticFeedback();

    void finalizeGestureControl();
    void finalizeVoiceControl();
    void finalizeEyeTracking();
    void finalizeHapticFeedback();
};

} // namespace VR_DAW 