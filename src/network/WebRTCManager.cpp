#include "WebRTCManager.hpp"
#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <webrtc/api/peer_connection_factory.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/audio_track_interface.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/create_peerconnection_factory.h>
#include <webrtc/rtc_base/thread.h>
#include <webrtc/rtc_base/ssl_adapter.h>
#include <memory>
#include <webrtc/api/jsep.h>

namespace VR_DAW {

class WebRTCManager::Impl {
public:
    std::map<std::string, rtc::scoped_refptr<webrtc::PeerConnectionInterface>> peerConnections;
    std::function<void(const std::string&, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>)> onTrackCallback;
    std::function<void(const std::string&, const webrtc::IceCandidateInterface*)> onIceCandidateCallback;
    std::mutex mutex;
    bool initialized = false;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory;
    rtc::scoped_refptr<webrtc::AudioSourceInterface> audioSource;

    Impl() = default;
    ~Impl() = default;
};

class WebRTCManager::PeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
    PeerConnectionObserver(const std::string& peerId, WebRTCManager* manager)
        : peerId(peerId), manager(manager) {}

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
    void OnRenegotiationNeeded() override {}
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
        if (manager && manager->onConnectionStateChangeCallback) {
            manager->onConnectionStateChangeCallback(peerId, new_state);
        }
    }
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
        if (manager && manager->onIceCandidateCallback && candidate) {
            std::string sdp;
            candidate->ToString(&sdp);
            manager->onIceCandidateCallback(peerId, sdp);
        }
    }
    void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override {
        if (manager && manager->onTrackCallback && transceiver->receiver()) {
            manager->onTrackCallback(peerId, transceiver->receiver()->track());
        }
    }
private:
    std::string peerId;
    WebRTCManager* manager;
};

class WebRTCManager::CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver {
public:
    CreateSessionDescriptionObserver(rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc, bool isOffer)
        : peerConnection(pc), isOffer(isOffer) {}

    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
        peerConnection->SetLocalDescription(
            new rtc::RefCountedObject<SetSessionDescriptionObserver>(peerConnection, desc),
            desc
        );
        std::cout << (isOffer ? "Offer" : "Answer") << " erfolgreich erstellt." << std::endl;
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cerr << "Fehler beim Erstellen der Session Description: " << error.message() << std::endl;
    }

private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
    bool isOffer;
};

class WebRTCManager::SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
public:
    SetSessionDescriptionObserver(rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc,
                                webrtc::SessionDescriptionInterface* desc)
        : peerConnection(pc), description(desc) {}

    void OnSuccess() override {
        std::cout << "Session Description erfolgreich gesetzt." << std::endl;
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cerr << "Fehler beim Setzen der Session Description: " << error.message() << std::endl;
    }

private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
    webrtc::SessionDescriptionInterface* description;
};

class AudioTrackSource : public webrtc::AudioSourceInterface {
public:
    AudioTrackSource() : audio_processor_(nullptr) {}

    void SetAudioProcessor(webrtc::AudioProcessorInterface* processor) {
        audio_processor_ = processor;
    }

    void AddSink(webrtc::AudioTrackSinkInterface* sink) override {
        sinks_.push_back(sink);
    }

    void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override {
        sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
    }

    void OnData(const void* audio_data, int bits_per_sample, int sample_rate,
                size_t number_of_channels, size_t number_of_frames) {
        if (!audio_processor_) return;

        // Audio-Daten verarbeiten
        audio_processor_->ProcessData(audio_data, bits_per_sample, sample_rate,
                                    number_of_channels, number_of_frames);

        // Verarbeitete Daten an alle Sinks senden
        for (auto* sink : sinks_) {
            sink->OnData(audio_data, bits_per_sample, sample_rate,
                        number_of_channels, number_of_frames);
        }
    }

    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }

private:
    webrtc::AudioProcessorInterface* audio_processor_;
    std::vector<webrtc::AudioTrackSinkInterface*> sinks_;
};

class AudioTrackSink : public webrtc::AudioTrackSinkInterface {
public:
    AudioTrackSink(WebRTCManager* manager, const std::string& peerId)
        : manager(manager), peerId(peerId) {}

