#include "AudioEngine.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <portaudio.h>
#include <sndfile.h>
#include <map>
#include <stdexcept>

namespace VR_DAW {

struct AudioEngine::Impl {
    int sampleRate;
    int bufferSize;
    std::vector<Track> tracks;
    std::map<int, std::vector<Plugin>> trackPlugins;
    double currentPosition;
    bool isPlaying;
    double bpm;
    int timeSignatureNumerator;
    int timeSignatureDenominator;
    PaStream* stream;
};

static int paCallback(const void* inputBuffer, void* outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void* userData) {
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    engine->processAudio(out, framesPerBuffer);
    return paContinue;
}

AudioEngine::AudioEngine()
    : stream(nullptr)
    , sampleRate(44100.0)
    , bufferSize(1024)
    , isInitialized(false)
{
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize() {
    if (isInitialized) return true;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio Fehler: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    audioProcessor = std::make_unique<AudioProcessing>();
    automation = std::make_unique<Automation>();
    mixer = std::make_unique<Mixer>();

    isInitialized = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!isInitialized) return;

    stopStream();
    
    PaError err = Pa_Terminate();
    if (err != paNoError) {
        std::cerr << "PortAudio Fehler beim Beenden: " << Pa_GetErrorText(err) << std::endl;
    }

    audioProcessor.reset();
    automation.reset();
    mixer.reset();
    effects.clear();

    isInitialized = false;
}

bool AudioEngine::startStream() {
    if (!isInitialized) return false;

    PaError err = Pa_OpenDefaultStream(&stream,
                                     2,          // Eingangskanäle
                                     2,          // Ausgangskanäle
                                     paFloat32,  // Sample-Format
                                     sampleRate,
                                     bufferSize,
                                     paCallback,
                                     this);

    if (err != paNoError) {
        std::cerr << "Fehler beim Öffnen des Streams: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Fehler beim Starten des Streams: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    return true;
}

void AudioEngine::stopStream() {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }
}

void AudioEngine::processBlock(float* input, float* output, unsigned long framesPerBuffer) {
    if (!isInitialized) return;

    // Audio-Verarbeitung
    audioProcessor->process(input, output, framesPerBuffer);

    // Effekte anwenden
    for (auto& effect : effects) {
        effect->process(output, framesPerBuffer);
    }

    // Automation anwenden
    automation->processBlock(std::vector<float>(output, output + framesPerBuffer));

    // Mixer-Verarbeitung
    mixer->process(output, framesPerBuffer);
}

void AudioEngine::setSampleRate(double rate) {
    sampleRate = rate;
}

void AudioEngine::setBufferSize(int size) {
    bufferSize = size;
}

void AudioEngine::addEffect(std::shared_ptr<Effects> effect) {
    effects.push_back(effect);
}

void AudioEngine::removeEffect(int index) {
    if (index >= 0 && index < effects.size()) {
        effects.erase(effects.begin() + index);
    }
}

void AudioEngine::setAutomation(const std::string& parameter, float value) {
    automation->setParameter(parameter, value);
}

void AudioEngine::setTrackVolume(int track, float volume) {
    mixer->setTrackVolume(track, volume);
}

void AudioEngine::setTrackPan(int track, float pan) {
    mixer->setTrackPan(track, pan);
}

void AudioEngine::muteTrack(int track) {
    mixer->muteTrack(track);
}

void AudioEngine::soloTrack(int track) {
    mixer->soloTrack(track);
}

int AudioEngine::paCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData) {
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    const float* in = static_cast<const float*>(inputBuffer);
    float* out = static_cast<float*>(outputBuffer);

    engine->processBlock(const_cast<float*>(in), out, framesPerBuffer);

    return paContinue;
}

int AudioEngine::createTrack(const std::string& name) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    Track track;
    track.id = pImpl->tracks.size() + 1;
    track.name = name;
    track.volume = 1.0f;
    track.pan = 0.0f;
    track.muted = false;
    track.solo = false;
    
    pImpl->tracks.push_back(track);
    return track.id;
}

bool AudioEngine::deleteTrack(int trackId) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
                          [trackId](const Track& t) { return t.id == trackId; });
    
    if (it != pImpl->tracks.end()) {
        pImpl->tracks.erase(it);
        pImpl->trackPlugins.erase(trackId);
        return true;
    }
    return false;
}

bool AudioEngine::loadAudioFile(int trackId, const std::string& filePath) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    SF_INFO fileInfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &fileInfo);
    if (!file) {
        std::cerr << "Fehler beim Öffnen der Audiodatei: " << sf_strerror(nullptr) << std::endl;
        return false;
    }

    Track* track = getTrack(trackId);
    if (!track) {
        sf_close(file);
        return false;
    }

    track->buffer.data.resize(fileInfo.frames * fileInfo.channels);
    track->buffer.sampleRate = fileInfo.samplerate;
    track->buffer.channels = fileInfo.channels;
    track->buffer.duration = static_cast<double>(fileInfo.frames) / fileInfo.samplerate;

    sf_readf_float(file, track->buffer.data.data(), fileInfo.frames);
    sf_close(file);

    return true;
}

