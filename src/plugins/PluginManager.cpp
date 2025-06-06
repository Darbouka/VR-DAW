#include "PluginManager.hpp"
#include <juce_audio_processors/juce_audio_processors.h>
#include <filesystem>

namespace VR_DAW {

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::initializeFormats() {
    formatManager = std::make_unique<juce::AudioPluginFormatManager>();
    
    // VST3-Format hinzufügen
    formatManager->addDefaultFormats();
    
    // AU-Format für macOS hinzufügen
    #if JUCE_MAC
    formatManager->addFormat(new juce::AudioUnitPluginFormat());
    #endif
}

void PluginManager::scanForPlugins() {
    if (!formatManager) {
        initializeFormats();
    }
    
    // Standard-Plugin-Verzeichnisse scannen
    std::vector<std::string> pluginDirectories;
    
    #if JUCE_WINDOWS
    pluginDirectories.push_back("C:\\Program Files\\Common Files\\VST3");
    pluginDirectories.push_back("C:\\Program Files\\VSTPlugins");
    #elif JUCE_MAC
    pluginDirectories.push_back("/Library/Audio/Plug-Ins/VST3");
    pluginDirectories.push_back("/Library/Audio/Plug-Ins/Components");
    pluginDirectories.push_back("~/Library/Audio/Plug-Ins/VST3");
    pluginDirectories.push_back("~/Library/Audio/Plug-Ins/Components");
    #endif
    
    for (const auto& directory : pluginDirectories) {
        scanDirectory(directory);
    }
}

void PluginManager::scanDirectory(const std::string& directory) {
    if (!std::filesystem::exists(directory)) {
        return;
    }
    
    for (const auto& format : formatManager->getFormats()) {
        juce::PluginDirectoryScanner scanner(
            [this](const juce::PluginDescription& desc) {
                addPluginToList(std::make_unique<juce::PluginDescription>(desc));
            },
            *format,
            juce::File(directory),
            true,
            juce::File()
        );
        
        juce::String pluginBeingScanned;
        while (scanner.scanNextFile(true, pluginBeingScanned)) {
            // Fortschritt könnte hier aktualisiert werden
        }
    }
}

void PluginManager::addPluginToList(std::unique_ptr<juce::PluginDescription> description) {
    knownPlugins.push_back(std::move(description));
}

std::vector<std::string> PluginManager::getAvailablePlugins() const {
    std::vector<std::string> pluginNames;
    for (const auto& plugin : knownPlugins) {
        pluginNames.push_back(plugin->name.toStdString());
    }
    return pluginNames;
}

bool PluginManager::loadPlugin(const std::string& pluginId) {
    auto it = std::find_if(knownPlugins.begin(), knownPlugins.end(),
        [&pluginId](const auto& plugin) {
            return plugin->name.toStdString() == pluginId;
        });
    
    if (it == knownPlugins.end()) {
        return false;
    }
    
    return true;
}

void PluginManager::unloadPlugin(const std::string& pluginId) {
    // Aktive Instanzen des Plugins beenden
    auto it = activeInstances.begin();
    while (it != activeInstances.end()) {
        if (it->second->getName().toStdString() == pluginId) {
            it = activeInstances.erase(it);
        } else {
            ++it;
        }
    }
}

std::shared_ptr<juce::AudioPluginInstance> PluginManager::createPluginInstance(const std::string& pluginId) {
    auto it = std::find_if(knownPlugins.begin(), knownPlugins.end(),
        [&pluginId](const auto& plugin) {
            return plugin->name.toStdString() == pluginId;
        });
    
    if (it == knownPlugins.end()) {
        return nullptr;
    }
    
    std::string instanceId = generateInstanceId();
    std::shared_ptr<juce::AudioPluginInstance> instance;
    
    juce::String errorMessage;
    instance = std::shared_ptr<juce::AudioPluginInstance>(
        formatManager->createPluginInstance(**it, 44100.0, 512, errorMessage)
    );
    
    if (instance) {
        activeInstances[instanceId] = instance;
        return instance;
    }
    
    return nullptr;
}

void PluginManager::destroyPluginInstance(const std::string& instanceId) {
    activeInstances.erase(instanceId);
}

void PluginManager::setParameter(const std::string& instanceId, int parameterIndex, float value) {
    auto it = activeInstances.find(instanceId);
    if (it != activeInstances.end()) {
        if (auto* param = it->second->getParameters()[parameterIndex]) {
            param->setValue(value);
        }
    }
}

float PluginManager::getParameter(const std::string& instanceId, int parameterIndex) const {
    auto it = activeInstances.find(instanceId);
    if (it != activeInstances.end()) {
        if (auto* param = it->second->getParameters()[parameterIndex]) {
            return param->getValue();
        }
    }
    return 0.0f;
}

std::vector<juce::AudioProcessorParameter*> PluginManager::getParameters(const std::string& instanceId) const {
    auto it = activeInstances.find(instanceId);
    if (it != activeInstances.end()) {
        return it->second->getParameters();
    }
    return {};
}

bool PluginManager::supportsVST3() const {
    return formatManager && formatManager->getFormats().size() > 0;
}

bool PluginManager::supportsAU() const {
    #if JUCE_MAC
    return formatManager && formatManager->getFormats().size() > 1;
    #else
    return false;
    #endif
}

bool PluginManager::supportsVST2() const {
    return formatManager && formatManager->getFormats().size() > 0;
}

bool PluginManager::isPluginLoaded(const std::string& pluginId) const {
    return std::find_if(knownPlugins.begin(), knownPlugins.end(),
        [&pluginId](const auto& plugin) {
            return plugin->name.toStdString() == pluginId;
        }) != knownPlugins.end();
}

bool PluginManager::isInstanceValid(const std::string& instanceId) const {
    return activeInstances.find(instanceId) != activeInstances.end();
}

std::string PluginManager::generateInstanceId() const {
    static int counter = 0;
    return "instance_" + std::to_string(++counter);
}

void PluginManager::cleanupInvalidInstances() {
    auto it = activeInstances.begin();
    while (it != activeInstances.end()) {
        if (!it->second) {
            it = activeInstances.erase(it);
        } else {
            ++it;
        }
    }
}

void PluginManager::initialize() {
    try {
        // Plugin-Verzeichnisse scannen
        if (!scanPluginDirectories()) {
            throw PluginError("Fehler beim Scannen der Plugin-Verzeichnisse");
        }
        
        // Plugins laden
        if (!loadPlugins()) {
            throw PluginError("Fehler beim Laden der Plugins");
        }
        
        // Plugin-Validierung durchführen
        if (!validatePlugins()) {
            throw PluginError("Fehler bei der Plugin-Validierung");
        }
        
        initialized = true;
    } catch (const PluginError& e) {
        logError("Plugin-Initialisierungsfehler: " + std::string(e.what()));
        cleanup();
        throw;
    }
}

void PluginManager::processPlugin(Plugin& plugin) {
    try {
        if (!initialized) {
            throw PluginError("Plugin-Manager ist nicht initialisiert");
        }
        
        // Plugin-Status überprüfen
        if (!plugin.isValid()) {
            throw PluginError("Ungültiger Plugin-Status");
        }
        
        // Plugin-Parameter validieren
        if (!validatePluginParameters(plugin)) {
            throw PluginError("Ungültige Plugin-Parameter");
        }
        
        // Plugin verarbeiten
        if (!plugin.process()) {
            throw PluginError("Fehler bei der Plugin-Verarbeitung");
        }
    } catch (const PluginError& e) {
        logError("Plugin-Verarbeitungsfehler: " + std::string(e.what()));
        handlePluginError(e);
    }
}

void PluginManager::handlePluginError(const PluginError& error) {
    // Fehler protokollieren
    logError("Plugin-Fehler: " + std::string(error.what()));
    
    // Fehlerbehandlung basierend auf Fehlertyp
    if (error.getType() == PluginError::Type::LoadError) {
        // Plugin neu laden
        reloadPlugin(error.getPluginId());
    } else if (error.getType() == PluginError::Type::ProcessError) {
        // Plugin zurücksetzen
        resetPlugin(error.getPluginId());
    } else if (error.getType() == PluginError::Type::ParameterError) {
        // Parameter zurücksetzen
        resetPluginParameters(error.getPluginId());
    }
    
    // Fehler an Error-Handler weiterleiten
    if (errorHandler) {
        errorHandler(error);
    }
}

bool PluginManager::validatePluginParameters(const Plugin& plugin) {
    try {
        // Parameter-Bereich überprüfen
        for (const auto& param : plugin.getParameters()) {
            if (!isParameterInRange(param)) {
                throw PluginError("Parameter außerhalb des gültigen Bereichs: " + param.getName());
            }
        }
        
        // Parameter-Abhängigkeiten überprüfen
        if (!validateParameterDependencies(plugin)) {
            throw PluginError("Ungültige Parameter-Abhängigkeiten");
        }
        
        return true;
    } catch (const PluginError& e) {
        logError("Parameter-Validierungsfehler: " + std::string(e.what()));
        return false;
    }
}

void PluginManager::reloadPlugin(int pluginId) {
    try {
        // Plugin entladen
        unloadPlugin(pluginId);
        
        // Kurze Pause
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Plugin neu laden
        if (!loadPlugin(pluginId)) {
            throw PluginError("Fehler beim erneuten Laden des Plugins");
        }
        
        // Plugin initialisieren
        if (!initializePlugin(pluginId)) {
            throw PluginError("Fehler beim erneuten Initialisieren des Plugins");
        }
    } catch (const PluginError& e) {
        logError("Fehler beim erneuten Laden des Plugins: " + std::string(e.what()));
        throw;
    }
}

} // namespace VR_DAW 