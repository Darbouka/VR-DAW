#include "Automation.hpp"
#include <algorithm>
#include <fstream>
#include <json/json.h>

namespace VR_DAW {

Automation& Automation::getInstance() {
    static Automation instance;
    return instance;
}

void Automation::initialize() {
    // Initialisierung der Automation-Komponenten
}

void Automation::shutdown() {
    tracks.clear();
}

void Automation::createTrack(const std::string& id) {
    AutomationTrack track;
    track.id = id;
    tracks.push_back(std::move(track));
}

void Automation::deleteTrack(const std::string& id) {
    auto it = std::find_if(tracks.begin(), tracks.end(),
        [&id](const auto& track) {
            return track.id == id;
        });
    
    if (it != tracks.end()) {
        tracks.erase(it);
    }
}

std::vector<Automation::AutomationTrack> Automation::getTracks() const {
    return tracks;
}

void Automation::createLane(const std::string& trackId, const std::string& parameterId) {
    auto* track = findTrack(trackId);
    if (track) {
        AutomationLane lane;
        lane.id = parameterId;
        lane.parameterId = parameterId;
        lane.enabled = true;
        track->lanes.push_back(std::move(lane));
    }
}

void Automation::deleteLane(const std::string& trackId, const std::string& laneId) {
    auto* track = findTrack(trackId);
    if (track) {
        auto it = std::find_if(track->lanes.begin(), track->lanes.end(),
            [&laneId](const auto& lane) {
                return lane.id == laneId;
            });
        
        if (it != track->lanes.end()) {
            track->lanes.erase(it);
        }
    }
}

void Automation::enableLane(const std::string& trackId, const std::string& laneId, bool enable) {
    auto* track = findTrack(trackId);
    if (track) {
        auto* lane = findLane(*track, laneId);
        if (lane) {
            lane->enabled = enable;
        }
    }
}

void Automation::addPoint(const std::string& trackId, const std::string& laneId,
                         double time, float value, float curve) {
    auto* track = findTrack(trackId);
    if (track) {
        auto* lane = findLane(*track, laneId);
        if (lane) {
            AutomationPoint point{time, value, curve};
            
            // Punkt an der richtigen Stelle einfügen
            auto it = std::lower_bound(lane->points.begin(), lane->points.end(), point,
                [](const auto& p1, const auto& p2) {
                    return p1.time < p2.time;
                });
            
            lane->points.insert(it, point);
        }
    }
}

void Automation::removePoint(const std::string& trackId, const std::string& laneId, double time) {
    auto* track = findTrack(trackId);
    if (track) {
        auto* lane = findLane(*track, laneId);
        if (lane) {
            auto it = std::find_if(lane->points.begin(), lane->points.end(),
                [time](const auto& point) {
                    return std::abs(point.time - time) < 0.0001;
                });
            
            if (it != lane->points.end()) {
                lane->points.erase(it);
            }
        }
    }
}

void Automation::updatePoint(const std::string& trackId, const std::string& laneId,
                           double time, float value, float curve) {
    auto* track = findTrack(trackId);
    if (track) {
        auto* lane = findLane(*track, laneId);
        if (lane) {
            auto it = std::find_if(lane->points.begin(), lane->points.end(),
                [time](const auto& point) {
                    return std::abs(point.time - time) < 0.0001;
                });
            
            if (it != lane->points.end()) {
                it->value = value;
                it->curve = curve;
            }
        }
    }
}

float Automation::getValueAtTime(const std::string& trackId, const std::string& laneId, 
                               double time) const {
    auto* track = findTrack(trackId);
    if (!track) return 0.0f;
    
    auto* lane = findLane(*track, laneId);
    if (!lane || !lane->enabled || lane->points.empty()) return 0.0f;
    
    // Finde die umgebenden Punkte
    auto it = std::lower_bound(lane->points.begin(), lane->points.end(), time,
        [](const auto& point, double t) {
            return point.time < t;
        });
    
    if (it == lane->points.begin()) {
        return lane->points.front().value;
    }
    
    if (it == lane->points.end()) {
        return lane->points.back().value;
    }
    
    // Interpoliere zwischen den Punkten
    const auto& p2 = *it;
    const auto& p1 = *(it - 1);
    
    return interpolateValue(p1, p2, time);
}

void Automation::processAutomation(double currentTime, juce::AudioBuffer<float>& buffer) {
    for (const auto& track : tracks) {
        for (const auto& lane : track.lanes) {
            if (!lane.enabled) continue;
            
            float value = getValueAtTime(track.id, lane.id, currentTime);
            
            // Wende den automatisierten Wert auf den Buffer an
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                buffer.applyGain(channel, 0, buffer.getNumSamples(), value);
            }
        }
    }
}

