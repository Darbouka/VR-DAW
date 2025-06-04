#include "AudioEditing.hpp"
#include <algorithm>
#include <fstream>
#include <json/json.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace VR_DAW {

AudioEditing& AudioEditing::getInstance() {
    static AudioEditing instance;
    return instance;
}

void AudioEditing::initialize() {
    // Initialisierung der Audio-Editing-Komponenten
}

void AudioEditing::shutdown() {
    regions.clear();
    flexMarkers.clear();
    flexModes.clear();
}

void AudioEditing::createRegion(const std::string& sourceId, double startTime, double endTime) {
    AudioRegion region;
    region.id = "region_" + std::to_string(regions.size());
    region.sourceId = sourceId;
    region.startTime = startTime;
    region.endTime = endTime;
    region.offset = 0.0;
    region.gain = 1.0f;
    region.muted = false;
    
    regions.push_back(std::move(region));
}

void AudioEditing::deleteRegion(const std::string& regionId) {
    auto it = std::find_if(regions.begin(), regions.end(),
        [&regionId](const auto& region) {
            return region.id == regionId;
        });
    
    if (it != regions.end()) {
        flexMarkers.erase(regionId);
        flexModes.erase(regionId);
        regions.erase(it);
    }
}

void AudioEditing::moveRegion(const std::string& regionId, double newStartTime) {
    auto* region = findRegion(regionId);
    if (region) {
        double duration = region->endTime - region->startTime;
        region->startTime = newStartTime;
        region->endTime = newStartTime + duration;
        updateRegionTiming(*region);
    }
}

void AudioEditing::resizeRegion(const std::string& regionId, double newStartTime, double newEndTime) {
    auto* region = findRegion(regionId);
    if (region) {
        region->startTime = newStartTime;
        region->endTime = newEndTime;
        updateRegionTiming(*region);
    }
}

void AudioEditing::splitRegion(const std::string& regionId, double splitTime) {
    auto* region = findRegion(regionId);
    if (region && splitTime > region->startTime && splitTime < region->endTime) {
        // Neue Region erstellen
        AudioRegion newRegion = *region;
        newRegion.id = "region_" + std::to_string(regions.size());
        newRegion.startTime = splitTime;
        newRegion.offset = region->offset + (splitTime - region->startTime);
        
        // Ursprüngliche Region anpassen
        region->endTime = splitTime;
        
        // Flex-Marker übertragen
        if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
            std::vector<FlexMarker> newMarkers;
            for (const auto& marker : it->second) {
                if (marker.time >= splitTime) {
                    FlexMarker newMarker = marker;
                    newMarker.time -= splitTime;
                    newMarkers.push_back(newMarker);
                }
            }
            flexMarkers[newRegion.id] = std::move(newMarkers);
        }
        
        // Flex-Modus übertragen
        if (auto it = flexModes.find(regionId); it != flexModes.end()) {
            flexModes[newRegion.id] = it->second;
        }
        
        regions.push_back(std::move(newRegion));
    }
}

void AudioEditing::mergeRegions(const std::string& regionId1, const std::string& regionId2) {
    auto* region1 = findRegion(regionId1);
    auto* region2 = findRegion(regionId2);
    
    if (region1 && region2 && region1->sourceId == region2->sourceId) {
        // Regionen zusammenführen
        region1->endTime = region2->endTime;
        
        // Flex-Marker zusammenführen
        if (auto it1 = flexMarkers.find(regionId1), it2 = flexMarkers.find(regionId2);
            it1 != flexMarkers.end() && it2 != flexMarkers.end()) {
            for (const auto& marker : it2->second) {
                FlexMarker newMarker = marker;
                newMarker.time += region1->endTime - region2->startTime;
                it1->second.push_back(newMarker);
            }
        }
        
        // Region2 löschen
        deleteRegion(regionId2);
    }
}

void AudioEditing::setFlexMode(const std::string& regionId, FlexMode mode) {
    if (findRegion(regionId)) {
        flexModes[regionId] = mode;
    }
}

