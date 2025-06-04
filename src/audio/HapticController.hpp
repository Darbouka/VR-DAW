#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace VR_DAW {

class HapticController {
public:
    static HapticController& getInstance();

    // Grundlegende Funktionen
    void initialize();
    void process();
    void optimize();
    void monitor();
    void adjust();
    void finalize();

    // Haptisches Feedback
    void triggerFeedback(const std::string& event);
    void stopFeedback();
    void pauseFeedback();
    void resumeFeedback();
    bool isFeedbackActive() const;

    // Muster-Generierung
    void addPattern(const std::string& name, const std::vector<float>& pattern);
    void removePattern(const std::string& name);
    void updatePattern(const std::string& name, const std::vector<float>& pattern);
    std::vector<float> getPattern(const std::string& name) const;

    // Parameter
    void setIntensity(float intensity);
    void setDuration(float duration);
    void setFrequency(float frequency);
    void setWaveform(const std::string& waveform);

private:
    HapticController() = default;
    ~HapticController() = default;

    HapticController(const HapticController&) = delete;
    HapticController& operator=(const HapticController&) = delete;

    // Haptisches Feedback Komponenten
    std::unique_ptr<class HapticPatternGenerator> patternGenerator;
    std::unique_ptr<class HapticWaveformGenerator> waveformGenerator;
    std::unique_ptr<class HapticScheduler> scheduler;
    std::unique_ptr<class HapticCalibrator> calibrator;

    // Verarbeitungskomponenten
    std::unique_ptr<class HapticProcessor> processor;
    std::unique_ptr<class HapticExecutor> executor;
    std::unique_ptr<class HapticQueue> queue;
    std::unique_ptr<class HapticManager> manager;

    // Monitoring Komponenten
    std::unique_ptr<class HapticMonitor> monitor;
    std::unique_ptr<class HapticAnalyzer> analyzer;
    std::unique_ptr<class HapticLogger> logger;
    std::unique_ptr<class HapticReporter> reporter;

    // Interne Hilfsfunktionen
    void initializeComponents();
    void processHapticData();
    void optimizeFeedback();
    void monitorPerformance();
    void adjustParameters();
    void finalizeComponents();

    // Haptisches Feedback-Datenstrukturen
    struct HapticData {
        std::string event;
        std::vector<float> pattern;
        float intensity;
        float duration;
        float frequency;
        std::string waveform;
    };

    struct FeedbackParameters {
        float intensity;
        float duration;
        float frequency;
        std::string waveform;
    };

    std::map<std::string, std::vector<float>> patterns;
    FeedbackParameters parameters;
    bool isFeedbackActive = false;
    bool isPaused = false;
    float currentIntensity = 1.0f;
    float currentDuration = 0.1f;
    float currentFrequency = 100.0f;
    std::string currentWaveform = "sine";
};

} // namespace VR_DAW 