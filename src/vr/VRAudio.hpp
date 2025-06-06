#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

namespace VR_DAW {

class VRAudio {
public:
    VRAudio();
    ~VRAudio();

    // Initialisierung und Shutdown
    bool initialize();
    void shutdown();
    void update();

    // Audio-Status
    bool isInitialized() const;
    bool isPlaying() const;
    std::string getStatus() const;

    // Audio-Quellen
    struct AudioSource {
        std::string name;
        glm::vec3 position;
        glm::vec3 direction;
        float volume;
        float pitch;
        float minDistance;
        float maxDistance;
        bool loop;
        bool spatial;
    };

    int createSource(const std::string& name, const AudioSource& source);
    void destroySource(int sourceId);
    void updateSource(int sourceId, const AudioSource& source);
    AudioSource getSource(int sourceId) const;

    // Audio-Wiedergabe
    void playSource(int sourceId);
    void pauseSource(int sourceId);
    void stopSource(int sourceId);
    bool isSourcePlaying(int sourceId) const;
    void setSourceVolume(int sourceId, float volume);
    void setSourcePitch(int sourceId, float pitch);
    void setSourcePosition(int sourceId, const glm::vec3& position);
    void setSourceDirection(int sourceId, const glm::vec3& direction);

    // Audio-Effekte
    struct AudioEffect {
        std::string name;
        std::string type;
        std::map<std::string, float> parameters;
    };

    void addEffect(int sourceId, const AudioEffect& effect);
    void removeEffect(int sourceId, const std::string& effectName);
    void updateEffect(int sourceId, const AudioEffect& effect);
    AudioEffect getEffect(int sourceId, const std::string& effectName) const;

    // 3D-Audio
    void setListenerPosition(const glm::vec3& position);
    void setListenerOrientation(const glm::quat& orientation);
    void setListenerVelocity(const glm::vec3& velocity);
    void setRoomProperties(float roomSize, float roomDamping, float roomReflection);
    void setReverbProperties(float density, float diffusion, float gain, float gainHF);

    // Audio-Streaming
    struct AudioStream {
        std::string name;
        std::string format;
        int channels;
        int sampleRate;
        bool streaming;
    };

    int createStream(const std::string& name, const AudioStream& stream);
    void destroyStream(int streamId);
    void updateStream(int streamId, const AudioStream& stream);
    AudioStream getStream(int streamId) const;
    void writeStreamData(int streamId, const void* data, size_t size);
    size_t getStreamBufferSize(int streamId) const;

    // Audio-Aufnahme
    struct AudioRecorder {
        std::string name;
        std::string format;
        int channels;
        int sampleRate;
        bool recording;
    };

    int createRecorder(const std::string& name, const AudioRecorder& recorder);
    void destroyRecorder(int recorderId);
    void updateRecorder(int recorderId, const AudioRecorder& recorder);
    AudioRecorder getRecorder(int recorderId) const;
    void startRecording(int recorderId);
    void stopRecording(int recorderId);
    bool isRecording(int recorderId) const;
    void getRecordingData(int recorderId, void* data, size_t size);

    // Audio-Analyse
    struct AudioAnalysis {
        float rms;
        float peak;
        float crest;
        std::vector<float> spectrum;
        std::vector<float> waveform;
    };

    AudioAnalysis analyzeSource(int sourceId) const;
    AudioAnalysis analyzeStream(int streamId) const;
    AudioAnalysis analyzeRecording(int recorderId) const;

    // Debug
    void enableDebugMode(bool enable);
    void showDebugInfo();
    void renderDebugShapes();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool initialized;
    bool playing;
    bool debugEnabled;

    // Audio-Quellen
    std::map<int, AudioSource> sources;
    std::map<int, AudioStream> streams;
    std::map<int, AudioRecorder> recorders;

    // Listener-Eigenschaften
    glm::vec3 listenerPosition;
    glm::quat listenerOrientation;
    glm::vec3 listenerVelocity;

    // Raum-Eigenschaften
    float roomSize;
    float roomDamping;
    float roomReflection;
    float reverbDensity;
    float reverbDiffusion;
    float reverbGain;
    float reverbGainHF;

    void initializeAudio();
    void shutdownAudio();
    void updateAudio();
    void renderDebugInfo();
};

} // namespace VR_DAW 