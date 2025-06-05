#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/audio_track_interface.h>
#include <webrtc/api/media_stream_interface.h>

namespace VR_DAW {

class WebRTCManager {
public:
    WebRTCManager();
    ~WebRTCManager();

    bool initialize();
    void shutdown();

    bool createPeerConnection(const std::string& peerId);
    bool addAudioTrack(const std::string& peerId, rtc::scoped_refptr<webrtc::AudioTrackInterface> track);
    bool removeAudioTrack(const std::string& peerId, const std::string& trackId);

    void setOnTrackCallback(std::function<void(const std::string&, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>)> callback);
    void setOnIceCandidateCallback(std::function<void(const std::string&, const webrtc::IceCandidateInterface*)> callback);

    bool setLocalDescription(const std::string& peerId, const webrtc::SessionDescriptionInterface* desc);
    bool setRemoteDescription(const std::string& peerId, const webrtc::SessionDescriptionInterface* desc);
    bool addIceCandidate(const std::string& peerId, const webrtc::IceCandidateInterface* candidate);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace VR_DAW 