    void OnData(const void* audio_data, int bits_per_sample, int sample_rate,
                size_t number_of_channels, size_t number_of_frames) override {
        // Audio-Daten an den Audio-Engine weiterleiten
        if (manager->audioCallback) {
            AudioEvent event;
            event.type = AudioEvent::Type::AudioData;
            event.audioData = static_cast<const float*>(audio_data);
            event.numFrames = number_of_frames;
            event.numChannels = number_of_channels;
            event.sampleRate = sample_rate;
            manager->audioCallback(event);
        }
    }

private:
    WebRTCManager* manager;
    std::string peerId;
};

class WebRTCManager::AudioProcessor {
public:
    AudioProcessor() : enabled(false) {
        config.noiseSuppression = true;
        config.echoCancellation = true;
        config.automaticGainControl = true;
        config.gainControlLevel = 1.0f;
        config.sampleRate = 48000;
        config.numChannels = 2;
        
        // Initialisiere adaptive Filter
        echoFilter.resize(1024, 0.0f);
        noiseFilter.resize(1024, 0.0f);
        learningRate = 0.01f;
    }

    void processAudio(AudioEvent& event) {
        if (!enabled) return;

        try {
            // Rauschunterdrückung
            if (config.noiseSuppression) {
                applyNoiseSuppression(event);
            }

            // Echounterdrückung
            if (config.echoCancellation) {
                applyEchoCancellation(event);
            }

            // Automatische Verstärkungsregelung
            if (config.automaticGainControl) {
                applyAutomaticGainControl(event);
            }
        } catch (const std::exception& e) {
            throw WebRTCError(WebRTCError::ErrorCode::AudioProcessingError,
                            "Fehler bei der Audio-Verarbeitung: " + std::string(e.what()));
        }
    }

    void setEnabled(bool value) { enabled = value; }
    void setConfig(const AudioProcessingConfig& newConfig) { config = newConfig; }

private:
    void applyNoiseSuppression(AudioEvent& event) {
        // Spektrale Subtraktion für Rauschunterdrückung
        std::vector<float> spectrum(event.numFrames);
        std::vector<float> noiseFloor(event.numFrames);
        
        // FFT der Eingabedaten
        for (size_t i = 0; i < event.numFrames; ++i) {
            spectrum[i] = event.audioData[i];
        }
        
        // Rauschschätzung
        for (size_t i = 0; i < event.numFrames; ++i) {
            noiseFloor[i] = std::min(noiseFloor[i], std::abs(spectrum[i]));
        }
        
        // Spektrale Subtraktion
        for (size_t i = 0; i < event.numFrames; ++i) {
            float magnitude = std::abs(spectrum[i]);
            float phase = std::arg(std::complex<float>(spectrum[i], 0.0f));
            float denoisedMagnitude = std::max(0.0f, magnitude - noiseFloor[i]);
            spectrum[i] = denoisedMagnitude * std::cos(phase);
        }
        
        // Rücktransformation
        for (size_t i = 0; i < event.numFrames; ++i) {
            const_cast<float*>(event.audioData)[i] = spectrum[i];
        }
    }

    void applyEchoCancellation(AudioEvent& event) {
        // Adaptiver Filter für Echounterdrückung
        std::vector<float> output(event.numFrames * event.numChannels);
        
        for (size_t i = 0; i < event.numFrames * event.numChannels; ++i) {
            float input = event.audioData[i];
            float echoEstimate = 0.0f;
            
            // Filter-Operation
            for (size_t j = 0; j < echoFilter.size(); ++j) {
                if (i >= j) {
                    echoEstimate += echoFilter[j] * event.audioData[i - j];
                }
            }
            
            // Fehlerberechnung und Filter-Update
            float error = input - echoEstimate;
            for (size_t j = 0; j < echoFilter.size(); ++j) {
                if (i >= j) {
                    echoFilter[j] += learningRate * error * event.audioData[i - j];
                }
            }
            
            output[i] = error;
        }
        
        // Ergebnis zurückkopieren
        std::copy(output.begin(), output.end(), const_cast<float*>(event.audioData));
    }

