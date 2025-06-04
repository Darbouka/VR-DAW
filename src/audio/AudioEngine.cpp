#include "AudioEngine.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace VR_DAW {

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

void AudioEngine::initialize() {
    deviceManager = std::make_unique<juce::AudioDeviceManager>();
    processorPlayer = std::make_unique<juce::AudioProcessorPlayer>();
    
    // Audio-Geräte initialisieren
    juce::String error = deviceManager->initialise(
        0,  // maximale Anzahl Eingangskanäle
        2,  // Anzahl Ausgangskanäle
        nullptr,  // Audio-Geräte-Einstellungen
        true  // Standard-Geräte verwenden
    );
    
    if (error.isNotEmpty()) {
        throw std::runtime_error("Fehler bei der Audio-Initialisierung: " + error.toStdString());
    }
    
    // AI-Komponenten initialisieren
    aiMastering = std::make_unique<AIMasteringProcessor>();
    aiMixing = std::make_unique<AIMixingProcessor>();
}

void AudioEngine::shutdown() {
    stopAudioStream();
    deviceManager->closeAudioDevice();
    deviceManager.reset();
    processorPlayer.reset();
    loadedPlugins.clear();
    effects.clear();
}

void AudioEngine::startAudioStream() {
    deviceManager->addAudioCallback(processorPlayer.get());
    processorPlayer->setProcessor(nullptr);  // Hauptprozessor wird später gesetzt
}

void AudioEngine::stopAudioStream() {
    deviceManager->removeAudioCallback(processorPlayer.get());
}

void AudioEngine::loadPlugin(const std::string& path) {
    juce::PluginDescription desc;
    desc.fileOrIdentifier = path;
    
    juce::AudioPluginFormatManager formatManager;
    formatManager.addDefaultFormats();
    
    for (auto* format : formatManager.getFormats()) {
        if (format->getName() == "VST3" || format->getName() == "AudioUnit") {
            format->createInstanceFromDescription(desc, 44100.0, 512, [this](std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error) {
                if (instance != nullptr) {
                    loadedPlugins.push_back(std::move(instance));
                }
            });
        }
    }
}

void AudioEngine::unloadPlugin(const std::string& pluginId) {
    auto it = std::find_if(loadedPlugins.begin(), loadedPlugins.end(),
        [&pluginId](const auto& plugin) {
            return plugin->getName().toStdString() == pluginId;
        });
    
    if (it != loadedPlugins.end()) {
        loadedPlugins.erase(it);
    }
}

void AudioEngine::addEffect(const std::string& effectType) {
    // Effekt-Factory implementieren
    std::unique_ptr<juce::AudioProcessor> effect;
    if (effectType == "Delay") {
        effect = std::make_unique<juce::DelayProcessor>();
    } else if (effectType == "Reverb") {
        effect = std::make_unique<juce::ReverbProcessor>();
    }
    // Weitere Effekte hier hinzufügen
    
    if (effect) {
        effects.push_back(std::move(effect));
    }
}

void AudioEngine::removeEffect(const std::string& effectId) {
    auto it = std::find_if(effects.begin(), effects.end(),
        [&effectId](const auto& effect) {
            return effect->getName().toStdString() == effectId;
        });
    
    if (it != effects.end()) {
        effects.erase(it);
    }
}

void AudioEngine::enableDolbyAtmos(bool enable) {
    dolbyAtmosEnabled = enable;
    if (enable && !atmosProcessor) {
        atmosProcessor = std::make_unique<DolbyAtmosProcessor>();
    }
}

void AudioEngine::configureDolbyAtmos(const std::string& config) {
    if (atmosProcessor) {
        atmosProcessor->configure(config);
    }
}

void AudioEngine::startRecording() {
    juce::File outputFile = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile("VR_DAW_Recording.wav");
    
    recordingStream = std::make_unique<juce::FileOutputStream>(outputFile);
    
    juce::WavAudioFormat wavFormat;
    recordingWriter = std::unique_ptr<juce::AudioFormatWriter>(
        wavFormat.createWriterFor(recordingStream.get(), 44100.0, 2, 16, {}, 0));
}

void AudioEngine::stopRecording() {
    recordingWriter.reset();
    recordingStream.reset();
}

void AudioEngine::saveRecording(const std::string& path) {
    if (recordingWriter) {
        recordingWriter->flush();
    }
}

void AudioEngine::setMasterVolume(float volume) {
    deviceManager->setMasterVolume(volume);
}

void AudioEngine::setChannelVolume(int channel, float volume) {
    // Kanal-Volumen-Implementierung
}

void AudioEngine::setPan(int channel, float pan) {
    // Pan-Implementierung
}

void AudioEngine::applyAIMastering() {
    if (aiMastering) {
        aiMastering->process();
    }
}

void AudioEngine::applyAIMixing() {
    if (aiMixing) {
        aiMixing->process();
    }
}

void AudioEngine::loadSample(const std::string& path) {
    juce::File sampleFile(path);
    if (sampleFile.existsAsFile()) {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        
        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(sampleFile));
            
        if (reader) {
            sampleBank.push_back(std::move(reader));
        }
    }
}

void AudioEngine::updateSampleBank() {
    // Implementierung der automatischen Sample-Bank-Aktualisierung
    // Hier würde die Logik für das Herunterladen und Aktualisieren der Samples implementiert
}

void AudioEngine::bounceToFile(const std::string& path, const std::string& format) {
    juce::File outputFile(path);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    if (auto* audioFormat = formatManager.findFormatForFileExtension(format)) {
        std::unique_ptr<juce::AudioFormatWriter> writer(
            audioFormat->createWriterFor(
                new juce::FileOutputStream(outputFile),
                44100.0,
                2,
                16,
                {},
                0));
                
        if (writer) {
            // Hier würde die Logik für das Bouncing implementiert
        }
    }
}

} // namespace VR_DAW 