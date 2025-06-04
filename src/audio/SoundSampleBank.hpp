#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace VR_DAW {

class SoundSampleBank {
public:
    SoundSampleBank();
    ~SoundSampleBank();
    
    // Initialisierung und Shutdown
    void initialize();
    void shutdown();
    
    // Sample-Kategorien
    enum class SampleCategory {
        Drums,          // Schlagzeug-Samples
        Bass,           // Bass-Samples
        Synth,          // Synthesizer-Samples
        Vocals,         // Gesangs-Samples
        Guitar,         // Gitarren-Samples
        Piano,          // Piano-Samples
        Strings,        // Streicher-Samples
        Brass,          // Blasinstrumente
        Woodwind,       // Holzblasinstrumente
        Percussion,     // Perkussion
        FX,             // Effekte
        Ambient,        // Ambient-Sounds
        World,          // Weltmusik-Instrumente
        Foley,          // Foley-Sounds
        Loops,          // Loop-Samples
        OneShots,       // One-Shot-Samples
        Pads,           // Pad-Sounds
        Leads,          // Lead-Sounds
        Arps,           // Arpeggiator-Sounds
        Plucks,         // Pluck-Sounds
        Keys,           // Keyboard-Sounds
        Ethnic,         // Ethnische Instrumente
        Cinematic,      // Filmische Sounds
        Game,           // Game-Sounds
        Nature,         // Naturgeräusche
        Urban,          // Urbane Geräusche
        Industrial,     // Industrielle Geräusche
        Experimental,   // Experimentelle Sounds
        Custom          // Benutzerdefinierte Kategorien
    };
    
    // Sample-Metadaten
    struct SampleMetadata {
        std::string name;
        std::string description;
        std::string source;
        std::string license;
        std::string format;
        float duration;
        float bpm;
        std::string key;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point lastUpdated;
        bool isFree;
        std::string downloadUrl;
        std::string previewUrl;
        std::string author;
        std::string country;
        std::string culture;
        std::string instrument;
        std::string technique;
        std::string mood;
        std::string genre;
        std::string style;
        std::string era;
        std::string quality;
        std::string bitDepth;
        std::string sampleRate;
        std::string channels;
        std::string size;
        std::string hash;
        std::string version;
    };
    
    // Sample-Verwaltung
    void addSample(const std::string& path, const SampleMetadata& metadata);
    void removeSample(const std::string& name);
    void updateSample(const std::string& name, const SampleMetadata& metadata);
    std::vector<SampleMetadata> getSamplesByCategory(SampleCategory category);
    std::vector<SampleMetadata> searchSamples(const std::string& query);
    std::vector<SampleMetadata> getNewSamples();
    std::vector<SampleMetadata> getUpdatedSamples();
    std::vector<SampleMetadata> getPopularSamples();
    std::vector<SampleMetadata> getRecommendedSamples();
    
    // Sample-Wiedergabe
    void playSample(const std::string& name);
    void stopSample(const std::string& name);
    void pauseSample(const std::string& name);
    void resumeSample(const std::string& name);
    void setSampleVolume(const std::string& name, float volume);
    void setSamplePan(const std::string& name, float pan);
    void setSamplePitch(const std::string& name, float pitch);
    void setSampleSpeed(const std::string& name, float speed);
    
    // Sample-Bearbeitung
    void trimSample(const std::string& name, float startTime, float endTime);
    void normalizeSample(const std::string& name);
    void reverseSample(const std::string& name);
    void loopSample(const std::string& name, float startTime, float endTime);
    void fadeSample(const std::string& name, float fadeInTime, float fadeOutTime);
    
    // Sample-Export
    void exportSample(const std::string& name, const std::string& format);
    void exportSampleBatch(const std::vector<std::string>& names, const std::string& format);
    void exportSampleCategory(SampleCategory category, const std::string& format);
    
    // Update-Verwaltung
    void checkForUpdates();
    void updateSamples();
    void scheduleUpdate(const std::chrono::system_clock::time_point& time);
    void cancelUpdate();
    std::chrono::system_clock::time_point getNextUpdateTime() const;
    
    // Lizenz-Verwaltung
    bool isSampleFree(const std::string& name) const;
    std::string getSampleLicense(const std::string& name) const;
    void setSampleLicense(const std::string& name, const std::string& license);
    
    // Qualitätskontrolle
    void validateSample(const std::string& name);
    void validateAllSamples();
    std::vector<std::string> getInvalidSamples() const;
    void repairSample(const std::string& name);
    
    // Statistik
    size_t getTotalSampleCount() const;
    size_t getCategorySampleCount(SampleCategory category) const;
    size_t getFreeSampleCount() const;
    size_t getPaidSampleCount() const;
    std::map<SampleCategory, size_t> getCategoryDistribution() const;
    std::map<std::string, size_t> getFormatDistribution() const;
    std::map<std::string, size_t> getLicenseDistribution() const;
    
private:
    // Interne Strukturen
    struct SampleData {
        juce::AudioBuffer<float> buffer;
        SampleMetadata metadata;
        bool isPlaying;
        float volume;
        float pan;
        float pitch;
        float speed;
    };
    
    // Speicher
    std::unordered_map<std::string, SampleData> samples;
    std::map<SampleCategory, std::vector<std::string>> categoryIndex;
    std::chrono::system_clock::time_point nextUpdateTime;
    
    // Hilfsfunktionen
    void loadSample(const std::string& path);
    void saveSample(const std::string& name, const std::string& path);
    void updateCategoryIndex(const std::string& name, SampleCategory category);
    void removeFromCategoryIndex(const std::string& name, SampleCategory category);
    void validateSampleFormat(const std::string& name);
    void validateSampleMetadata(const SampleMetadata& metadata);
    void updateSampleStatistics();
    void notifyUpdateAvailable();
    void downloadSample(const std::string& url, const std::string& path);
    void extractSample(const std::string& archivePath, const std::string& extractPath);
    void compressSample(const std::string& path, const std::string& format);
    void generatePreview(const std::string& name);
    void updateSampleHash(const std::string& name);
    void backupSample(const std::string& name);
    void restoreSample(const std::string& name);
    void cleanupOldSamples();
    void optimizeStorage();
    void checkDiskSpace();
    void validateDownload(const std::string& url);
    void reportError(const std::string& message);
    void logActivity(const std::string& activity);
};

} // namespace VR_DAW 