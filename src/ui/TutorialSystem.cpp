#include "TutorialSystem.hpp"
#include <juce_gui_extra/juce_gui_extra.h>

namespace VR_DAW {

TutorialSystem::TutorialSystem() {
    // Initialisiere Tutorial-Daten
    initializeTutorials();
    
    // Erstelle UI-Elemente
    createUIElements();
}

void TutorialSystem::initializeTutorials() {
    // Grundlegende Tutorials
    tutorials.push_back({
        "Grundlagen",
        "Willkommen in der VR-DAW! Lernen Sie die Grundlagen der virtuellen Audio-Workstation kennen.",
        {
            {"Navigation", "Bewegen Sie sich mit den Controllern durch den Raum."},
            {"Fenster", "Öffnen und schließen Sie Fenster mit den Controllern."},
            {"Steuerung", "Interagieren Sie mit den Steuerelementen durch Greifen und Bewegen."}
        }
    });
    
    // Audio-Tutorials
    tutorials.push_back({
        "Audio-Grundlagen",
        "Lernen Sie die Grundlagen der Audiobearbeitung in der VR-DAW.",
        {
            {"Spuren", "Erstellen und verwalten Sie Audiospuren."},
            {"Plugins", "Fügen Sie Effekte und Instrumente hinzu."},
            {"Mischen", "Mischen Sie Ihre Spuren mit dem virtuellen Mixer."}
        }
    });
    
    // Effekt-Tutorials
    tutorials.push_back({
        "Effekte",
        "Entdecken Sie die verschiedenen Effekte und ihre Anwendung.",
        {
            {"Dynamik", "Verwenden Sie Kompressoren und Gates."},
            {"EQ", "Bearbeiten Sie den Frequenzgang mit dem EQ."},
            {"Reverb", "Fügen Sie Raumklang mit dem Reverb hinzu."}
        }
    });
    
    // Fortgeschrittene Tutorials
    tutorials.push_back({
        "Fortgeschrittene Techniken",
        "Lernen Sie fortgeschrittene Techniken für professionelle Ergebnisse.",
        {
            {"Automation", "Automatisieren Sie Parameter über die Zeit."},
            {"Sidechain", "Verwenden Sie Sidechain-Kompression für dynamische Effekte."},
            {"Mastering", "Optimieren Sie Ihren Mix für die Veröffentlichung."}
        }
    });
}

void TutorialSystem::createUIElements() {
    // Erstelle Tutorial-Fenster
    tutorialWindow = std::make_unique<TutorialWindow>();
    
    // Erstelle Tutorial-Steuerelemente
    createControls();
    
    // Erstelle Tutorial-Overlays
    createOverlays();
}

void TutorialSystem::startTutorial(const juce::String& tutorialId) {
    // Finde Tutorial
    auto it = std::find_if(tutorials.begin(), tutorials.end(),
        [&tutorialId](const auto& tutorial) {
            return tutorial.id == tutorialId;
        });
    
    if (it != tutorials.end()) {
        currentTutorial = &(*it);
        currentStep = 0;
        
        // Zeige Tutorial-Fenster
        tutorialWindow->setVisible(true);
        
        // Starte ersten Schritt
        showStep(currentStep);
    }
}

void TutorialSystem::showStep(int stepIndex) {
    if (currentTutorial && stepIndex < currentTutorial->steps.size()) {
        // Zeige Schritt-Overlay
        auto& step = currentTutorial->steps[stepIndex];
        showOverlay(step);
        
        // Aktualisiere Tutorial-Fenster
        tutorialWindow->updateContent(currentTutorial->title,
            currentTutorial->description,
            step.title,
            step.description);
    }
}

void TutorialSystem::nextStep() {
    if (currentTutorial && currentStep < currentTutorial->steps.size() - 1) {
        currentStep++;
        showStep(currentStep);
    } else {
        finishTutorial();
    }
}

void TutorialSystem::previousStep() {
    if (currentTutorial && currentStep > 0) {
        currentStep--;
        showStep(currentStep);
    }
}

void TutorialSystem::finishTutorial() {
    // Verstecke Tutorial-Fenster
    tutorialWindow->setVisible(false);
    
    // Entferne Overlays
    removeOverlays();
    
    // Setze Tutorial-Status zurück
    currentTutorial = nullptr;
    currentStep = 0;
}

void TutorialSystem::showOverlay(const TutorialStep& step) {
    // Erstelle Overlay für aktuellen Schritt
    auto overlay = std::make_unique<TutorialOverlay>();
    overlay->setContent(step.title, step.description);
    
    // Positioniere Overlay
    positionOverlay(overlay.get(), step);
    
    // Füge Overlay hinzu
    overlays.push_back(std::move(overlay));
}

void TutorialSystem::removeOverlays() {
    overlays.clear();
}

void TutorialSystem::positionOverlay(TutorialOverlay* overlay, const TutorialStep& step) {
    // Positioniere Overlay basierend auf Schritt-Typ
    if (step.type == "Navigation") {
        overlay->setPosition(glm::vec3(0.0f, 1.5f, -2.0f));
    } else if (step.type == "Fenster") {
        overlay->setPosition(glm::vec3(1.0f, 1.5f, -2.0f));
    } else if (step.type == "Steuerung") {
        overlay->setPosition(glm::vec3(-1.0f, 1.5f, -2.0f));
    }
}

class TutorialWindow : public juce::DocumentWindow {
public:
    TutorialWindow() : juce::DocumentWindow("Tutorial", juce::Colours::black, 0) {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setBounds(100, 100, 400, 300);
        
        // Erstelle Inhalts-Komponente
        contentComponent = std::make_unique<TutorialContent>();
        setContentOwned(contentComponent.get(), true);
    }
    
