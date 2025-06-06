#include "AudioEngine.hpp"
#include "../utils/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <immintrin.h> // Für SIMD-Instruktionen
#include <thread>
#include <chrono>
#include <mutex>

namespace VR_DAW {

struct AudioEngine::Impl {
    std::vector<AudioTrack> tracks;
    std::vector<AudioPlugin> plugins;
    PaStream* stream;
    jack_client_t* jackClient;
    bool isPlaying;
    double playbackPosition;
    float masterVolume;
    int sampleRate;
    int bufferSize;
    std::map<int, SynthesizerConfig> synthesizers;
    AudioCallback audioCallback;
    bool initialized;
    std::mutex mutex;
};

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::AudioEngine()
    : pImpl(std::make_unique<Impl>())
    , initialized(false)
    , sampleRate(44100)
    , bufferSize(1024)
    , masterVolume(1.0f)
    , isPlaying(false)
    , playbackPosition(0.0)
    , stream(nullptr)
    , jackClient(nullptr)
    , threadCount(std::thread::hardware_concurrency())
    , simdEnabled(true)
    , processingMode(ProcessingMode::MultiThreaded)
    , shouldProcess(false)
{
}

AudioEngine::~AudioEngine() {
    shutdown();
}

void AudioEngine::initialize() {
    try {
        // Audio-Geräte initialisieren
        if (!initializeAudioDevices()) {
            throw AudioError("Fehler beim Initialisieren der Audio-Geräte");
        }
        
        // Audio-Buffer initialisieren
        if (!initializeBuffers()) {
            throw AudioError("Fehler beim Initialisieren der Audio-Buffer");
        }
        
        // Audio-Thread starten
        if (!startAudioThread()) {
            throw AudioError("Fehler beim Starten des Audio-Threads");
        }
        
        initialized = true;
    } catch (const AudioError& e) {
        logError("Audio-Initialisierungsfehler: " + std::string(e.what()));
        cleanup();
        throw;
    }
}

void AudioEngine::shutdown() {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->synthesizers.clear();
    pImpl->initialized = false;
    Logger::getInstance().log(LogLevel::Info, "AudioEngine heruntergefahren");

    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }

    if (jackClient) {
        jack_client_close(jackClient);
        jackClient = nullptr;
    }

    Pa_Terminate();
}

void AudioEngine::update() {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    // Hier könnte die Aktualisierung der Synthesizer-Parameter erfolgen
}

void AudioEngine::createSynthesizer(int trackId, const SynthesizerConfig& config) {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->synthesizers[trackId] = config;
    Logger::getInstance().log(LogLevel::Info, "Synthesizer für Track " + std::to_string(trackId) + " erstellt");
}

void AudioEngine::updateSynthesizer(int trackId, const SynthesizerConfig& config) {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->synthesizers.find(trackId);
    if (it != pImpl->synthesizers.end()) {
        it->second = config;
        Logger::getInstance().log(LogLevel::Info, "Synthesizer für Track " + std::to_string(trackId) + " aktualisiert");
    }
}

void AudioEngine::deleteSynthesizer(int trackId) {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->synthesizers.erase(trackId);
    Logger::getInstance().log(LogLevel::Info, "Synthesizer für Track " + std::to_string(trackId) + " gelöscht");
}

void AudioEngine::processAudio() {
    try {
        if (!initialized) {
            throw AudioError("Audio-Engine ist nicht initialisiert");
        }
        
        // Audio-Daten verarbeiten
        for (auto& track : pImpl->tracks) {
            if (!processTrack(track)) {
                throw AudioError("Fehler beim Verarbeiten des Tracks: " + track.getName());
            }
        }
        
        // Effekte anwenden
        for (auto& effect : effects) {
            if (!applyEffect(effect)) {
                throw AudioError("Fehler beim Anwenden des Effekts: " + effect.getName());
            }
        }
        
        // Mixing durchführen
        if (!mixTracks()) {
            throw AudioError("Fehler beim Mixing der Tracks");
        }
    } catch (const AudioError& e) {
        logError("Audio-Verarbeitungsfehler: " + std::string(e.what()));
        handleAudioError(e);
    }
}