    void applyAutomaticGainControl(AudioEvent& event) {
        // Dynamische Verstärkungsregelung
        float targetLevel = -23.0f; // dBFS
        float compressionRatio = 4.0f;
        float attackTime = 0.01f; // Sekunden
        float releaseTime = 0.1f; // Sekunden
        
        float attackGain = std::exp(-1.0f / (attackTime * config.sampleRate));
        float releaseGain = std::exp(-1.0f / (releaseTime * config.sampleRate));
        
        float currentGain = 1.0f;
        float envelope = 0.0f;
        
        for (size_t i = 0; i < event.numFrames * event.numChannels; ++i) {
            float input = event.audioData[i];
            float inputLevel = 20.0f * std::log10(std::abs(input) + 1e-10f);
            
            // Hüllkurvenberechnung
            float targetGain = std::pow(10.0f, (targetLevel - inputLevel) / 20.0f);
            targetGain = std::min(1.0f, targetGain);
            
            // Dynamische Verstärkungsanpassung
            if (targetGain < currentGain) {
                currentGain = attackGain * currentGain + (1.0f - attackGain) * targetGain;
            } else {
                currentGain = releaseGain * currentGain + (1.0f - releaseGain) * targetGain;
            }
            
            // Verstärkung anwenden
            const_cast<float*>(event.audioData)[i] = input * currentGain;
        }
    }

    std::vector<float> echoFilter;
    std::vector<float> noiseFilter;
    float learningRate;
    bool enabled;
    AudioProcessingConfig config;
};

void WebRTCManager::processAudioData(const AudioEvent& event) {
    if (audioProcessor) {
        AudioEvent processedEvent = event;
        audioProcessor->processAudio(processedEvent);
        if (audioCallback) {
            audioCallback(processedEvent);
        }
    } else if (audioCallback) {
        audioCallback(event);
    }
}

void WebRTCManager::setAudioProcessingEnabled(bool enabled) {
    if (audioProcessor) {
        audioProcessor->setEnabled(enabled);
    }
}

void WebRTCManager::setAudioProcessingConfig(const AudioProcessingConfig& config) {
    if (audioProcessor) {
        audioProcessor->setConfig(config);
    }
}

class AudioDeviceModule : public webrtc::AudioDeviceModule {
public:
    AudioDeviceModule() : initialized_(false) {}

    int32_t Init() override {
        if (initialized_) return 0;
        initialized_ = true;
        return 0;
    }

    int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override {
        audio_callback_ = audioCallback;
        return 0;
    }

    int32_t StartPlayout() override {
        if (!initialized_) return -1;
        playing_ = true;
        return 0;
    }

    int32_t StopPlayout() override {
        playing_ = false;
        return 0;
    }

    int32_t StartRecording() override {
        if (!initialized_) return -1;
        recording_ = true;
        return 0;
    }

    int32_t StopRecording() override {
        recording_ = false;
        return 0;
    }

    bool Playing() const override { return playing_; }
    bool Recording() const override { return recording_; }

    int32_t PlayoutDelay(uint16_t* delayMS) const override {
        *delayMS = 0;
        return 0;
    }

    int32_t RecordingDelay(uint16_t* delayMS) const override {
        *delayMS = 0;
        return 0;
    }

    int32_t SetPlayoutBuffer(const webrtc::AudioDeviceModule::BufferType type,
                            uint16_t sizeMS) override {
        return 0;
    }

    int32_t PlayoutBuffer(webrtc::AudioDeviceModule::BufferType* type,
                         uint16_t* sizeMS) const override {
        *type = webrtc::AudioDeviceModule::kFixedBufferSize;
        *sizeMS = 10;
        return 0;
    }

    int32_t PlayoutDevices() override { return 1; }
    int32_t RecordingDevices() override { return 1; }