    void updateContent(const juce::String& title,
                      const juce::String& description,
                      const juce::String& stepTitle,
                      const juce::String& stepDescription) {
        contentComponent->updateContent(title, description, stepTitle, stepDescription);
    }
    
private:
    std::unique_ptr<TutorialContent> contentComponent;
};

class TutorialContent : public juce::Component {
public:
    TutorialContent() {
        // Erstelle UI-Elemente
        titleLabel.setFont(juce::Font(20.0f));
        titleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(titleLabel);
        
        descriptionLabel.setFont(juce::Font(16.0f));
        descriptionLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(descriptionLabel);
        
        stepTitleLabel.setFont(juce::Font(18.0f));
        stepTitleLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(stepTitleLabel);
        
        stepDescriptionLabel.setFont(juce::Font(14.0f));
        stepDescriptionLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(stepDescriptionLabel);
        
        nextButton.setButtonText("Weiter");
        nextButton.onClick = [this]() { if (onNext) onNext(); };
        addAndMakeVisible(nextButton);
        
        previousButton.setButtonText("Zurück");
        previousButton.onClick = [this]() { if (onPrevious) onPrevious(); };
        addAndMakeVisible(previousButton);
    }
    
    void updateContent(const juce::String& title,
                      const juce::String& description,
                      const juce::String& stepTitle,
                      const juce::String& stepDescription) {
        titleLabel.setText(title, juce::dontSendNotification);
        descriptionLabel.setText(description, juce::dontSendNotification);
        stepTitleLabel.setText(stepTitle, juce::dontSendNotification);
        stepDescriptionLabel.setText(stepDescription, juce::dontSendNotification);
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        titleLabel.setBounds(bounds.removeFromTop(40));
        descriptionLabel.setBounds(bounds.removeFromTop(60));
        stepTitleLabel.setBounds(bounds.removeFromTop(40));
        stepDescriptionLabel.setBounds(bounds.removeFromTop(100));
        
        auto buttonBounds = bounds.removeFromBottom(40);
        previousButton.setBounds(buttonBounds.removeFromLeft(100));
        nextButton.setBounds(buttonBounds.removeFromRight(100));
    }
    
    std::function<void()> onNext;
    std::function<void()> onPrevious;
    
private:
    juce::Label titleLabel;
    juce::Label descriptionLabel;
    juce::Label stepTitleLabel;
    juce::Label stepDescriptionLabel;
    juce::TextButton nextButton;
    juce::TextButton previousButton;
};

class TutorialOverlay {
public:
    void setContent(const juce::String& title, const juce::String& description) {
        this->title = title;
        this->description = description;
    }
    
    void setPosition(const glm::vec3& position) {
        this->position = position;
    }
    
private:
    juce::String title;
    juce::String description;
    glm::vec3 position;
};

} // namespace VR_DAW 