void AudioEngine::handleAudioError(const AudioError& error) {
    // Fehler protokollieren
    logError("Audio-Fehler: " + std::string(error.what()));
    
    // Fehlerbehandlung basierend auf Fehlertyp
    if (error.getType() == AudioError::Type::DeviceError) {
        // Gerät neu initialisieren
        reinitializeDevice(error.getDeviceId());
    } else if (error.getType() == AudioError::Type::BufferError) {
        // Buffer leeren und neu initialisieren
        clearBuffers();
        initializeBuffers();
    } else if (error.getType() == AudioError::Type::ProcessingError) {
        // Verarbeitung zurücksetzen
        resetProcessing();
    }
    
    // Fehler an Error-Handler weiterleiten
    if (errorHandler) {
        errorHandler(error);
    }
}

bool AudioEngine::processTrack(AudioTrack& track) {
    try {
        // Track-Status überprüfen
        if (!track.isValid()) {
            throw AudioError("Ungültiger Track-Status");
        }
        
        // Audio-Daten laden
        if (!loadTrackData(track)) {
            throw AudioError("Fehler beim Laden der Track-Daten");
        }
        
        // Effekte anwenden
        for (auto& effect : track.getEffects()) {
            if (!applyTrackEffect(track, effect)) {
                throw AudioError("Fehler beim Anwenden des Track-Effekts");
            }
        }
        
        return true;
    } catch (const AudioError& e) {
        logError("Track-Verarbeitungsfehler: " + std::string(e.what()));
        return false;
    }
}

void AudioEngine::reinitializeDevice(int deviceId) {
    try {
        // Gerät schließen
        closeDevice(deviceId);
        
        // Kurze Pause
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Gerät neu öffnen
        if (!openDevice(deviceId)) {
            throw AudioError("Fehler beim erneuten Öffnen des Audio-Geräts");
        }
        
        // Gerät neu initialisieren
        if (!initializeDevice(getDeviceInfo(deviceId))) {
            throw AudioError("Fehler beim erneuten Initialisieren des Audio-Geräts");
        }
    } catch (const AudioError& e) {
        logError("Fehler beim erneuten Initialisieren des Audio-Geräts: " + std::string(e.what()));
        throw;
    }
}

void AudioEngine::handleAudioEvent(const AudioEvent& event) {
    if (!pImpl || !pImpl->initialized) return;
    
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    auto it = pImpl->synthesizers.find(event.channel);
    if (it != pImpl->synthesizers.end()) {
        // Synthesizer-Parameter basierend auf Audio-Event aktualisieren
        switch (event.type) {
            case AudioEvent::Type::NoteOn:
                // Note-On verarbeiten
                break;
            case AudioEvent::Type::NoteOff:
                // Note-Off verarbeiten
                break;
            case AudioEvent::Type::ControlChange:
                // Control-Change verarbeiten
                break;
            case AudioEvent::Type::PitchBend:
                // Pitch-Bend verarbeiten
                break;
            default:
                break;
        }
    }
}

void AudioEngine::setAudioCallback(AudioCallback callback) {
    if (!pImpl) return;
    pImpl->audioCallback = callback;
}

void AudioEngine::process(float* input, float* output, unsigned long frameCount) {
    if (!initialized || !isPlaying) return;

    // Audio-Verarbeitung für jeden Track
    for (auto& track : pImpl->tracks) {
        if (track.muted) continue;
        
        // Track-Buffer mit Effekten verarbeiten
        applyEffects(track, output, frameCount);
        
        // Volume und Pan anwenden
        for (unsigned long i = 0; i < frameCount; ++i) {
            output[i] *= track.volume * masterVolume;
            // TODO: Pan-Implementierung
        }
    }
}

