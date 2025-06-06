#include "VRDAW.hpp"
#include "utils/Config.hpp"
#include "utils/Logger.hpp"
#include <stdexcept>
#include <filesystem>

namespace VR_DAW {

VRDAW::VRDAW()
    : initialized(false)
    , audioEngine(std::make_unique<AudioEngine>())
    , midiEngine(std::make_unique<MIDIEngine>())
    , vrSystem(std::make_unique<VRSystem>())
    , vrUI(std::make_unique<VRUI>())
{
    // Konfiguration laden
    loadConfiguration();
}

VRDAW::~VRDAW() {
    LOG_INFO("VRDAW instance destroyed");
    shutdown();
}

bool VRDAW::initialize() {
    if (initialized) return true;

    LOG_INFO("Initialisiere VR-DAW...");

    // Audio-Engine initialisieren
    if (!audioEngine->initialize()) {
        LOG_ERROR("Fehler beim Initialisieren der Audio-Engine");
        return false;
    }

    // MIDI-Engine initialisieren
    if (!midiEngine->initialize()) {
        LOG_ERROR("Fehler beim Initialisieren der MIDI-Engine");
        return false;
    }

    // VR-System initialisieren
    if (!vrSystem->initialize()) {
        LOG_ERROR("Fehler beim Initialisieren des VR-Systems");
        return false;
    }

    // VR-UI initialisieren
    if (!vrUI->initialize()) {
        LOG_ERROR("Fehler beim Initialisieren der VR-UI");
        return false;
    }

    // MIDI-Engine mit Audio-Engine verbinden
    audioEngine->setMIDIEngine(midiEngine);

    // VR-UI mit Audio-Engine verbinden
    connectAudioUI();

    initialized = true;
    LOG_INFO("VR-DAW erfolgreich initialisiert");
    return true;
}

void VRDAW::shutdown() {
    if (!initialized) return;

    LOG_INFO("Beende VR-DAW...");

    // Konfiguration speichern
    saveConfiguration();

    // Ressourcen freigeben
    vrUI.reset();
    vrSystem.reset();
    midiEngine.reset();
    audioEngine.reset();

    initialized = false;
    LOG_INFO("VR-DAW erfolgreich beendet");
}

void VRDAW::update() {
    if (!initialized) return;

    // Audio-Engine aktualisieren
    audioEngine->update();

    // MIDI-Engine aktualisieren
    midiEngine->update();

    // VR-System aktualisieren
    vrSystem->update();

    // VR-UI aktualisieren
    vrUI->update();
}

void VRDAW::render() {
    if (!initialized) return;

    // VR-UI rendern
    vrUI->render();
}

void VRDAW::loadConfiguration() {
    auto& config = Config::getInstance();
    
    // Standardkonfigurationsdatei
    std::string configPath = "config/vrdaw_config.json";
    
    // Prüfen ob Konfigurationsdatei existiert
    if (std::filesystem::exists(configPath)) {
        if (!config.loadFromFile(configPath)) {
            LOG_WARNING("Konnte Konfiguration nicht laden, verwende Standardwerte");
            config.resetToDefaults();
        }
    } else {
        LOG_INFO("Keine Konfigurationsdatei gefunden, erstelle neue mit Standardwerten");
        config.resetToDefaults();
        config.saveToFile(configPath);
    }

    // Konfiguration anwenden
    applyConfiguration();
}

void VRDAW::saveConfiguration() {
    auto& config = Config::getInstance();
    
    // Konfigurationsverzeichnis erstellen falls nicht vorhanden
    std::filesystem::create_directories("config");
    
    // Konfiguration speichern
    if (!config.saveToFile("config/vrdaw_config.json")) {
        LOG_ERROR("Fehler beim Speichern der Konfiguration");
    }
}

void VRDAW::applyConfiguration() {
    auto& config = Config::getInstance();

    // Audio-Konfiguration anwenden
    const auto& audioConfig = config.getAudioConfig();
    audioEngine->setSampleRate(audioConfig.sampleRate);
    audioEngine->setBufferSize(audioConfig.bufferSize);
    audioEngine->setNumChannels(audioConfig.numChannels);
    audioEngine->setMIDIEnabled(audioConfig.enableMIDI);
    audioEngine->setAudioDevice(audioConfig.audioDevice);

    // MIDI-Konfiguration anwenden
    const auto& midiConfig = config.getMIDIConfig();
    midiEngine->setClockEnabled(midiConfig.enableClock);
    midiEngine->setClockPPQ(midiConfig.clockPPQ);
    midiEngine->setTempo(midiConfig.defaultTempo);
    midiEngine->setInputDevices(midiConfig.inputDevices);
    midiEngine->setOutputDevices(midiConfig.outputDevices);

    // VR-Konfiguration anwenden
    const auto& vrConfig = config.getVRConfig();
    vrUI->setRenderScale(vrConfig.renderScale);
    vrUI->setRenderQuality(vrConfig.renderQuality);
    vrUI->enableDebugRendering(vrConfig.enableDebugRendering);
    vrUI->setLayout(vrConfig.defaultLayout);
    vrUI->setUIScale(vrConfig.uiScale);
    vrUI->setDefaultPosition(vrConfig.defaultPosition);

    // Log-Konfiguration anwenden
    const auto& logConfig = config.getLogConfig();
    Logger::getInstance().setLogFile(logConfig.logFile);
    Logger::getInstance().setLogLevel(logConfig.logLevel);
    Logger::getInstance().setConsoleOutput(logConfig.enableConsoleOutput);
    Logger::getInstance().setFileOutput(logConfig.enableFileOutput);
    Logger::getInstance().setMaxLogSize(logConfig.maxLogSize);
    Logger::getInstance().setMaxLogFiles(logConfig.maxLogFiles);

    // Synthesizer-Konfiguration anwenden
    const auto& synthConfig = config.getSynthesizerConfig();
    
    // Synthesizer für jeden Track konfigurieren
    for (const auto& track : tracks) {
        auto synthesizer = track.second->getSynthesizer();
        if (synthesizer) {
            // Allgemeine Einstellungen
            synthesizer->setVolume(synthConfig.defaultVolume);
            synthesizer->setPan(synthConfig.defaultPan);
            synthesizer->setMaxVoices(synthConfig.maxVoices);

            // Oszillatoren konfigurieren
            for (size_t i = 0; i < synthConfig.oscillators.size(); ++i) {
                const auto& osc = synthConfig.oscillators[i];
                synthesizer->setOscillatorType(i, osc.type);
                synthesizer->setOscillatorMix(i, osc.mix);
                synthesizer->setOscillatorDetune(i, osc.detune);
                synthesizer->setOscillatorPhase(i, osc.phase);
                synthesizer->setOscillatorLevel(i, osc.level);
            }

            // Filter konfigurieren
            synthesizer->setFilterType(synthConfig.filter.type);
            synthesizer->setFilterCutoff(synthConfig.filter.cutoff);
            synthesizer->setFilterResonance(synthConfig.filter.resonance);
            synthesizer->setFilterDrive(synthConfig.filter.drive);
            synthesizer->setFilterEnvelopeAmount(synthConfig.filter.envelopeAmount);
            synthesizer->setFilterEnvelopeAttack(synthConfig.filter.envelopeAttack);
            synthesizer->setFilterEnvelopeDecay(synthConfig.filter.envelopeDecay);

            // LFOs konfigurieren
            for (size_t i = 0; i < synthConfig.lfos.size(); ++i) {
                const auto& lfo = synthConfig.lfos[i];
                synthesizer->setLFOWaveform(i, lfo.waveform);
                synthesizer->setLFORate(i, lfo.rate);
                synthesizer->setLFODepth(i, lfo.depth);
                synthesizer->setLFODestination(i, lfo.destination);
            }

            // Hüllkurven konfigurieren
            synthesizer->setAmplitudeEnvelope(
                synthConfig.amplitudeEnvelope.attack,
                synthConfig.amplitudeEnvelope.decay,
                synthConfig.amplitudeEnvelope.sustain,
                synthConfig.amplitudeEnvelope.release
            );

            synthesizer->setFilterEnvelope(
                synthConfig.filterEnvelope.attack,
                synthConfig.filterEnvelope.decay,
                synthConfig.filterEnvelope.sustain,
                synthConfig.filterEnvelope.release
            );

            // Effekte konfigurieren
            if (synthConfig.enableEffects) {
                for (const auto& effect : synthConfig.effects) {
                    if (effect.enabled) {
                        synthesizer->setEffectEnabled(effect.type, true);
                        for (const auto& param : effect.parameters) {
                            synthesizer->setEffectParameter(effect.type, param.first, param.second);
                        }
                    }
                }
            }
        }
    }
}

void VRDAW::connectAudioUI() {
    // Audio-Engine Callbacks
    audioEngine->setMIDICallback([this](const MIDIMessage& msg) {
        // MIDI-Nachrichten an UI weiterleiten
        vrUI->handleMIDIMessage(msg);
    });

    // VR-UI Callbacks
    vrUI->setAudioCallback([this](const AudioEvent& event) {
        // Audio-Events an Engine weiterleiten
        audioEngine->handleAudioEvent(event);
    });
}

int VRDAW::createTrack(const std::string& name) {
    if (!audioEngine) {
        LOG_ERROR("Cannot create track: Audio engine not initialized");
        return -1;
    }

    int trackId = nextTrackId++;
    LOG_INFO("Creating track {} with name '{}'", trackId, name);

    auto track = std::make_shared<AudioTrack>();
    track->setName(name);
    
    tracks[trackId] = track;
    audioEngine->addTrack(track);

    // Synthesizer-Controller erstellen
    auto controller = std::make_unique<SynthesizerController>(track, vrUI.get());
    synthesizerControllers[trackId] = std::move(controller);

    LOG_INFO("Track {} created successfully", trackId);
    return trackId;
}

void VRDAW::deleteTrack(int trackId) {
    auto trackIt = tracks.find(trackId);
    if (trackIt == tracks.end()) {
        LOG_WARNING("Attempted to delete non-existent track {}", trackId);
        return;
    }

    LOG_INFO("Deleting track {}", trackId);

    if (audioEngine) {
        audioEngine->removeTrack(trackIt->second);
    }

    // Synthesizer-Controller entfernen
    synthesizerControllers.erase(trackId);
    
    // Track entfernen
    tracks.erase(trackIt);

    LOG_INFO("Track {} deleted successfully", trackId);
}

std::shared_ptr<AudioTrack> VRDAW::getTrack(int trackId) {
    auto it = tracks.find(trackId);
    if (it == tracks.end()) {
        LOG_DEBUG("Track {} not found", trackId);
        return nullptr;
    }
    return it->second;
}

void VRDAW::setTrackSynthesizer(int trackId, const std::string& type) {
    auto track = getTrack(trackId);
    if (!track) {
        LOG_WARNING("Cannot set synthesizer: Track {} not found", trackId);
        return;
    }

    LOG_INFO("Setting synthesizer type '{}' for track {}", type, trackId);
    track->setSynthesizerType(type);
    updateSynthesizerUI(trackId);
}

void VRDAW::updateSynthesizerUI(int trackId) {
    auto controllerIt = synthesizerControllers.find(trackId);
    if (controllerIt != synthesizerControllers.end()) {
        controllerIt->second->updateUI();
        LOG_DEBUG("Updated synthesizer UI for track {}", trackId);
    }
}

void VRDAW::handleSynthesizerParameterChange(int trackId, const std::string& param, float value) {
    auto controllerIt = synthesizerControllers.find(trackId);
    if (controllerIt != synthesizerControllers.end()) {
        controllerIt->second->handleParameterChange(param, value);
        LOG_DEBUG("Updated synthesizer parameter '{}' to {} for track {}", param, value, trackId);
    }
}

void VRDAW::initializeAudio() {
    LOG_INFO("Initializing audio engine...");
    audioEngine = std::make_unique<AudioEngine>();
    if (!audioEngine->initialize()) {
        throw std::runtime_error("Failed to initialize audio engine");
    }
    LOG_INFO("Audio engine initialized successfully");
}

void VRDAW::initializeMIDI() {
    LOG_INFO("Initializing MIDI engine...");
    midiEngine = std::make_unique<MIDIEngine>();
    if (!midiEngine->initialize()) {
        throw std::runtime_error("Failed to initialize MIDI engine");
    }

    // MIDI-Engine mit Audio-Engine verbinden
    audioEngine->setMIDIEngine(midiEngine.get());
    LOG_INFO("MIDI engine initialized successfully");
}

void VRDAW::initializeVR() {
    LOG_INFO("Initializing VR UI...");
    vrUI = std::make_unique<VRUI>();
    if (!vrUI->initialize()) {
        throw std::runtime_error("Failed to initialize VR UI");
    }
    LOG_INFO("VR UI initialized successfully");
}

void VRDAW::cleanup() {
    LOG_INFO("Cleaning up VRDAW resources...");
    synthesizerControllers.clear();
    tracks.clear();
    vrUI.reset();
    midiEngine.reset();
    audioEngine.reset();
    LOG_INFO("VRDAW cleanup completed");
}

void VRDAW::updateTracks() {
    for (const auto& [trackId, track] : tracks) {
        try {
            // Track-Status aktualisieren
            if (track->isActive()) {
                // Audio-Verarbeitung
                float buffer[1024 * 2]; // Stereo-Buffer
                track->processBlock(buffer, 1024);
            }

            // Synthesizer-UI aktualisieren
            updateSynthesizerUI(trackId);
        } catch (const std::exception& e) {
            LOG_ERROR("Error updating track {}: {}", trackId, e.what());
        }
    }
}

void VRDAW::renderTracks() {
    for (const auto& [trackId, track] : tracks) {
        try {
            // Track-UI rendern
            auto controllerIt = synthesizerControllers.find(trackId);
            if (controllerIt != synthesizerControllers.end()) {
                controllerIt->second->updateUI();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error rendering track {}: {}", trackId, e.what());
        }
    }
}

} // namespace VR_DAW 