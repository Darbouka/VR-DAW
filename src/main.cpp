#include "vr/VRUI.hpp"
#include "audio/AudioEngine.hpp"
#include "plugins/PluginInterface.hpp"
#include "plugins/plugins/ReverbPlugin.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace VR_DAW;

int main() {
    try {
        // Audio-Engine initialisieren
        AudioEngine audioEngine;
        if (!audioEngine.initialize()) {
            std::cerr << "Fehler bei der Audio-Engine-Initialisierung" << std::endl;
            return 1;
        }

        // VR-UI initialisieren
        VRUI vrUI;
        if (!vrUI.initialize()) {
            std::cerr << "Fehler bei der VR-UI-Initialisierung" << std::endl;
            return 1;
        }

        // Reverb-Plugin registrieren
        auto reverbFactory = std::make_unique<ReverbPluginFactory>();
        PluginManager::getInstance().registerPluginFactory(std::move(reverbFactory));

        // Beispiel-Track erstellen
        auto* track = audioEngine.createTrack("Hauptspur");
        auto* reverbPlugin = audioEngine.loadPlugin("Reverb", "Reverb");
        
        if (reverbPlugin) {
            reverbPlugin->setParameter("Room Size", 0.7f);
            reverbPlugin->setParameter("Wet Level", 0.3f);
        }

        // Hauptschleife
        bool running = true;
        while (running) {
            // VR-UI aktualisieren
            vrUI.update();

            // Audio verarbeiten
            float inputBuffer[1024] = {0};
            float outputBuffer[1024] = {0};
            audioEngine.process(inputBuffer, outputBuffer, 1024);

            // VR-UI rendern
            vrUI.render();

            // Kurze Pause
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // Aufräumen
        audioEngine.shutdown();
        vrUI.shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fehler: " << e.what() << std::endl;
        return 1;
    }
} 