void AudioEditing::addFlexMarker(const std::string& regionId, double time, double stretch) {
    auto* region = findRegion(regionId);
    if (region && time >= region->startTime && time <= region->endTime) {
        FlexMarker marker{time, stretch, false};
        
        auto& markers = flexMarkers[regionId];
        auto it = std::lower_bound(markers.begin(), markers.end(), marker,
            [](const auto& m1, const auto& m2) {
                return m1.time < m2.time;
            });
        
        markers.insert(it, marker);
    }
}

void AudioEditing::removeFlexMarker(const std::string& regionId, double time) {
    if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
        auto& markers = it->second;
        auto markerIt = std::find_if(markers.begin(), markers.end(),
            [time](const auto& marker) {
                return std::abs(marker.time - time) < 0.0001;
            });
        
        if (markerIt != markers.end()) {
            markers.erase(markerIt);
        }
    }
}

void AudioEditing::updateFlexMarker(const std::string& regionId, double time, double stretch) {
    if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
        auto& markers = it->second;
        auto markerIt = std::find_if(markers.begin(), markers.end(),
            [time](const auto& marker) {
                return std::abs(marker.time - time) < 0.0001;
            });
        
        if (markerIt != markers.end()) {
            markerIt->stretch = stretch;
        }
    }
}

void AudioEditing::lockFlexMarker(const std::string& regionId, double time, bool locked) {
    if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
        auto& markers = it->second;
        auto markerIt = std::find_if(markers.begin(), markers.end(),
            [time](const auto& marker) {
                return std::abs(marker.time - time) < 0.0001;
            });
        
        if (markerIt != markers.end()) {
            markerIt->locked = locked;
        }
    }
}

void AudioEditing::processAudio(juce::AudioBuffer<float>& buffer, double currentTime) {
    for (const auto& region : regions) {
        if (currentTime >= region.startTime && currentTime < region.endTime) {
            renderRegion(region.id, buffer);
        }
    }
}

void AudioEditing::applyFlex(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    auto* region = findRegion(regionId);
    if (!region) return;
    
    auto modeIt = flexModes.find(regionId);
    if (modeIt == flexModes.end()) return;
    
    switch (modeIt->second) {
        case FlexMode::Slicing:
            applySlicing(regionId, buffer);
            break;
        case FlexMode::Stretching:
            applyStretching(regionId, buffer);
            break;
        case FlexMode::Warping:
            applyWarping(regionId, buffer);
            break;
        case FlexMode::Rhythmic:
            applyRhythmic(regionId, buffer);
            break;
    }
}

void AudioEditing::renderRegion(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    auto* region = findRegion(regionId);
    if (!region || region->muted) return;
    
    // Audio-Daten laden
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(juce::File(region->sourceId)));
    
    if (!reader) return;
    
    // Buffer für die Region erstellen
    int numSamples = static_cast<int>((region->endTime - region->startTime) * reader->sampleRate);
    juce::AudioBuffer<float> regionBuffer(reader->numChannels, numSamples);
    
    // Audio-Daten lesen
    reader->read(&regionBuffer, 0, numSamples, region->offset, true, true);
    
    // Effekte anwenden
    if (std::abs(region->pitch) > 0.01f) {
        applyPitchShift(regionBuffer, region->pitch);
    }
    
    if (region->bpm > 0.0f) {
        float originalBPM = 120.0f; // Standard-BPM, sollte aus Metadaten gelesen werden
        applyBPMChange(regionBuffer, originalBPM, region->bpm);
    }
    
    // Flex anwenden
    applyFlex(regionId, regionBuffer);
    
    // Fade-Effekte anwenden
    if (region->fadeInTime > 0.0f) {
        auto curveIt = fadeInCurves.find(regionId);
        FadeCurve curve = curveIt != fadeInCurves.end() ? curveIt->second : FadeCurve::Linear;
        applyFadeIn(regionBuffer, region->fadeInTime, curve);
    }
    
    if (region->fadeOutTime > 0.0f) {
        auto curveIt = fadeOutCurves.find(regionId);
        FadeCurve curve = curveIt != fadeOutCurves.end() ? curveIt->second : FadeCurve::Linear;
        applyFadeOut(regionBuffer, region->fadeOutTime, curve);
    }
    
    // Gain anwenden
    regionBuffer.applyGain(region->gain);
    
    // In den Hauptbuffer kopieren
    int startSample = static_cast<int>((region->startTime) * reader->sampleRate);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        buffer.addFrom(channel, startSample, regionBuffer,
            channel % regionBuffer.getNumChannels(), 0, numSamples);
    }
}

