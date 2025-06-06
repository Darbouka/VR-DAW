#pragma once

#include <vector>
#include <string>
#include <memory>

namespace VR_DAW {

struct Track {
    int id;
    std::string name;
    float volume;
    float pan;
    bool muted;
    bool solo;
    std::vector<float> buffer;
};

class Mixer {
public:
    Mixer();
    ~Mixer();

    // Track-Management
    int createTrack(const std::string& name);
    void deleteTrack(int trackId);
    Track* getTrack(int trackId);
    std::vector<Track>& getTracks();

    // Track-Kontrolle
    void setTrackVolume(int trackId, float volume);
    void setTrackPan(int trackId, float pan);
    void muteTrack(int trackId);
    void soloTrack(int trackId);
    void unmuteTrack(int trackId);
    void unsoloTrack(int trackId);

    // Mixer-Verarbeitung
    void process(float* output, unsigned long framesPerBuffer);
    void clear();

private:
    std::vector<Track> tracks;
    int nextTrackId;
    
    void updateTrackStates();
    void mixTracks(float* output, unsigned long framesPerBuffer);
};

} // namespace VR_DAW 