    int32_t PlayoutDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize],
                             char guid[webrtc::kAdmMaxGuidSize]) override {
        if (index != 0) return -1;
        strcpy(name, "Default Audio Device");
        strcpy(guid, "");
        return 0;
    }

    int32_t RecordingDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize],
                               char guid[webrtc::kAdmMaxGuidSize]) override {
        if (index != 0) return -1;
        strcpy(name, "Default Audio Device");
        strcpy(guid, "");
        return 0;
    }

    int32_t SetPlayoutDevice(uint16_t index) override { return 0; }
    int32_t SetRecordingDevice(uint16_t index) override { return 0; }

    int32_t InitSpeaker() override { return 0; }
    int32_t InitMicrophone() override { return 0; }

    int32_t SpeakerVolumeIsAvailable(bool* available) override {
        *available = true;
        return 0;
    }

    int32_t SetSpeakerVolume(uint32_t volume) override {
        speaker_volume_ = volume;
        return 0;
    }

    int32_t SpeakerVolume(uint32_t* volume) const override {
        *volume = speaker_volume_;
        return 0;
    }

    int32_t MaxSpeakerVolume(uint32_t* maxVolume) const override {
        *maxVolume = 100;
        return 0;
    }

    int32_t MinSpeakerVolume(uint32_t* minVolume) const override {
        *minVolume = 0;
        return 0;
    }

    int32_t MicrophoneVolumeIsAvailable(bool* available) override {
        *available = true;
        return 0;
    }

    int32_t SetMicrophoneVolume(uint32_t volume) override {
        microphone_volume_ = volume;
        return 0;
    }

    int32_t MicrophoneVolume(uint32_t* volume) const override {
        *volume = microphone_volume_;
        return 0;
    }

    int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const override {
        *maxVolume = 100;
        return 0;
    }

    int32_t MinMicrophoneVolume(uint32_t* minVolume) const override {
        *minVolume = 0;
        return 0;
    }

    int32_t SpeakerMuteIsAvailable(bool* available) override {
        *available = true;
        return 0;
    }

    int32_t SetSpeakerMute(bool enable) override {
        speaker_muted_ = enable;
        return 0;
    }

    int32_t SpeakerMute(bool* enabled) const override {
        *enabled = speaker_muted_;
        return 0;
    }

    int32_t MicrophoneMuteIsAvailable(bool* available) override {
        *available = true;
        return 0;
    }

    int32_t SetMicrophoneMute(bool enable) override {
        microphone_muted_ = enable;
        return 0;
    }

    int32_t MicrophoneMute(bool* enabled) const override {
        *enabled = microphone_muted_;
        return 0;
    }

    int32_t StereoPlayoutIsAvailable(bool* available) const override {
        *available = true;
        return 0;
    }

    int32_t SetStereoPlayout(bool enable) override {
        stereo_playout_ = enable;
        return 0;
    }

    int32_t StereoPlayout(bool* enabled) const override {
        *enabled = stereo_playout_;
        return 0;
    }

    int32_t StereoRecordingIsAvailable(bool* available) const override {
        *available = true;
        return 0;
    }

    int32_t SetStereoRecording(bool enable) override {
        stereo_recording_ = enable;
        return 0;
    }

    int32_t StereoRecording(bool* enabled) const override {
        *enabled = stereo_recording_;
        return 0;
    }

    int32_t SetPlayoutSampleRate(const uint32_t samplesPerSec) override {
        playout_sample_rate_ = samplesPerSec;
        return 0;
    }

    int32_t PlayoutSampleRate(uint32_t* samplesPerSec) const override {
        *samplesPerSec = playout_sample_rate_;
        return 0;
    }

    int32_t SetRecordingSampleRate(const uint32_t samplesPerSec) override {
        recording_sample_rate_ = samplesPerSec;
        return 0;
    }

    int32_t RecordingSampleRate(uint32_t* samplesPerSec) const override {
        *samplesPerSec = recording_sample_rate_;
        return 0;
    }

    int32_t SetLoudspeakerStatus(bool enable) override {
        loudspeaker_enabled_ = enable;
        return 0;
    }

    int32_t GetLoudspeakerStatus(bool* enabled) const override {
        *enabled = loudspeaker_enabled_;
        return 0;
    }

    bool BuiltInAECIsAvailable() const override { return true; }
    bool BuiltInAGCIsAvailable() const override { return true; }
    bool BuiltInNSIsAvailable() const override { return true; }

    int32_t EnableBuiltInAEC(bool enable) override {
        aec_enabled_ = enable;
        return 0;
    }

    int32_t EnableBuiltInAGC(bool enable) override {
        agc_enabled_ = enable;
        return 0;
    }

    int32_t EnableBuiltInNS(bool enable) override {
        ns_enabled_ = enable;
        return 0;
    }

