#pragma once

// Temporär deaktiviert
//#include <webrtc/api/peer_connection_interface.h>

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <webrtc/api/peer_connection_factory_interface.h>
#include <webrtc/api/audio_track_interface.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/audio_device_module.h>
#include <webrtc/api/audio/audio_processor_interface.h>
#include <webrtc/api/audio/audio_transport.h>

namespace VR_DAW {

class WebRTCError : public std::runtime_error {
public:
    enum class ErrorCode {
        InitializationFailed,
        PeerConnectionCreationFailed,
        AudioTrackCreationFailed,
        IceCandidateError,
        SdpError,
        AudioProcessingError,
        ResourceError,
        ConfigurationError,
        ConnectionError
    };

    WebRTCError(ErrorCode code, const std::string& message)
        : std::runtime_error(message), errorCode(code) {}

    ErrorCode getErrorCode() const { return errorCode; }

private:
    ErrorCode errorCode;
};

class WebRTCManager {
public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Failed
    };

    struct AudioEvent {
        const float* audioData;
        size_t numFrames;
        size_t numChannels;
        uint32_t sampleRate;
        std::string peerId;
    };

    using OnIceCandidateCallback = std::function<void(const std::string& peerId, const std::string& candidate)>;
    using OnTrackCallback = std::function<void(const AudioEvent& event)>;
    using OnConnectionStateChangeCallback = std::function<void(const std::string& peerId, ConnectionState state)>;
    using AudioCallback = std::function<void(const AudioEvent& event)>;

    static WebRTCManager& getInstance();

    WebRTCManager();
    ~WebRTCManager();

    // Temporär deaktivierte Methoden
    /*
    void initialize();
    void connect(const std::string& peerId);
    void disconnect();
    void sendAudioData(const float* data, size_t size);
    */

    bool createPeerConnection(const std::string& peerId);
    bool addAudioTrack(const std::string& peerId);
    bool removeAudioTrack(const std::string& peerId);
    void cleanupPeerConnections();

    // SDP-Handling
    bool createOffer(const std::string& peerId);
    bool createAnswer(const std::string& peerId);
    bool setRemoteDescription(const std::string& peerId, const std::string& sdp, bool isOffer);
    bool addIceCandidate(const std::string& peerId, const std::string& candidate);

    // Callback-Setter
    void setOnIceCandidateCallback(OnIceCandidateCallback callback);
    void setOnTrackCallback(OnTrackCallback callback);
    void setOnConnectionStateChangeCallback(OnConnectionStateChangeCallback callback);
    void setAudioCallback(AudioCallback callback);

    // Audio-Verarbeitung
    void processAudioData(const AudioEvent& event);
    void setAudioProcessingEnabled(bool enabled);
    void setAudioProcessingConfig(const AudioProcessingConfig& config);

    // Fehlerbehandlung
    bool hasError() const {
        std::lock_guard<std::mutex> lock(errorMutex);
        return !lastError.empty();
    }
    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(errorMutex);
        return lastError;
    }
    void clearError() {
        std::lock_guard<std::mutex> lock(errorMutex);
        lastError.clear();
    }
    std::vector<std::string> getErrorLog() const {
        std::lock_guard<std::mutex> lock(errorMutex);
        return errorLog;
    }

private:
    bool initializePeerConnectionFactory();
    void cleanup();

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
    std::map<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface>> peerConnections;
    
    OnIceCandidateCallback onIceCandidateCallback;
    OnTrackCallback onTrackCallback;
    OnConnectionStateChangeCallback onConnectionStateChangeCallback;
    AudioCallback audioCallback;

    bool audioProcessingEnabled;
    AudioProcessingConfig audioProcessingConfig;

    // Audio-Verarbeitung
    class AudioProcessor;
    std::unique_ptr<AudioProcessor> audioProcessor;

    // Thread-Sicherheit
    mutable std::shared_mutex peerConnectionsMutex;
    mutable std::mutex audioProcessorMutex;
    mutable std::mutex callbackMutex;
    mutable std::mutex errorMutex;

    // Thread-sichere Methoden
    void setError(const std::string& error) {
        std::lock_guard<std::mutex> lock(errorMutex);
        lastError = error;
        errorLog.push_back(error);
        if (errorLog.size() > 100) {
            errorLog.erase(errorLog.begin());
        }
    }

    std::string lastError;
    std::vector<std::string> errorLog;
};

struct AudioProcessingConfig {
    bool noiseSuppression;
    bool echoCancellation;
    bool automaticGainControl;
    float gainControlLevel;
    int sampleRate;
    int numChannels;
};

} // namespace VR_DAW 