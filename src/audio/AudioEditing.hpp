#pragma once

#include <memory>
#include <vector>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>

namespace VR_DAW {

class AudioEditing {
public:
    static AudioEditing& getInstance();

    // Audio-Region
    struct AudioRegion {
        std::string id;
        std::string sourceId;
        double startTime;     // Startzeit in Sekunden
        double endTime;       // Endzeit in Sekunden
        double offset;        // Offset in der Quelldatei
        float gain;          // Verstärkung
        bool muted;          // Stummgeschaltet
        float pitch;         // Tonhöhenänderung in Halbtönen
        float bpm;           // BPM des Samples
        float fadeInTime;    // Fade-In-Zeit in Sekunden
        float fadeOutTime;   // Fade-Out-Zeit in Sekunden
    };

    // Flex-Marker
    struct FlexMarker {
        double time;         // Zeit in Sekunden
        double stretch;      // Dehnungsfaktor
        bool locked;         // Marker fixiert
    };

    // Flex-Modus
    enum class FlexMode {
        Slicing,            // Zeitbasierte Slices
        Stretching,         // Zeitstreckung
        Warping,            // Zeitverzerrung
        Rhythmic           // Rhythmusbasierte Anpassung
    };

    // Fade-Kurve
    enum class FadeCurve {
        Linear,             // Lineare Fade-Kurve
        Exponential,        // Exponentielle Fade-Kurve
        Logarithmic,        // Logarithmische Fade-Kurve
        Cosine             // Kosinus-Fade-Kurve
    };

    // Initialisierung
    void initialize();
    void shutdown();

    // Region-Verwaltung
    void createRegion(const std::string& sourceId, double startTime, double endTime);
    void deleteRegion(const std::string& regionId);
    void moveRegion(const std::string& regionId, double newStartTime);
    void resizeRegion(const std::string& regionId, double newStartTime, double newEndTime);
    void splitRegion(const std::string& regionId, double splitTime);
    void mergeRegions(const std::string& regionId1, const std::string& regionId2);

    // Flex-Funktionalität
    void setFlexMode(const std::string& regionId, FlexMode mode);
    void addFlexMarker(const std::string& regionId, double time, double stretch = 1.0);
    void removeFlexMarker(const std::string& regionId, double time);
    void updateFlexMarker(const std::string& regionId, double time, double stretch);
    void lockFlexMarker(const std::string& regionId, double time, bool locked);

    // Neue Funktionen für Tonhöhe und BPM
    void setPitch(const std::string& regionId, float semitones);
    float getPitch(const std::string& regionId) const;
    void setBPM(const std::string& regionId, float bpm);
    float getBPM(const std::string& regionId) const;
    void adjustPitchToNote(const std::string& regionId, const std::string& note); // z.B. "C4", "D#3"

    // Neue Funktionen für Fade-Effekte
    void setFadeIn(const std::string& regionId, float time, FadeCurve curve = FadeCurve::Linear);
    void setFadeOut(const std::string& regionId, float time, FadeCurve curve = FadeCurve::Linear);
    void removeFadeIn(const std::string& regionId);
    void removeFadeOut(const std::string& regionId);
    float getFadeInTime(const std::string& regionId) const;
    float getFadeOutTime(const std::string& regionId) const;

    // Audio-Verarbeitung
    void processAudio(juce::AudioBuffer<float>& buffer, double currentTime);
    void applyFlex(const std::string& regionId, juce::AudioBuffer<float>& buffer);
    void renderRegion(const std::string& regionId, juce::AudioBuffer<float>& buffer);

    // Import/Export
    void importRegions(const std::string& filePath);
    void exportRegions(const std::string& filePath) const;

private:
    AudioEditing() = default;
    ~AudioEditing() = default;
    
    AudioEditing(const AudioEditing&) = delete;
    AudioEditing& operator=(const AudioEditing&) = delete;

    std::vector<AudioRegion> regions;
    std::map<std::string, std::vector<FlexMarker>> flexMarkers;
    std::map<std::string, FlexMode> flexModes;
    std::map<std::string, FadeCurve> fadeInCurves;
    std::map<std::string, FadeCurve> fadeOutCurves;
    
    // Interne Hilfsfunktionen
    AudioRegion* findRegion(const std::string& id);
    void updateRegionTiming(AudioRegion& region);
    void processFlexMarkers(const std::string& regionId, juce::AudioBuffer<float>& buffer);
    double calculateStretchFactor(const std::string& regionId, double time) const;
    
    // Neue interne Hilfsfunktionen
    void applyPitchShift(juce::AudioBuffer<float>& buffer, float semitones);
    void applyBPMChange(juce::AudioBuffer<float>& buffer, float originalBPM, float targetBPM);
    void applyFadeIn(juce::AudioBuffer<float>& buffer, float time, FadeCurve curve);
    void applyFadeOut(juce::AudioBuffer<float>& buffer, float time, FadeCurve curve);
    float calculateFadeGain(float position, FadeCurve curve) const;
    float noteToSemitones(const std::string& note) const;
}; 