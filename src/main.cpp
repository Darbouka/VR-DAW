#include <JuceHeader.h>
#include "audio/AudioEngine.hpp"
#include "vr/VRInterface.hpp"
#include "ui/VRUI.hpp"
#include "video/VideoEngine.hpp"
#include "utils/Logger.hpp"
#include "utils/Config.hpp"

class VRDAWApplication : public juce::JUCEApplication
{
public:
    VRDAWApplication() = default;

    const juce::String getApplicationName() override { return "VR-DAW"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override
    {
        try
        {
            // Initialisiere Logger
            Logger::getInstance().initialize("vrdaw.log");
            Logger::getInstance().log(LogLevel::Info, "VR-DAW wird gestartet...");

            // Lade Konfiguration
            Config::getInstance().load("config.json");

            // Initialisiere Audio Engine
            if (!AudioEngine::getInstance().initialize())
            {
                throw std::runtime_error("Fehler bei der Initialisierung der Audio Engine");
            }

            // Initialisiere VR Interface
            if (!VRInterface::getInstance().initialize())
            {
                throw std::runtime_error("Fehler bei der Initialisierung des VR Interface");
            }

            // Initialisiere Video Engine
            if (!VideoEngine::getInstance().initialize())
            {
                throw std::runtime_error("Fehler bei der Initialisierung der Video Engine");
            }

            // Erstelle Hauptfenster
            mainWindow = std::make_unique<MainWindow>(getApplicationName());
            
            // Desktop-Modus konfigurieren
            auto& vrInterface = VRInterface::getInstance();
            vrInterface.setOperationMode(VRInterface::OperationMode::Desktop);
            vrInterface.setDesktopViewport(1280, 720);
            vrInterface.setDesktopCamera(
                glm::vec3(0.0f, 1.6f, 2.0f),  // Standard-Kameraposition
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f)  // Keine Rotation
            );
            vrInterface.setDesktopScale(1.0f);
        }
        catch (const std::exception& e)
        {
            Logger::getInstance().log(LogLevel::Error, "Fehler beim Start: " + std::string(e.what()));
            quit();
        }
    }

    void shutdown() override
    {
        try
        {
            // Speichere Konfiguration
            Config::getInstance().save();

            // Beende Video Engine
            VideoEngine::getInstance().shutdown();

            // Beende VR Interface
            VRInterface::getInstance().shutdown();

            // Beende Audio Engine
            AudioEngine::getInstance().shutdown();

            // Beende Logger
            Logger::getInstance().shutdown();

            mainWindow = nullptr;
        }
        catch (const std::exception& e)
        {
            juce::Logger::writeToLog("Fehler beim Beenden: " + std::string(e.what()));
        }
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // Ignoriere weitere Instanzen
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new VRUI(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
            
            // Fenstergrößenänderung-Handler
            addComponentListener(this);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
        
        void componentMovedOrResized(bool wasMoved, bool wasResized) override
        {
            if (wasResized)
            {
                // Desktop-Viewport aktualisieren
                auto& vrInterface = VRInterface::getInstance();
                vrInterface.setDesktopViewport(getWidth(), getHeight());
            }
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(VRDAWApplication) 