void Automation::importAutomation(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return;
    
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(file, root)) return;
    
    tracks.clear();
    
    for (const auto& trackJson : root["tracks"]) {
        AutomationTrack track;
        track.id = trackJson["id"].asString();
        
        for (const auto& laneJson : trackJson["lanes"]) {
            AutomationLane lane;
            lane.id = laneJson["id"].asString();
            lane.parameterId = laneJson["parameterId"].asString();
            lane.enabled = laneJson["enabled"].asBool();
            
            for (const auto& pointJson : laneJson["points"]) {
                AutomationPoint point;
                point.time = pointJson["time"].asDouble();
                point.value = pointJson["value"].asFloat();
                point.curve = pointJson["curve"].asFloat();
                lane.points.push_back(point);
            }
            
            track.lanes.push_back(std::move(lane));
        }
        
        tracks.push_back(std::move(track));
    }
}

void Automation::exportAutomation(const std::string& filePath) const {
    Json::Value root;
    Json::Value tracksJson(Json::arrayValue);
    
    for (const auto& track : tracks) {
        Json::Value trackJson;
        trackJson["id"] = track.id;
        
        Json::Value lanesJson(Json::arrayValue);
        for (const auto& lane : track.lanes) {
            Json::Value laneJson;
            laneJson["id"] = lane.id;
            laneJson["parameterId"] = lane.parameterId;
            laneJson["enabled"] = lane.enabled;
            
            Json::Value pointsJson(Json::arrayValue);
            for (const auto& point : lane.points) {
                Json::Value pointJson;
                pointJson["time"] = point.time;
                pointJson["value"] = point.value;
                pointJson["curve"] = point.curve;
                pointsJson.append(pointJson);
            }
            
            laneJson["points"] = pointsJson;
            lanesJson.append(laneJson);
        }
        
        trackJson["lanes"] = lanesJson;
        tracksJson.append(trackJson);
    }
    
    root["tracks"] = tracksJson;
    
    std::ofstream file(filePath);
    if (file.is_open()) {
        Json::StyledWriter writer;
        file << writer.write(root);
    }
}

float Automation::interpolateValue(const AutomationPoint& p1, const AutomationPoint& p2,
                                 double time) const {
    double t = (time - p1.time) / (p2.time - p1.time);
    
    // Bézier-Kurven-Interpolation basierend auf dem curve-Parameter
    if (std::abs(p1.curve) > 0.0001) {
        double c = p1.curve;
        t = t * t * (3 - 2 * t) + c * t * (1 - t) * (1 - t);
    }
    
    return p1.value + (p2.value - p1.value) * static_cast<float>(t);
}

Automation::AutomationTrack* Automation::findTrack(const std::string& id) {
    auto it = std::find_if(tracks.begin(), tracks.end(),
        [&id](const auto& track) {
            return track.id == id;
        });
    
    return it != tracks.end() ? &(*it) : nullptr;
}

Automation::AutomationLane* Automation::findLane(AutomationTrack& track, const std::string& laneId) {
    auto it = std::find_if(track.lanes.begin(), track.lanes.end(),
        [&laneId](const auto& lane) {
            return lane.id == laneId;
        });
    
    return it != track.lanes.end() ? &(*it) : nullptr;
}

} // namespace VR_DAW 