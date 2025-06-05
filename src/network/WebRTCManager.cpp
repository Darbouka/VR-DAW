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

WebRTCManager::WebRTCManager() : pImpl(std::make_unique<Impl>()) {}
WebRTCManager::~WebRTCManager() = default;

bool WebRTCManager::initialize() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    if (pImpl->initialized) return true;

    // WebRTC globales Setup
    rtc::InitializeSSL();
    rtc::ThreadManager::Instance()->WrapCurrentThread();

    // PeerConnectionFactory erstellen
    pImpl->peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
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

    if (!pImpl->peerConnectionFactory) {
        std::cerr << "Failed to create PeerConnectionFactory" << std::endl;
        return false;
    }

    pImpl->initialized = true;
    return true;
}

void WebRTCManager::shutdown() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->peerConnections.clear();
    pImpl->peerConnectionFactory = nullptr;
    pImpl->initialized = false;
    rtc::CleanupSSL();
}

bool WebRTCManager::createPeerConnection(const std::string& peerId) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    if (pImpl->peerConnections.count(peerId)) return false;

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;

    webrtc::PeerConnectionInterface::IceServer iceServer;
    iceServer.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(iceServer);

    auto result = pImpl->peerConnectionFactory->CreatePeerConnection(
        config,
        nullptr,
        nullptr,
        this
    );

    if (!result.ok()) {
        std::cerr << "Failed to create PeerConnection: " << result.error().message() << std::endl;
        return false;
    }

    pImpl->peerConnections[peerId] = result.value();
    return true;
}

bool WebRTCManager::addAudioTrack(const std::string& peerId, rtc::scoped_refptr<webrtc::AudioTrackInterface> track) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->peerConnections.find(peerId);
    if (it == pImpl->peerConnections.end()) return false;

    auto result = it->second->AddTrack(track, {});
    return result.ok();
}

bool WebRTCManager::removeAudioTrack(const std::string& peerId, const std::string& trackId) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->peerConnections.find(peerId);
    if (it == pImpl->peerConnections.end()) return false;

    // TODO: Implement track removal logic
    return true;
}

void WebRTCManager::setOnTrackCallback(std::function<void(const std::string&, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>)> callback) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->onTrackCallback = callback;
}

void WebRTCManager::setOnIceCandidateCallback(std::function<void(const std::string&, const webrtc::IceCandidateInterface*)> callback) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->onIceCandidateCallback = callback;
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

bool WebRTCManager::addIceCandidate(const std::string& peerId, const webrtc::IceCandidateInterface* candidate) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->peerConnections.find(peerId);
    if (it == pImpl->peerConnections.end()) return false;

    auto result = it->second->AddIceCandidate(candidate);
    return result.ok();
}

} // namespace VR_DAW 