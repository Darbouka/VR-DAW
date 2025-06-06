#include "Mixer.hpp"
#include <algorithm>
#include <stdexcept>

namespace VR_DAW {

Mixer::Mixer() : nextTrackId(0) {}

Mixer::~Mixer() = default;

int Mixer::createTrack(const std::string& name) {
    Track track;
    track.id = nextTrackId++;
    track.name = name;
    track.volume = 1.0f;
    track.pan = 0.0f;
    track.muted = false;
    track.solo = false;
    track.buffer.resize(4096); // Standard-Buffer-Größe
    
    tracks.push_back(track);
    return track.id;
}

void Mixer::deleteTrack(int trackId) {
    auto it = std::find_if(tracks.begin(), tracks.end(),
                          [trackId](const Track& track) { return track.id == trackId; });
    if (it != tracks.end()) {
        tracks.erase(it);
    }
}

Track* Mixer::getTrack(int trackId) {
    auto it = std::find_if(tracks.begin(), tracks.end(),
                          [trackId](const Track& track) { return track.id == trackId; });
    return it != tracks.end() ? &(*it) : nullptr;
}

std::vector<Track>& Mixer::getTracks() {
    return tracks;
}

void Mixer::setTrackVolume(int trackId, float volume) {
    Track* track = getTrack(trackId);
    if (track) {
        track->volume = std::max(0.0f, std::min(1.0f, volume));
    }
}

void Mixer::setTrackPan(int trackId, float pan) {
    Track* track = getTrack(trackId);
    if (track) {
        track->pan = std::max(-1.0f, std::min(1.0f, pan));
    }
}

void Mixer::muteTrack(int trackId) {
    Track* track = getTrack(trackId);
    if (track) {
        track->muted = true;
    }
}

void Mixer::soloTrack(int trackId) {
    Track* track = getTrack(trackId);
    if (track) {
        track->solo = true;
        updateTrackStates();
    }
}

void Mixer::unmuteTrack(int trackId) {
    Track* track = getTrack(trackId);
    if (track) {
        track->muted = false;
    }
}

void Mixer::unsoloTrack(int trackId) {
    Track* track = getTrack(trackId);
    if (track) {
        track->solo = false;
        updateTrackStates();
    }
}

void Mixer::process(float* output, unsigned long framesPerBuffer) {
    // Buffer löschen
    std::fill(output, output + framesPerBuffer * 2, 0.0f);
    
    // Tracks mischen
    mixTracks(output, framesPerBuffer);
}

void Mixer::clear() {
    tracks.clear();
    nextTrackId = 0;
}

void Mixer::updateTrackStates() {
    bool anySolo = false;
    for (const auto& track : tracks) {
        if (track.solo) {
            anySolo = true;
            break;
        }
    }
    
    if (anySolo) {
        for (auto& track : tracks) {
            if (!track.solo) {
                track.muted = true;
            }
        }
    }
}

void Mixer::mixTracks(float* output, unsigned long framesPerBuffer) {
    for (const auto& track : tracks) {
        if (track.muted) continue;
        
        // Stereo-Panning
        float leftGain = std::sqrt(2.0f) / 2.0f * (1.0f - track.pan);
        float rightGain = std::sqrt(2.0f) / 2.0f * (1.0f + track.pan);
        
        // Track zum Mix hinzufügen
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            if (i < track.buffer.size() / 2) {
                output[i * 2] += track.buffer[i * 2] * track.volume * leftGain;
                output[i * 2 + 1] += track.buffer[i * 2 + 1] * track.volume * rightGain;
            }
        }
    }
}

} // namespace VR_DAW 