void AudioEngine::processAudio(float* outputBuffer, int numFrames) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    // Buffer zurücksetzen
    std::fill(outputBuffer, outputBuffer + numFrames * 2, 0.0f);

    if (!pImpl->isPlaying) {
        return;
    }

    // Alle Tracks verarbeiten
    for (const auto& track : pImpl->tracks) {
        if (track.muted) continue;
        
        float* tempBuffer = new float[numFrames * 2];
        processTrack(track, tempBuffer, numFrames);
        
        // Zum Ausgabebuffer hinzufügen
        for (int i = 0; i < numFrames * 2; ++i) {
            outputBuffer[i] += tempBuffer[i];
        }
        
        delete[] tempBuffer;
    }

    pImpl->currentPosition += static_cast<double>(numFrames) / pImpl->sampleRate;
}

void AudioEngine::processTrack(const Track& track, float* outputBuffer, int numFrames) {
    // Track-Daten in den Buffer kopieren
    size_t startFrame = static_cast<size_t>(pImpl->currentPosition * track.buffer.sampleRate);
    size_t framesToCopy = std::min(numFrames, 
                                 static_cast<int>(track.buffer.data.size() / track.buffer.channels - startFrame));

    for (size_t i = 0; i < framesToCopy; ++i) {
        for (int ch = 0; ch < track.buffer.channels; ++ch) {
            outputBuffer[i * 2 + ch] = track.buffer.data[(startFrame + i) * track.buffer.channels + ch];
        }
    }

    // Effekte anwenden
    applyEffects(const_cast<Track&>(track), outputBuffer, numFrames);
}

void AudioEngine::applyEffects(Track& track, float* buffer, int numFrames) {
    // Gain
    applyGainToBuffer(buffer, numFrames, track.volume);
    
    // Pan
    applyPanToBuffer(buffer, numFrames, track.pan);
    
    // Plugins
    auto& plugins = pImpl->trackPlugins[track.id];
    for (auto& plugin : plugins) {
        if (plugin.enabled) {
            processPlugin(plugin, buffer, numFrames);
        }
    }
}

void AudioEngine::applyGainToBuffer(float* buffer, int numFrames, float gain) {
    for (int i = 0; i < numFrames * 2; ++i) {
        buffer[i] *= gain;
    }
}

void AudioEngine::applyPanToBuffer(float* buffer, int numFrames, float pan) {
    float leftGain = std::sqrt(2.0f) / 2.0f * (1.0f - pan);
    float rightGain = std::sqrt(2.0f) / 2.0f * (1.0f + pan);
    
    for (int i = 0; i < numFrames; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        buffer[i * 2] = left * leftGain;
        buffer[i * 2 + 1] = right * rightGain;
    }
}

void AudioEngine::play() {
    std::lock_guard<std::mutex> lock(engineMutex);
    if (!pImpl->isPlaying) {
        PaError err = Pa_StartStream(pImpl->stream);
        if (err == paNoError) {
            pImpl->isPlaying = true;
        }
    }
}

void AudioEngine::pause() {
    std::lock_guard<std::mutex> lock(engineMutex);
    if (pImpl->isPlaying) {
        PaError err = Pa_StopStream(pImpl->stream);
        if (err == paNoError) {
            pImpl->isPlaying = false;
        }
    }
}

void AudioEngine::stop() {
    std::lock_guard<std::mutex> lock(engineMutex);
    pause();
    pImpl->currentPosition = 0.0;
}

void AudioEngine::setPosition(double position) {
    std::lock_guard<std::mutex> lock(engineMutex);
    pImpl->currentPosition = position;
}

double AudioEngine::getPosition() const {
    std::lock_guard<std::mutex> lock(engineMutex);
    return pImpl->currentPosition;
}

bool AudioEngine::isPlaying() const {
    std::lock_guard<std::mutex> lock(engineMutex);
    return pImpl->isPlaying;
}

void AudioEngine::setBPM(double bpm) {
    std::lock_guard<std::mutex> lock(engineMutex);
    pImpl->bpm = bpm;
}

double AudioEngine::getBPM() const {
    std::lock_guard<std::mutex> lock(engineMutex);
    return pImpl->bpm;
}

void AudioEngine::setTimeSignature(int numerator, int denominator) {
    std::lock_guard<std::mutex> lock(engineMutex);
    pImpl->timeSignatureNumerator = numerator;
    pImpl->timeSignatureDenominator = denominator;
}

AudioEngine::Track* AudioEngine::getTrack(int trackId) {
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
                          [trackId](const Track& t) { return t.id == trackId; });
    return it != pImpl->tracks.end() ? &(*it) : nullptr;
}

std::vector<AudioEngine::Track> AudioEngine::getTracks() const {
    std::lock_guard<std::mutex> lock(engineMutex);
    return pImpl->tracks;
}

void AudioEngine::processPlugin(Plugin& plugin, float* buffer, int numFrames) {
    // Dummy: Plugin-Verarbeitung noch nicht implementiert
    // Hier könnte z.B. ein VST-Plugin geladen und auf den Buffer angewendet werden
}

} // namespace VR_DAW 