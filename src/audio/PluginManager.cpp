#include "PluginManager.hpp"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace VR_DAW {

PluginManager::PluginManager() {
    // Initialisiere Plugin-Format-Manager
    formatManager.addDefaultFormats();
    
    // Registriere VST3-Format
    formatManager.addFormat(new juce::VST3PluginFormat());
    
    // Initialisiere Plugin-Scanner
    pluginScanner = std::make_unique<juce::KnownPluginList>();
    pluginScanner->setCustomScanner(std::make_unique<VST3Scanner>());
}

void PluginManager::scanForPlugins() {
    // Scanne Standard-VST3-Verzeichnisse
    juce::FileSearchPath searchPath;
    
    // Windows
    searchPath.add(juce::File::getSpecialLocation(juce::File::globalApplicationsDirectory)
        .getChildFile("VST3"));
    
    // macOS
    searchPath.add(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Audio/Plug-Ins/VST3"));
    
    // Linux
    searchPath.add(juce::File("~/.vst3"));
    
    // Scanne Plugins
    pluginScanner->scanAndAddDirectory(searchPath, true, true);
    
    // Speichere Plugin-Liste
    savePluginList();
}

void PluginManager::loadPlugin(const juce::String& identifier) {
    // Lade Plugin
    auto* format = formatManager.getFormat(0);
    if (format) {
        auto* plugin = format->createInstance(identifier, sampleRate, maxBlockSize);
        if (plugin) {
            plugins.push_back(std::unique_ptr<juce::AudioPluginInstance>(plugin));
            
            // Initialisiere Plugin
            plugin->prepareToPlay(sampleRate, maxBlockSize);
            plugin->setPlayConfigDetails(numChannels, numChannels, sampleRate, maxBlockSize);
        }
    }
}

void PluginManager::unloadPlugin(const juce::String& identifier) {
    // Finde und entferne Plugin
    auto it = std::find_if(plugins.begin(), plugins.end(),
        [&identifier](const auto& plugin) {
            return plugin->getPluginDescription().identifier == identifier;
        });
    
    if (it != plugins.end()) {
        (*it)->releaseResources();
        plugins.erase(it);
    }
}

void PluginManager::savePluginList() {
    // Speichere Plugin-Liste in XML
    juce::XmlElement* xml = pluginScanner->createXml();
    if (xml) {
        xml->writeToFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("VR-DAW/plugins.xml"), "");
        delete xml;
    }
}

void PluginManager::loadPluginList() {
    // Lade Plugin-Liste aus XML
    auto file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("VR-DAW/plugins.xml");
    
    if (file.existsAsFile()) {
        if (auto* xml = juce::XmlDocument::parse(file)) {
            pluginScanner->recreateFromXml(*xml);
            delete xml;
        }
    }
}

class VST3Scanner : public juce::PluginDirectoryScanner {
public:
    VST3Scanner() : juce::PluginDirectoryScanner(pluginList, format, searchPath, true, true) {}
    
    void scanDirectory(const juce::File& directory) {
        searchPath.add(directory);
        scanNextFile(true);
    }
    
private:
    juce::KnownPluginList pluginList;
    juce::VST3PluginFormat format;
    juce::FileSearchPath searchPath;
};

} // namespace VR_DAW 