AudioEngine::AudioTrack* AudioEngine::createTrack(const std::string& name) {
    AudioTrack track;
    track.id = pImpl->tracks.size();
    track.name = name;
    track.volume = 1.0f;
    track.pan = 0.0f;
    track.muted = false;
    track.soloed = false;
    
    pImpl->tracks.push_back(track);
    return &pImpl->tracks.back();
}

void AudioEngine::deleteTrack(int trackId) {
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
        [trackId](const AudioTrack& t) { return t.id == trackId; });
    
    if (it != pImpl->tracks.end()) {
        pImpl->tracks.erase(it);
    }
}

void AudioEngine::updateTrack(AudioTrack* track) {
    if (!track) return;
    
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
        [track](const AudioTrack& t) { return t.id == track->id; });
    
    if (it != pImpl->tracks.end()) {
        *it = *track;
    }
}

AudioEngine::AudioPlugin* AudioEngine::loadPlugin(const std::string& name, const std::string& type) {
    AudioPlugin plugin;
    plugin.id = pImpl->plugins.size();
    plugin.name = name;
    plugin.type = type;
    
    pImpl->plugins.push_back(plugin);
    return &pImpl->plugins.back();
}

void AudioEngine::unloadPlugin(int pluginId) {
    auto it = std::find_if(pImpl->plugins.begin(), pImpl->plugins.end(),
        [pluginId](const AudioPlugin& p) { return p.id == pluginId; });
    
    if (it != pImpl->plugins.end()) {
        pImpl->plugins.erase(it);
    }
}

void AudioEngine::setPluginParameter(int pluginId, const std::string& paramName, float value) {
    auto it = std::find_if(pImpl->plugins.begin(), pImpl->plugins.end(),
        [pluginId](const AudioPlugin& p) { return p.id == pluginId; });
    
    if (it != pImpl->plugins.end()) {
        it->parameters[paramName] = value;
    }
}

void AudioEngine::startPlayback() {
    if (!initialized) return;
    
    isPlaying = true;
    if (stream) {
        Pa_StartStream(stream);
    }
}

void AudioEngine::stopPlayback() {
    if (!initialized) return;
    
    isPlaying = false;
    if (stream) {
        Pa_StopStream(stream);
    }
    playbackPosition = 0.0;
}

void AudioEngine::pausePlayback() {
    if (!initialized) return;
    
    isPlaying = false;
    if (stream) {
        Pa_StopStream(stream);
    }
}

void AudioEngine::setPlaybackPosition(double position) {
    playbackPosition = std::max(0.0, position);
}

void AudioEngine::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(1.0f, volume));
}

void AudioEngine::setSampleRate(int rate) {
    if (rate > 0) {
        sampleRate = rate;
    }
}

void AudioEngine::setBufferSize(int size) {
    if (size > 0) {
        bufferSize = size;
    }
}

std::vector<float> AudioEngine::getWaveform(int trackId, int channel) {
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
        [trackId](const AudioTrack& t) { return t.id == trackId; });
    
    if (it != pImpl->tracks.end()) {
        return it->buffer;
    }
    return std::vector<float>();
}

void AudioEngine::updateWaveform(int trackId, const std::vector<float>& data) {
    auto it = std::find_if(pImpl->tracks.begin(), pImpl->tracks.end(),
        [trackId](const AudioTrack& t) { return t.id == trackId; });
    
    if (it != pImpl->tracks.end()) {
        it->buffer = data;
    }
}

void AudioEngine::initializePortAudio() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error("PortAudio initialization failed");
    }

    err = Pa_OpenDefaultStream(&stream,
                             0,          // Keine Eingabekanäle
                             2,          // Stereo-Ausgabe
                             paFloat32,  // 32-bit floating point
                             sampleRate,
                             bufferSize,
                             nullptr,    // Kein Callback
                             nullptr);   // Keine Callback-Daten

    if (err != paNoError) {
        throw std::runtime_error("Failed to open PortAudio stream");
    }
}