void AudioEditing::importRegions(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return;
    
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(file, root)) return;
    
    regions.clear();
    flexMarkers.clear();
    flexModes.clear();
    
    for (const auto& regionJson : root["regions"]) {
        AudioRegion region;
        region.id = regionJson["id"].asString();
        region.sourceId = regionJson["sourceId"].asString();
        region.startTime = regionJson["startTime"].asDouble();
        region.endTime = regionJson["endTime"].asDouble();
        region.offset = regionJson["offset"].asDouble();
        region.gain = regionJson["gain"].asFloat();
        region.muted = regionJson["muted"].asBool();
        
        regions.push_back(std::move(region));
    }
    
    for (const auto& flexJson : root["flexMarkers"]) {
        std::string regionId = flexJson["regionId"].asString();
        std::vector<FlexMarker> markers;
        
        for (const auto& markerJson : flexJson["markers"]) {
            FlexMarker marker;
            marker.time = markerJson["time"].asDouble();
            marker.stretch = markerJson["stretch"].asDouble();
            marker.locked = markerJson["locked"].asBool();
            markers.push_back(marker);
        }
        
        flexMarkers[regionId] = std::move(markers);
    }
    
    for (const auto& modeJson : root["flexModes"]) {
        std::string regionId = modeJson["regionId"].asString();
        FlexMode mode = static_cast<FlexMode>(modeJson["mode"].asInt());
        flexModes[regionId] = mode;
    }
}

void AudioEditing::exportRegions(const std::string& filePath) const {
    Json::Value root;
    
    Json::Value regionsJson(Json::arrayValue);
    for (const auto& region : regions) {
        Json::Value regionJson;
        regionJson["id"] = region.id;
        regionJson["sourceId"] = region.sourceId;
        regionJson["startTime"] = region.startTime;
        regionJson["endTime"] = region.endTime;
        regionJson["offset"] = region.offset;
        regionJson["gain"] = region.gain;
        regionJson["muted"] = region.muted;
        regionsJson.append(regionJson);
    }
    root["regions"] = regionsJson;
    
    Json::Value flexMarkersJson(Json::arrayValue);
    for (const auto& [regionId, markers] : flexMarkers) {
        Json::Value flexJson;
        flexJson["regionId"] = regionId;
        
        Json::Value markersJson(Json::arrayValue);
        for (const auto& marker : markers) {
            Json::Value markerJson;
            markerJson["time"] = marker.time;
            markerJson["stretch"] = marker.stretch;
            markerJson["locked"] = marker.locked;
            markersJson.append(markerJson);
        }
        
        flexJson["markers"] = markersJson;
        flexMarkersJson.append(flexJson);
    }
    root["flexMarkers"] = flexMarkersJson;
    
    Json::Value flexModesJson(Json::arrayValue);
    for (const auto& [regionId, mode] : flexModes) {
        Json::Value modeJson;
        modeJson["regionId"] = regionId;
        modeJson["mode"] = static_cast<int>(mode);
        flexModesJson.append(modeJson);
    }
    root["flexModes"] = flexModesJson;
    
    std::ofstream file(filePath);
    if (file.is_open()) {
        Json::StyledWriter writer;
        file << writer.write(root);
    }
}