private:
    bool initialized_;
    bool playing_;
    bool recording_;
    bool speaker_muted_;
    bool microphone_muted_;
    bool stereo_playout_;
    bool stereo_recording_;
    bool loudspeaker_enabled_;
    bool aec_enabled_;
    bool agc_enabled_;
    bool ns_enabled_;
    uint32_t speaker_volume_;
    uint32_t microphone_volume_;
    uint32_t playout_sample_rate_;
    uint32_t recording_sample_rate_;
    webrtc::AudioTransport* audio_callback_;
};

class AudioDeviceModuleFactory : public webrtc::AudioDeviceModule::AudioLayer {
public:
    static rtc::scoped_refptr<webrtc::AudioDeviceModule> CreateAudioDeviceModule() {
        return rtc::scoped_refptr<webrtc::AudioDeviceModule>(new AudioDeviceModule());
    }
};

WebRTCManager& WebRTCManager::getInstance() {
    static WebRTCManager instance;
    return instance;
}

WebRTCManager::WebRTCManager()
    : initialized_(false)
    , onIceCandidateCallback(nullptr)
    , onTrackCallback(nullptr)
    , onConnectionStateChangeCallback(nullptr)
    , audioCallback(nullptr) {
}

WebRTCManager::~WebRTCManager() {
    try {
        cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Fehler im Destruktor: " << e.what() << std::endl;
    }
}

bool WebRTCManager::initialize() {
    if (!initializePeerConnectionFactory()) {
        return false;
    }

    // AudioProcessor initialisieren
    audioProcessor = std::make_unique<AudioProcessor>();
    audioProcessingEnabled = true;

    // Standard-Audio-Verarbeitungskonfiguration
    AudioProcessingConfig defaultConfig;
    defaultConfig.noiseSuppression = true;
    defaultConfig.echoCancellation = true;
    defaultConfig.automaticGainControl = true;
    defaultConfig.gainControlLevel = 1.0f;
    defaultConfig.sampleRate = 48000;
    defaultConfig.numChannels = 2;
    setAudioProcessingConfig(defaultConfig);

    return true;
}

void WebRTCManager::shutdown() {
    cleanup();
    audioProcessor.reset();
}

bool WebRTCManager::initializePeerConnectionFactory() {
    peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
        nullptr, // network_thread
        nullptr, // worker_thread
        nullptr, // signaling_thread
        nullptr, // default_adm
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        nullptr, // video_encoder_factory
        nullptr, // video_decoder_factory
        nullptr, // audio_mixer
        nullptr  // audio_processing
    );
    if (!peerConnectionFactory) {
        std::cerr << "PeerConnectionFactory konnte nicht erstellt werden!" << std::endl;
        return false;
    }
    std::cout << "PeerConnectionFactory initialisiert." << std::endl;
    return true;
}

bool WebRTCManager::createPeerConnection(const std::string& peerId) {
    if (peerConnections.find(peerId) != peerConnections.end()) {
        std::cerr << "Peer-Verbindung für " << peerId << " existiert bereits" << std::endl;
        return false;
    }

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;

    webrtc::PeerConnectionInterface::IceServer iceServer;
    iceServer.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(iceServer);

    auto peerConnection = peerConnectionFactory->CreatePeerConnection(
        config,
        nullptr,
        nullptr,
        this
    );

    if (!peerConnection) {
        std::cerr << "Peer-Verbindung konnte nicht erstellt werden" << std::endl;
        return false;
    }

    // Audio-Verarbeitung für die Peer-Verbindung konfigurieren
    if (audioProcessor) {
        webrtc::AudioProcessing::Config apConfig;
        apConfig.noise_suppression.enabled = audioProcessingConfig.noiseSuppression;
        apConfig.echo_canceller.enabled = audioProcessingConfig.echoCancellation;
        apConfig.gain_controller1.enabled = audioProcessingConfig.automaticGainControl;
        apConfig.gain_controller1.target_level_dbfs = 3;
        apConfig.gain_controller1.compression_gain_db = 9;
        apConfig.gain_controller1.enable_limiter = true;

        peerConnection->GetAudioProcessingModule()->ApplyConfig(apConfig);
    }

    peerConnections[peerId] = peerConnection;
    std::cout << "Peer-Verbindung für " << peerId << " erfolgreich erstellt" << std::endl;
    return true;
}

