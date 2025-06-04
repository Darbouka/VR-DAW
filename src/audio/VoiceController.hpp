#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace VR_DAW {

class VoiceController {
public:
    static VoiceController& getInstance();

    // Grundlegende Funktionen
    void initialize();
    void process();
    void optimize();
    void monitor();
    void adjust();
    void finalize();

    // Spracherkennung
    void addCommand(const std::string& command, const std::string& action);
    void removeCommand(const std::string& command);
    void updateCommand(const std::string& command, const std::string& action);
    bool isCommandRecognized(const std::string& command) const;

    // Sprachverarbeitung
    void setCommandCallback(const std::string& command, std::function<void()> callback);
    void removeCommandCallback(const std::string& command);
    void processCommand(const std::string& command);

    // Spracheinstellungen
    void setLanguage(const std::string& language);
    void setVoiceModel(const std::string& model);
    void setNoiseReduction(bool enable);
    void setAutoGainControl(bool enable);

    // Parameter
    void setSensitivity(float sensitivity);
    void setConfidenceThreshold(float threshold);
    void setMaxAlternatives(int alternatives);
    void setContinuousMode(bool enable);

private:
    VoiceController() = default;
    ~VoiceController() = default;

    VoiceController(const VoiceController&) = delete;
    VoiceController& operator=(const VoiceController&) = delete;

    // Spracherkennung Komponenten
    std::unique_ptr<class VoiceRecognizer> recognizer;
    std::unique_ptr<class VoiceModel> model;
    std::unique_ptr<class VoicePredictor> predictor;
    std::unique_ptr<class VoiceCalibrator> calibrator;

    // Sprachverarbeitung Komponenten
    std::unique_ptr<class VoiceProcessor> processor;
    std::unique_ptr<class VoiceExecutor> executor;
    std::unique_ptr<class VoiceQueue> queue;
    std::unique_ptr<class VoiceScheduler> scheduler;

    // Monitoring Komponenten
    std::unique_ptr<class VoiceMonitor> monitor;
    std::unique_ptr<class VoiceAnalyzer> analyzer;
    std::unique_ptr<class VoiceLogger> logger;
    std::unique_ptr<class VoiceReporter> reporter;

    // Interne Hilfsfunktionen
    void initializeComponents();
    void processVoice();
    void optimizeRecognition();
    void monitorPerformance();
    void adjustParameters();
    void finalizeComponents();

    // Sprach-Datenstrukturen
    struct VoiceData {
        std::string command;
        std::string action;
        std::function<void()> callback;
        float sensitivity;
        float confidenceThreshold;
        int maxAlternatives;
        bool continuousMode;
    };

    std::map<std::string, VoiceData> commands;
    std::string currentLanguage = "de-DE";
    std::string currentModel = "default";
    bool noiseReductionEnabled = true;
    bool autoGainControlEnabled = true;
    float currentSensitivity = 1.0f;
    float currentConfidenceThreshold = 0.7f;
    int currentMaxAlternatives = 3;
    bool continuousModeEnabled = false;
};

} // namespace VR_DAW 