void AudioEngine::initializeJack() {
    jackClient = jack_client_open("VR_DAW", JackNoStartServer, nullptr);
    if (!jackClient) {
        throw std::runtime_error("Failed to create JACK client");
    }

    jack_set_process_callback(jackClient,
        [](jack_nframes_t nframes, void* arg) -> int {
            auto* engine = static_cast<AudioEngine*>(arg);
            // TODO: JACK-Audio-Verarbeitung
            return 0;
        }, this);

    if (jack_activate(jackClient) != 0) {
        throw std::runtime_error("Failed to activate JACK client");
    }
}

void AudioEngine::processAudio(float* input, float* output, unsigned long frameCount) {
    // TODO: Implementierung der Audio-Verarbeitung
}

void AudioEngine::applyEffects(AudioTrack& track, float* buffer, unsigned long frameCount) {
    // TODO: Implementierung der Effekt-Verarbeitung
}

void AudioEngine::initializeThreads() {
    shouldProcess = true;
    for (int i = 0; i < threadCount; ++i) {
        processingThreads.emplace_back([this]() {
            while (shouldProcess) {
                AudioBuffer buffer;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    queueCondition.wait(lock, [this]() {
                        return !bufferQueue.empty() || !shouldProcess;
                    });

                    if (!shouldProcess) break;

                    buffer = bufferQueue.front();
                    bufferQueue.pop();
                }

                processBuffer(buffer);
            }
        });
    }
}

