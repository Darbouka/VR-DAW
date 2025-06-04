#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <juce_audio_basics/juce_audio_basics.h>

namespace VR_DAW {

class Automation {
public:
    static Automation& getInstance();

    // Automation-Punkt
    struct AutomationPoint {
        double time;      // Zeit in Sekunden
        float value;      // Automatisierter Wert
        float curve;      // Interpolationskurve (-1 bis 1)
    };

    // Automation-Lane
    struct AutomationLane {
        std::string id;
        std::string parameterId;
        std::vector<AutomationPoint> points;
        bool enabled;
    };

    // Automation-Track
    struct AutomationTrack {
        std::string id;
        std::vector<AutomationLane> lanes;
    };

    // Initialisierung
    void initialize();
    void shutdown();

    // Automation-Track-Verwaltung
    void createTrack(const std::string& id);
    void deleteTrack(const std::string& id);
    std::vector<AutomationTrack> getTracks() const;

    // Automation-Lane-Verwaltung
    void createLane(const std::string& trackId, const std::string& parameterId);
    void deleteLane(const std::string& trackId, const std::string& laneId);
    void enableLane(const std::string& trackId, const std::string& laneId, bool enable);

    // Automation-Punkt-Verwaltung
    void addPoint(const std::string& trackId, const std::string& laneId, 
                 double time, float value, float curve = 0.0f);
    void removePoint(const std::string& trackId, const std::string& laneId, double time);
    void updatePoint(const std::string& trackId, const std::string& laneId,
                    double time, float value, float curve = 0.0f);

    // Automation-Ausführung
    float getValueAtTime(const std::string& trackId, const std::string& laneId, double time) const;
    void processAutomation(double currentTime, juce::AudioBuffer<float>& buffer);

    // Automation-Import/Export
    void importAutomation(const std::string& filePath);
    void exportAutomation(const std::string& filePath) const;

private:
    Automation() = default;
    ~Automation() = default;
    
    Automation(const Automation&) = delete;
    Automation& operator=(const Automation&) = delete;

    std::vector<AutomationTrack> tracks;
    
    // Interne Hilfsfunktionen
    float interpolateValue(const AutomationPoint& p1, const AutomationPoint& p2, 
                          double time) const;
    AutomationTrack* findTrack(const std::string& id);
    AutomationLane* findLane(AutomationTrack& track, const std::string& laneId);
}; 