bool WebRTCManager::addAudioTrack(const std::string& peerId) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "Keine Peer-Verbindung für " << peerId << " gefunden" << std::endl;
        return false;
    }

    auto peerConnection = it->second;

    // Audio-Track-Quelle erstellen
    cricket::AudioOptions options;
    options.echo_cancellation = audioProcessingConfig.echoCancellation;
    options.noise_suppression = audioProcessingConfig.noiseSuppression;
    options.auto_gain_control = audioProcessingConfig.automaticGainControl;

    rtc::scoped_refptr<webrtc::AudioSourceInterface> audioSource =
        peerConnectionFactory->CreateAudioSource(options);

    // Audio-Track erstellen
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack =
        peerConnectionFactory->CreateAudioTrack("audio_track", audioSource);

    if (!audioTrack) {
        std::cerr << "Audio-Track konnte nicht erstellt werden" << std::endl;
        return false;
    }

    // Audio-Track zur Peer-Verbindung hinzufügen
    auto result = peerConnection->AddTrack(audioTrack);
    if (!result.ok()) {
        std::cerr << "Audio-Track konnte nicht zur Peer-Verbindung hinzugefügt werden: "
                  << result.error().message() << std::endl;
        return false;
    }

    std::cout << "Audio-Track für " << peerId << " erfolgreich hinzugefügt" << std::endl;
    return true;
}

bool WebRTCManager::removeAudioTrack(const std::string& peerId) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "Keine Peer-Verbindung für " << peerId << " gefunden" << std::endl;
        return false;
    }

    auto peerConnection = it->second;
    auto senders = peerConnection->GetSenders();

    for (const auto& sender : senders) {
        if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
            auto result = peerConnection->RemoveTrack(sender);
            if (!result.ok()) {
                std::cerr << "Audio-Track konnte nicht entfernt werden: "
                          << result.error().message() << std::endl;
                return false;
            }
            std::cout << "Audio-Track für " << peerId << " erfolgreich entfernt" << std::endl;
            return true;
        }
    }

    std::cerr << "Kein Audio-Track für " << peerId << " gefunden" << std::endl;
    return false;
}

void WebRTCManager::setOnTrackCallback(OnTrackCallback callback) {
    onTrackCallback = std::move(callback);
}

void WebRTCManager::setOnIceCandidateCallback(OnIceCandidateCallback callback) {
    onIceCandidateCallback = std::move(callback);
}

void WebRTCManager::setOnConnectionStateChangeCallback(OnConnectionStateChangeCallback callback) {
    onConnectionStateChangeCallback = std::move(callback);
}

bool WebRTCManager::setLocalDescription(const std::string& peerId, const webrtc::SessionDescriptionInterface* desc) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->peerConnections.find(peerId);
    if (it == pImpl->peerConnections.end()) return false;

    auto result = it->second->SetLocalDescription(desc);
    return result.ok();
}

bool WebRTCManager::setRemoteDescription(const std::string& peerId, const webrtc::SessionDescriptionInterface* desc) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->peerConnections.find(peerId);
    if (it == pImpl->peerConnections.end()) return false;

    auto result = it->second->SetRemoteDescription(desc);
    return result.ok();
}

bool WebRTCManager::addIceCandidate(const std::string& peerId, const std::string& candidate) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "PeerConnection für Peer " << peerId << " nicht gefunden!" << std::endl;
        return false;
    }
    webrtc::SdpParseError error;
    auto iceCandidate = webrtc::CreateIceCandidate("", 0, candidate, &error);
    if (!iceCandidate) {
        std::cerr << "Fehler beim Parsen des ICE-Candidates: " << error.description << std::endl;
        return false;
    }
    if (!it->second->AddIceCandidate(iceCandidate)) {
        std::cerr << "Fehler beim Hinzufügen des ICE-Candidates für Peer " << peerId << std::endl;
        return false;
    }
    std::cout << "ICE-Candidate für Peer " << peerId << " hinzugefügt." << std::endl;
    return true;
}