void AudioEngine::cleanupThreads() {
    shouldProcess = false;
    queueCondition.notify_all();

    for (auto& thread : processingThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    processingThreads.clear();
}

void AudioEngine::processBuffer(AudioBuffer& buffer) {
    if (buffer.isLocked) return;

    switch (processingMode) {
        case ProcessingMode::SingleThreaded:
            processAudioSingleThreaded(buffer);
            break;
        case ProcessingMode::MultiThreaded:
            processAudioMultiThreaded(buffer);
            break;
        case ProcessingMode::SIMD:
            processAudioSIMD(buffer);
            break;
        case ProcessingMode::GPU:
            processAudioGPU(buffer);
            break;
    }
}

void AudioEngine::processAudioSIMD(AudioBuffer& buffer) {
    if (!simdEnabled) {
        processAudioSingleThreaded(buffer);
        return;
    }

    // SIMD-optimierte Audio-Verarbeitung
    const int simdWidth = 8; // Für AVX2
    const int numSamples = buffer.size;
    const int numSIMDIterations = numSamples / simdWidth;

    for (int i = 0; i < numSIMDIterations; ++i) {
        __m256 input = _mm256_loadu_ps(&buffer.data[i * simdWidth]);
        __m256 processed = _mm256_mul_ps(input, _mm256_set1_ps(masterVolume));
        _mm256_storeu_ps(&buffer.data[i * simdWidth], processed);
    }

    // Restliche Samples verarbeiten
    for (int i = numSIMDIterations * simdWidth; i < numSamples; ++i) {
        buffer.data[i] *= masterVolume;
    }
}

void AudioEngine::processAudioMultiThreaded(AudioBuffer& buffer) {
    const int numSamples = buffer.size;
    const int samplesPerThread = numSamples / threadCount;

    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        const int start = i * samplesPerThread;
        const int end = (i == threadCount - 1) ? numSamples : (i + 1) * samplesPerThread;

        threads.emplace_back([this, &buffer, start, end]() {
            for (int j = start; j < end; ++j) {
                buffer.data[j] *= masterVolume;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void AudioEngine::processAudioSingleThreaded(AudioBuffer& buffer) {
    const int numSamples = buffer.size;
    for (int i = 0; i < numSamples; ++i) {
        buffer.data[i] *= masterVolume;
    }
}

void AudioEngine::processAudioGPU(AudioBuffer& buffer) {
    // TODO: GPU-Beschleunigung implementieren
    processAudioSIMD(buffer);
}

void AudioEngine::optimizeBufferSize() {
    // Optimale Buffer-Größe basierend auf System-Performance berechnen
    const int minBufferSize = 256;
    const int maxBufferSize = 4096;
    const int targetLatency = 10; // ms

    int optimalSize = (sampleRate * targetLatency) / 1000;
    optimalSize = std::max(minBufferSize, std::min(maxBufferSize, optimalSize));
    
    // Auf nächste Potenz von 2 runden
    optimalSize = 1 << (32 - __builtin_clz(optimalSize - 1));
    
    setBufferSize(optimalSize);
}

void AudioEngine::setThreadCount(int count) {
    if (count > 0) {
        threadCount = count;
        if (initialized) {
            cleanupThreads();
            initializeThreads();
        }
    }
}

void AudioEngine::enableSIMD(bool enable) {
    simdEnabled = enable;
}

void AudioEngine::setProcessingMode(ProcessingMode mode) {
    processingMode = mode;
}

void AudioEngine::setMIDIEngine(std::shared_ptr<MIDIEngine> engine) {
    std::lock_guard<std::mutex> lock(midiMutex);
    midiEngine = engine;
    
    if (midiEngine) {
        midiEngine->setMessageCallback([this](const MIDIEngine::MIDIMessage& msg) {
            handleMIDIMessage(msg);
        });
    }
}

void AudioEngine::processMIDIInput() {
    if (!midiEngine) return;

    std::lock_guard<std::mutex> lock(midiMutex);
    midiEngine->processMessages();
}

void AudioEngine::handleMIDIMessage(const MIDIEngine::MIDIMessage& message) {
    if (!midiEngine) return;

    std::lock_guard<std::mutex> lock(midiMutex);

    // MIDI-Nachricht aufzeichnen, wenn aktiv
    if (midiRecordingActive) {
        recordedMIDI.push_back(message);
    }

    // MIDI-Callback aufrufen
    if (midiCallback) {
        midiCallback(message);
    }

    // MIDI-Nachricht an aktive Tracks weiterleiten
    for (auto& track : pImpl->tracks) {
        if (track.isActive && track.midiEnabled) {
            switch (message.type) {
                case MIDIEngine::MIDIMessage::Type::NoteOn:
                    // Note-On an Track senden
                    track.processMIDINoteOn(message.channel, message.data1, message.data2);
                    break;
                case MIDIEngine::MIDIMessage::Type::NoteOff:
                    // Note-Off an Track senden
                    track.processMIDINoteOff(message.channel, message.data1, message.data2);
                    break;
                case MIDIEngine::MIDIMessage::Type::ControlChange:
                    // Control Change an Track senden
                    track.processMIDIControlChange(message.channel, message.data1, message.data2);
                    break;
                case MIDIEngine::MIDIMessage::Type::PitchBend:
                    // Pitch Bend an Track senden
                    track.processMIDIPitchBend(message.channel, 
                        (message.data2 << 7) | message.data1);
                    break;
                default:
                    break;
            }
        }
    }
}

void AudioEngine::startMIDIRecording() {
    std::lock_guard<std::mutex> lock(midiMutex);
    midiRecordingActive = true;
    recordedMIDI.clear();
    
    if (midiEngine) {
        midiEngine->startRecording();
    }
}

void AudioEngine::stopMIDIRecording() {
    std::lock_guard<std::mutex> lock(midiMutex);
    midiRecordingActive = false;
    
    if (midiEngine) {
        midiEngine->stopRecording();
    }
}

bool AudioEngine::isMIDIRecording() const {
    return midiRecordingActive;
}

std::vector<MIDIEngine::MIDIMessage> AudioEngine::getRecordedMIDI() const {
    std::lock_guard<std::mutex> lock(midiMutex);
    return recordedMIDI;
}

void AudioEngine::clearMIDIRecording() {
    std::lock_guard<std::mutex> lock(midiMutex);
    recordedMIDI.clear();
    
    if (midiEngine) {
        midiEngine->clearRecording();
    }
}

void AudioEngine::setMIDICallback(std::function<void(const MIDIEngine::MIDIMessage&)> callback) {
    std::lock_guard<std::mutex> lock(midiMutex);
    midiCallback = callback;
}

} // namespace VR_DAW 