AudioEditing::AudioRegion* AudioEditing::findRegion(const std::string& id) {
    auto it = std::find_if(regions.begin(), regions.end(),
        [&id](const auto& region) {
            return region.id == id;
        });
    
    return it != regions.end() ? &(*it) : nullptr;
}

void AudioEditing::updateRegionTiming(AudioRegion& region) {
    // Flex-Marker aktualisieren
    if (auto it = flexMarkers.find(region.id); it != flexMarkers.end()) {
        for (auto& marker : it->second) {
            if (marker.time < region.startTime) {
                marker.time = region.startTime;
            } else if (marker.time > region.endTime) {
                marker.time = region.endTime;
            }
        }
    }
}

void AudioEditing::processFlexMarkers(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
        const auto& markers = it->second;
        if (markers.empty()) return;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            float* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                double time = static_cast<double>(sample) / buffer.getSampleRate();
                double stretch = calculateStretchFactor(regionId, time);
                
                // Zeitstreckung anwenden
                if (std::abs(stretch - 1.0) > 0.0001) {
                    int newSample = static_cast<int>(sample * stretch);
                    if (newSample >= 0 && newSample < buffer.getNumSamples()) {
                        channelData[sample] = channelData[newSample];
                    }
                }
            }
        }
    }
}

double AudioEditing::calculateStretchFactor(const std::string& regionId, double time) const {
    if (auto it = flexMarkers.find(regionId); it != flexMarkers.end()) {
        const auto& markers = it->second;
        if (markers.empty()) return 1.0;
        
        // Finde die umgebenden Marker
        auto markerIt = std::lower_bound(markers.begin(), markers.end(), time,
            [](const auto& marker, double t) {
                return marker.time < t;
            });
        
        if (markerIt == markers.begin()) {
            return markers.front().stretch;
        }
        
        if (markerIt == markers.end()) {
            return markers.back().stretch;
        }
        
        // Interpoliere zwischen den Markern
        const auto& m2 = *markerIt;
        const auto& m1 = *(markerIt - 1);
        
        double t = (time - m1.time) / (m2.time - m1.time);
        return m1.stretch + (m2.stretch - m1.stretch) * t;
    }
    
    return 1.0;
}

void AudioEditing::applySlicing(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    // Implementierung der Slicing-Funktionalität
    // Hier würde die Logik für das Aufteilen und Neuordnen von Audio-Slices implementiert
}

void AudioEditing::applyStretching(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    // Implementierung der Stretching-Funktionalität
    // Hier würde die Logik für die Zeitstreckung implementiert
}

void AudioEditing::applyWarping(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    // Implementierung der Warping-Funktionalität
    // Hier würde die Logik für die Zeitverzerrung implementiert
}

void AudioEditing::applyRhythmic(const std::string& regionId, juce::AudioBuffer<float>& buffer) {
    // Implementierung der Rhythmic-Funktionalität
    // Hier würde die Logik für die rhythmusbasierte Anpassung implementiert
}

void AudioEditing::setPitch(const std::string& regionId, float semitones) {
    auto* region = findRegion(regionId);
    if (region) {
        region->pitch = semitones;
    }
}

float AudioEditing::getPitch(const std::string& regionId) const {
    auto* region = findRegion(regionId);
    return region ? region->pitch : 0.0f;
}

void AudioEditing::setBPM(const std::string& regionId, float bpm) {
    auto* region = findRegion(regionId);
    if (region) {
        region->bpm = bpm;
    }
}

float AudioEditing::getBPM(const std::string& regionId) const {
    auto* region = findRegion(regionId);
    return region ? region->bpm : 0.0f;
}

void AudioEditing::adjustPitchToNote(const std::string& regionId, const std::string& note) {
    float semitones = noteToSemitones(note);
    setPitch(regionId, semitones);
}

void AudioEditing::setFadeIn(const std::string& regionId, float time, FadeCurve curve) {
    auto* region = findRegion(regionId);
    if (region) {
        region->fadeInTime = time;
        fadeInCurves[regionId] = curve;
    }
}