bool WebRTCManager::createOffer(const std::string& peerId) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "PeerConnection für Peer " << peerId << " nicht gefunden!" << std::endl;
        return false;
    }
    it->second->CreateOffer(
        new rtc::RefCountedObject<CreateSessionDescriptionObserver>(it->second, true),
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()
    );
    std::cout << "Offer für Peer " << peerId << " erstellt." << std::endl;
    return true;
}

bool WebRTCManager::createAnswer(const std::string& peerId) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "PeerConnection für Peer " << peerId << " nicht gefunden!" << std::endl;
        return false;
    }
    it->second->CreateAnswer(
        new rtc::RefCountedObject<CreateSessionDescriptionObserver>(it->second, false),
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions()
    );
    std::cout << "Answer für Peer " << peerId << " erstellt." << std::endl;
    return true;
}

bool WebRTCManager::setRemoteDescription(const std::string& peerId, const std::string& sdp) {
    auto it = peerConnections.find(peerId);
    if (it == peerConnections.end()) {
        std::cerr << "PeerConnection für Peer " << peerId << " nicht gefunden!" << std::endl;
        return false;
    }
    webrtc::SdpParseError error;
    auto desc = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, sdp, &error);
    if (!desc) {
        std::cerr << "Fehler beim Parsen der Remote Description: " << error.description << std::endl;
        return false;
    }
    it->second->SetRemoteDescription(
        new rtc::RefCountedObject<SetSessionDescriptionObserver>(it->second, desc),
        desc
    );
    std::cout << "RemoteDescription für Peer " << peerId << " gesetzt." << std::endl;
    return true;
}

void WebRTCManager::cleanupPeerConnections() {
    for (auto& [peerId, peerConnection] : peerConnections) {
        // Audio-Tracks entfernen
        auto senders = peerConnection->GetSenders();
        for (const auto& sender : senders) {
            if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
                peerConnection->RemoveTrack(sender);
            }
        }

        // Peer-Verbindung schließen
        peerConnection->Close();
    }

    peerConnections.clear();
    std::cout << "Alle Peer-Verbindungen wurden geschlossen und Ressourcen freigegeben" << std::endl;
}

void WebRTCManager::cleanup() {
    try {
        // Peer-Verbindungen schließen
        {
            std::lock_guard<std::shared_mutex> lock(peerConnectionsMutex);
            for (auto& [peerId, peerConnection] : peerConnections) {
                if (peerConnection) {
                    // Audio-Tracks entfernen
                    auto senders = peerConnection->GetSenders();
                    for (const auto& sender : senders) {
                        if (sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
                            peerConnection->RemoveTrack(sender);
                        }
                    }

                    // Peer-Verbindung schließen
                    peerConnection->Close();
                }
            }
            peerConnections.clear();
        }

        // Audio-Processor zurücksetzen
        {
            std::lock_guard<std::mutex> lock(audioProcessorMutex);
            if (audioProcessor) {
                audioProcessor->setEnabled(false);
                audioProcessor.reset();
            }
        }

        // Callbacks zurücksetzen
        {
            std::lock_guard<std::mutex> lock(callbackMutex);
            onIceCandidateCallback = nullptr;
            onTrackCallback = nullptr;
            onConnectionStateChangeCallback = nullptr;
            audioCallback = nullptr;
        }

        // PeerConnectionFactory zurücksetzen
        if (peerConnectionFactory) {
            peerConnectionFactory = nullptr;
        }

        // Fehler zurücksetzen
        {
            std::lock_guard<std::mutex> lock(errorMutex);
            lastError.clear();
            errorLog.clear();
        }

        std::cout << "WebRTC-Manager erfolgreich bereinigt" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fehler bei der Bereinigung des WebRTC-Managers: " << e.what() << std::endl;
        throw WebRTCError(WebRTCError::ErrorCode::ResourceError,
                         "Fehler bei der Bereinigung: " + std::string(e.what()));
    }
}

} // namespace VR_DAW 