void AudioEditing::setFadeOut(const std::string& regionId, float time, FadeCurve curve) {
    auto* region = findRegion(regionId);
    if (region) {
        region->fadeOutTime = time;
        fadeOutCurves[regionId] = curve;
    }
}

void AudioEditing::removeFadeIn(const std::string& regionId) {
    auto* region = findRegion(regionId);
    if (region) {
        region->fadeInTime = 0.0f;
        fadeInCurves.erase(regionId);
    }
}

void AudioEditing::removeFadeOut(const std::string& regionId) {
    auto* region = findRegion(regionId);
    if (region) {
        region->fadeOutTime = 0.0f;
        fadeOutCurves.erase(regionId);
    }
}

float AudioEditing::getFadeInTime(const std::string& regionId) const {
    auto* region = findRegion(regionId);
    return region ? region->fadeInTime : 0.0f;
}

float AudioEditing::getFadeOutTime(const std::string& regionId) const {
    auto* region = findRegion(regionId);
    return region ? region->fadeOutTime : 0.0f;
}

void AudioEditing::applyPitchShift(juce::AudioBuffer<float>& buffer, float semitones) {
    if (std::abs(semitones) < 0.01f) return;

    // FFT-basierte Tonhöhenänderung
    const int fftSize = 2048;
    juce::dsp::FFT fft(std::log2(fftSize));
    
    // Buffer in FFT-Domäne transformieren
    std::vector<float> window(fftSize);
    std::vector<float> fftData(fftSize * 2);
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i += fftSize / 2) {
            // Fenster anwenden
            for (int j = 0; j < fftSize; ++j) {
                window[j] = channelData[i + j] * 0.5f * (1.0f - std::cos(2.0f * M_PI * j / (fftSize - 1)));
            }
            
            // FFT durchführen
            std::copy(window.begin(), window.end(), fftData.begin());
            fft.performRealOnlyForwardTransform(fftData.data());
            
            // Phasen verschieben
            float phaseShift = 2.0f * M_PI * semitones / 12.0f;
            for (int j = 0; j < fftSize / 2; ++j) {
                float magnitude = std::sqrt(fftData[j * 2] * fftData[j * 2] + 
                                         fftData[j * 2 + 1] * fftData[j * 2 + 1]);
                float phase = std::atan2(fftData[j * 2 + 1], fftData[j * 2]);
                phase += phaseShift;
                fftData[j * 2] = magnitude * std::cos(phase);
                fftData[j * 2 + 1] = magnitude * std::sin(phase);
            }
            
            // Inverse FFT
            fft.performRealOnlyInverseTransform(fftData.data());
            
            // Ergebnis zurück in den Buffer
            for (int j = 0; j < fftSize; ++j) {
                channelData[i + j] = fftData[j] * 0.5f * (1.0f - std::cos(2.0f * M_PI * j / (fftSize - 1)));
            }
        }
    }
}

void AudioEditing::applyBPMChange(juce::AudioBuffer<float>& buffer, float originalBPM, float targetBPM) {
    if (std::abs(originalBPM - targetBPM) < 0.1f) return;
    
    float ratio = targetBPM / originalBPM;
    
    // Zeitstreckung mit Phase Vocoder
    const int fftSize = 2048;
    const int hopSize = fftSize / 4;
    
    juce::dsp::FFT fft(std::log2(fftSize));
    std::vector<float> window(fftSize);
    std::vector<float> fftData(fftSize * 2);
    std::vector<float> phase(fftSize / 2);
    std::vector<float> prevPhase(fftSize / 2);
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i += hopSize) {
            // Fenster anwenden
            for (int j = 0; j < fftSize; ++j) {
                window[j] = channelData[i + j] * 0.5f * (1.0f - std::cos(2.0f * M_PI * j / (fftSize - 1)));
            }
            
            // FFT durchführen
            std::copy(window.begin(), window.end(), fftData.begin());
            fft.performRealOnlyForwardTransform(fftData.data());
            
            // Phasen aktualisieren
            for (int j = 0; j < fftSize / 2; ++j) {
                float magnitude = std::sqrt(fftData[j * 2] * fftData[j * 2] + 
                                         fftData[j * 2 + 1] * fftData[j * 2 + 1]);
                float currentPhase = std::atan2(fftData[j * 2 + 1], fftData[j * 2]);
                float phaseDiff = currentPhase - prevPhase[j];
                
                // Phasendifferenz auf [-π, π] begrenzen
                while (phaseDiff > M_PI) phaseDiff -= 2.0f * M_PI;
                while (phaseDiff < -M_PI) phaseDiff += 2.0f * M_PI;
                
                phase[j] += phaseDiff * ratio;
                prevPhase[j] = currentPhase;
                
                // Magnitude und Phase zurück in FFT-Daten
                fftData[j * 2] = magnitude * std::cos(phase[j]);
                fftData[j * 2 + 1] = magnitude * std::sin(phase[j]);
            }
            
            // Inverse FFT
            fft.performRealOnlyInverseTransform(fftData.data());
            
            // Ergebnis zurück in den Buffer
            for (int j = 0; j < fftSize; ++j) {
                channelData[i + j] = fftData[j] * 0.5f * (1.0f - std::cos(2.0f * M_PI * j / (fftSize - 1)));
            }
        }
    }
}

void AudioEditing::applyFadeIn(juce::AudioBuffer<float>& buffer, float time, FadeCurve curve) {
    if (time <= 0.0f) return;
    
    int numSamples = static_cast<int>(time * buffer.getSampleRate());
    numSamples = std::min(numSamples, buffer.getNumSamples());
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < numSamples; ++i) {
            float position = static_cast<float>(i) / numSamples;
            float gain = calculateFadeGain(position, curve);
            channelData[i] *= gain;
        }
    }
}

void AudioEditing::applyFadeOut(juce::AudioBuffer<float>& buffer, float time, FadeCurve curve) {
    if (time <= 0.0f) return;
    
    int numSamples = static_cast<int>(time * buffer.getSampleRate());
    numSamples = std::min(numSamples, buffer.getNumSamples());
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < numSamples; ++i) {
            float position = static_cast<float>(i) / numSamples;
            float gain = calculateFadeGain(1.0f - position, curve);
            channelData[buffer.getNumSamples() - numSamples + i] *= gain;
        }
    }
}

float AudioEditing::calculateFadeGain(float position, FadeCurve curve) const {
    switch (curve) {
        case FadeCurve::Linear:
            return position;
            
        case FadeCurve::Exponential:
            return position * position;
            
        case FadeCurve::Logarithmic:
            return std::log10(1.0f + 9.0f * position) / std::log10(10.0f);
            
        case FadeCurve::Cosine:
            return 0.5f * (1.0f - std::cos(M_PI * position));
            
        default:
            return position;
    }
}

float AudioEditing::noteToSemitones(const std::string& note) const {
    // Noten-String parsen (z.B. "C4", "D#3")
    if (note.length() < 2) return 0.0f;
    
    char noteChar = std::toupper(note[0]);
    int octave = note[note.length() - 1] - '0';
    
    // Grundton in Halbtönen
    int semitones = 0;
    switch (noteChar) {
        case 'C': semitones = 0; break;
        case 'D': semitones = 2; break;
        case 'E': semitones = 4; break;
        case 'F': semitones = 5; break;
        case 'G': semitones = 7; break;
        case 'A': semitones = 9; break;
        case 'B': semitones = 11; break;
        default: return 0.0f;
    }
    
    // Vorzeichen berücksichtigen
    if (note.length() > 2) {
        if (note[1] == '#') semitones++;
        else if (note[1] == 'b') semitones--;
    }
    
    // Oktave berücksichtigen
    semitones += (octave - 4) * 12;
    
    return static_cast<float>(semitones);
}

} // namespace VR_DAW 