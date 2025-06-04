#include "VoiceSynthesizer.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <chrono>
#include <thread>

namespace VR_DAW {

VoiceSynthesizer::VoiceSynthesizer() {
    initializeModels();
    initializeVoiceActorParameters();
}

VoiceSynthesizer::~VoiceSynthesizer() = default;

void VoiceSynthesizer::initializeModels() {
    voiceModel = std::make_unique<VoiceModel>();
    phonemeConverter = std::make_unique<PhonemeConverter>();
}

void VoiceSynthesizer::initializeVoiceActorParameters() {
    // Vordefinierte Synchronsprecher-Parameter mit Originalstimmen
    std::map<std::string, Parameters::VoiceActorParams> defaultParams = {
        // Marvel Charaktere mit Originalstimmen
        {"Robert Downey Jr.", {1.0f, 1.1f, 1.2f, 1.1f, 0.2f, 0.3f}},  // Iron Man Original
        {"Chris Evans", {1.1f, 1.0f, 1.0f, 1.2f, 0.1f, 0.2f}},  // Captain America Original
        {"Scarlett Johansson", {1.2f, 0.9f, 1.1f, 1.0f, 0.1f, 0.2f}},  // Black Widow Original
        {"Chris Hemsworth", {0.9f, 1.2f, 0.9f, 1.3f, 0.3f, 0.1f}},  // Thor Original
        
        // Star Wars Charaktere mit Originalstimmen
        {"James Earl Jones", {0.7f, 1.4f, 0.8f, 1.5f, 0.5f, 0.0f}},  // Darth Vader Original
        {"Mark Hamill", {1.1f, 0.9f, 1.0f, 1.0f, 0.1f, 0.2f}},  // Luke Skywalker Original
        {"Ewan McGregor", {1.0f, 1.0f, 0.9f, 1.1f, 0.2f, 0.3f}},  // Obi-Wan Original
        
        // Game of Thrones Charaktere mit Originalstimmen
        {"Peter Dinklage", {1.1f, 1.0f, 1.1f, 1.1f, 0.2f, 0.3f}},  // Tyrion Original
        {"Emilia Clarke", {1.2f, 0.9f, 1.0f, 1.0f, 0.1f, 0.2f}},  // Daenerys Original
        {"Kit Harington", {1.0f, 1.1f, 1.0f, 1.2f, 0.2f, 0.2f}},  // Jon Snow Original
        
        // Breaking Bad Charaktere mit Originalstimmen
        {"Bryan Cranston", {1.0f, 1.1f, 1.0f, 1.2f, 0.2f, 0.2f}},  // Walter White Original
        {"Aaron Paul", {1.1f, 1.0f, 1.2f, 1.1f, 0.2f, 0.3f}},  // Jesse Pinkman Original
        
        // The Witcher Charaktere mit Originalstimmen
        {"Henry Cavill", {0.9f, 1.2f, 0.9f, 1.3f, 0.3f, 0.2f}},  // Geralt Original
        {"Anya Chalotra", {1.1f, 1.0f, 1.0f, 1.1f, 0.2f, 0.2f}},  // Yennefer Original
        
        // The Last of Us Charaktere mit Originalstimmen
        {"Pedro Pascal", {0.9f, 1.2f, 1.0f, 1.3f, 0.3f, 0.2f}},  // Joel Original
        {"Bella Ramsey", {1.2f, 0.9f, 1.1f, 1.0f, 0.1f, 0.2f}}  // Ellie Original
    };
    
    // Parameter für jeden Schauspieler speichern
    for (const auto& [actor, params] : defaultParams) {
        voiceActorModels[actor] = std::make_unique<VoiceModel>();
        voiceActorModels[actor]->setParameters(params);
        
        // Sprachspezifische Daten initialisieren
        VoiceActorData data;
        data.originalLanguage = "English";  // Standardmäßig Englisch
        data.currentLanguage = VoiceLanguage::Original;
        data.languageParams.accentStrength = 0.0f;
        data.languageParams.pronunciationAccuracy = 1.0f;
        data.languageParams.languageBlend = 0.0f;
        
        voiceActorData[actor] = std::move(data);
    }
}

void VoiceSynthesizer::setVoiceActor(const std::string& actorName) {
    if (voiceActorModels.find(actorName) != voiceActorModels.end()) {
        currentVoiceActor = actorName;
        parameters.voiceActorParams = voiceActorModels[actorName]->getParameters();
    }
}

std::vector<std::string> VoiceSynthesizer::getAvailableVoiceActors() const {
    std::vector<std::string> actors;
    for (const auto& [actor, _] : voiceActorModels) {
        actors.push_back(actor);
    }
    return actors;
}

void VoiceSynthesizer::loadVoiceActorModel(const std::string& actorName) {
    if (voiceActorModels.find(actorName) != voiceActorModels.end()) {
        voiceActorModels[actorName]->load("models/voice_actors/" + actorName + ".bin");
    }
}

void VoiceSynthesizer::trainVoiceActorModel(const std::string& actorName, const std::string& trainingDataPath) {
    if (voiceActorModels.find(actorName) != voiceActorModels.end()) {
        voiceActorModels[actorName]->train(trainingDataPath);
    }
}

juce::AudioBuffer<float> VoiceSynthesizer::synthesize(const std::string& text) {
    if (!currentVoiceActor.empty() && voiceActorData.find(currentVoiceActor) != voiceActorData.end()) {
        const auto& data = voiceActorData[currentVoiceActor];
        
        // Sprachspezifische Verarbeitung
        if (data.currentLanguage == VoiceLanguage::Both) {
            // Beide Sprachen mischen
            auto germanBuffer = data.languageModels[VoiceLanguage::German]->synthesize(text);
            auto originalBuffer = data.languageModels[VoiceLanguage::Original]->synthesize(text);
            
            // Buffer mischen
            auto outputBuffer = germanBuffer;
            outputBuffer.addFrom(0, 0, originalBuffer, 0, 0, outputBuffer.getNumSamples(), 
                               data.languageParams.languageBlend);
            
            return outputBuffer;
        } else {
            // Einzelne Sprache verwenden
            return data.languageModels[data.currentLanguage]->synthesize(text);
        }
    }
    
    // Fallback auf Standard-Synthese
    return voiceModel->synthesize(text);
}

void VoiceSynthesizer::applyVoiceActorParameters(juce::AudioBuffer<float>& buffer) {
    if (currentVoiceActor.empty() || voiceActorModels.find(currentVoiceActor) == voiceActorModels.end()) {
        return;
    }
    
    auto& params = parameters.voiceActorParams;
    
    // Grundlegende Stimmanpassungen
    if (voiceModel) {
        voiceModel->setPitch(params.basePitch);
        voiceModel->setFormantShift(params.baseFormantShift);
        voiceModel->setSpeechRate(params.baseSpeechRate);
        
        // Spezifische Stimmeigenschaften
        voiceModel->setTimbre(params.voiceTimbre);
        voiceModel->setRoughness(params.voiceRoughness);
        voiceModel->setBreathiness(params.voiceBreathiness);
    }
    
    // Buffer mit aktualisierten Parametern neu generieren
    auto newBuffer = voiceModel->process(buffer);
    buffer.makeCopyOf(newBuffer);
}

void VoiceSynthesizer::setVoiceStyle(const std::string& style) {
    parameters.voiceStyle = style;
    if (voiceModel) {
        voiceModel->setVoiceStyle(style);
    }
}

void VoiceSynthesizer::setLanguage(const std::string& language) {
    parameters.language = language;
    if (phonemeConverter) {
        phonemeConverter->setLanguage(language);
    }
}

void VoiceSynthesizer::setEmotion(const std::string& emotion) {
    parameters.emotion = emotion;
}

void VoiceSynthesizer::setVolume(float volume) {
    parameters.volume = volume;
}

void VoiceSynthesizer::setPitch(float pitch) {
    parameters.pitch = pitch;
    if (voiceModel) {
        voiceModel->setPitch(pitch);
    }
}

void VoiceSynthesizer::setSpeechRate(float rate) {
    parameters.speechRate = rate;
    if (voiceModel) {
        voiceModel->setSpeechRate(rate);
    }
}

void VoiceSynthesizer::setFormantShift(float shift) {
    parameters.formantShift = shift;
    if (voiceModel) {
        voiceModel->setFormantShift(shift);
    }
}

void VoiceSynthesizer::loadModel(const std::string& modelPath) {
    if (voiceModel) {
        voiceModel->load(modelPath);
    }
    if (phonemeConverter) {
        phonemeConverter->load(modelPath);
    }
}

void VoiceSynthesizer::saveModel(const std::string& modelPath) {
    if (voiceModel) {
        voiceModel->save(modelPath);
    }
    if (phonemeConverter) {
        phonemeConverter->save(modelPath);
    }
}

void VoiceSynthesizer::trainModel(const std::string& trainingDataPath) {
    if (voiceModel && phonemeConverter) {
        // Trainingsdaten laden und verarbeiten
        // Hier würde die Implementierung des Trainings folgen
    }
}

std::vector<std::string> VoiceSynthesizer::textToPhonemes(const std::string& text) {
    if (!phonemeConverter) return std::vector<std::string>();
    return phonemeConverter->convert(text);
}

juce::AudioBuffer<float> VoiceSynthesizer::phonemesToAudio(const std::vector<std::string>& phonemes) {
    if (!voiceModel) return juce::AudioBuffer<float>();
    return voiceModel->synthesize(phonemes);
}

void VoiceSynthesizer::applyVoiceStyle(juce::AudioBuffer<float>& buffer) {
    if (!voiceModel) return;
    
    // Voice-Style Parameter anwenden
    if (parameters.voiceStyle == "Robot") {
        voiceModel->setPitch(1.5f);
        voiceModel->setFormantShift(1.2f);
    } else if (parameters.voiceStyle == "Whisper") {
        voiceModel->setPitch(0.8f);
        voiceModel->setFormantShift(0.9f);
    } else if (parameters.voiceStyle == "Choir") {
        voiceModel->setPitch(1.2f);
        voiceModel->setFormantShift(1.1f);
    } else {
        voiceModel->setPitch(1.0f);
        voiceModel->setFormantShift(1.0f);
    }
    
    // Buffer mit aktualisierten Parametern neu generieren
    auto newBuffer = voiceModel->process(buffer);
    buffer.makeCopyOf(newBuffer);
}

void VoiceSynthesizer::applyEmotion(juce::AudioBuffer<float>& buffer) {
    if (!voiceModel) return;
    
    // Emotionale Parameter anwenden
    if (parameters.emotion == "Freude") {
        voiceModel->setPitch(1.2f);
        voiceModel->setFormantShift(1.1f);
        voiceModel->setSpeechRate(1.1f);
    } else if (parameters.emotion == "Trauer") {
        voiceModel->setPitch(0.8f);
        voiceModel->setFormantShift(0.9f);
        voiceModel->setSpeechRate(0.9f);
    } else if (parameters.emotion == "Wut") {
        voiceModel->setPitch(1.5f);
        voiceModel->setFormantShift(1.3f);
        voiceModel->setSpeechRate(1.2f);
    } else {
        voiceModel->setPitch(1.0f);
        voiceModel->setFormantShift(1.0f);
        voiceModel->setSpeechRate(1.0f);
    }
    
    // Buffer mit aktualisierten Parametern neu generieren
    auto newBuffer = voiceModel->process(buffer);
    buffer.makeCopyOf(newBuffer);
}

void VoiceSynthesizer::optimizeVoiceParameters() {
    // Optimierte Parameter für Vocoder und Editor
    struct VocoderOptimization {
        float carrierMix = 0.6f;
        float modulatorMix = 0.4f;
        float formantShift = 1.2f;
        float pitchShift = 1.1f;
        int numBands = 24;  // Erhöhte Bandanzahl für bessere Auflösung
        float bandWidth = 0.8f;
        float envelopeAttack = 0.01f;
        float envelopeRelease = 0.1f;
    } vocoderOpt;

    struct EditorOptimization {
        float pitchShift = 1.0f;
        float formantShift = 1.0f;
        float timbreShift = 1.0f;
        float breathiness = 0.0f;
        float roughness = 0.0f;
        float clarity = 1.0f;
        float presence = 1.0f;
        float warmth = 1.0f;
    } editorOpt;

    // KI-Optimierungsparameter
    struct KIOptimization {
        float learningRate = 0.001f;
        float dropoutRate = 0.2f;
        int hiddenLayers = 3;
        int neuronsPerLayer = 128;
        float regularizationStrength = 0.01f;
    } kiOpt;

    // Optimierte Parameter für perfekte Tonlage und Aussprache
    std::map<std::string, Parameters::VoiceActorParams> optimizedParams = {
        // Marvel Charaktere mit optimierten Originalstimmen
        {"Robert Downey Jr.", {1.05f, 1.15f, 1.15f, 1.12f, 0.15f, 0.25f, 0.95f, 1.05f}},  // Iron Man
        {"Chris Evans", {1.08f, 1.12f, 1.05f, 1.15f, 0.12f, 0.18f, 0.98f, 1.02f}},  // Captain America
        {"Scarlett Johansson", {1.15f, 0.95f, 1.08f, 1.05f, 0.08f, 0.15f, 1.02f, 0.98f}},  // Black Widow
        {"Chris Hemsworth", {0.95f, 1.18f, 0.95f, 1.25f, 0.25f, 0.12f, 0.95f, 1.05f}},  // Thor
        
        // Star Wars Charaktere mit optimierten Originalstimmen
        {"James Earl Jones", {0.75f, 1.35f, 0.85f, 1.45f, 0.45f, 0.05f, 0.90f, 1.10f}},  // Darth Vader
        {"Mark Hamill", {1.12f, 0.92f, 1.05f, 1.02f, 0.08f, 0.18f, 1.02f, 0.98f}},  // Luke Skywalker
        {"Ewan McGregor", {1.02f, 1.05f, 0.92f, 1.12f, 0.15f, 0.25f, 0.98f, 1.02f}},  // Obi-Wan
        
        // Game of Thrones Charaktere mit optimierten Originalstimmen
        {"Peter Dinklage", {1.12f, 0.98f, 1.08f, 1.12f, 0.18f, 0.28f, 1.00f, 1.00f}},  // Tyrion
        {"Emilia Clarke", {1.18f, 0.92f, 1.02f, 1.02f, 0.08f, 0.15f, 1.02f, 0.98f}},  // Daenerys
        {"Kit Harington", {1.02f, 1.08f, 1.02f, 1.15f, 0.18f, 0.18f, 0.98f, 1.02f}}  // Jon Snow
    };

    // Parameter aktualisieren
    for (const auto& [actor, params] : optimizedParams) {
        if (voiceActorModels.find(actor) != voiceActorModels.end()) {
            voiceActorModels[actor]->setParameters(params);
        }
    }

    // Vocoder-Optimierung
    if (vocoder) {
        vocoder->setCarrierMix(vocoderOpt.carrierMix);
        vocoder->setModulatorMix(vocoderOpt.modulatorMix);
        vocoder->setFormantShift(vocoderOpt.formantShift);
        vocoder->setPitchShift(vocoderOpt.pitchShift);
        vocoder->setNumBands(vocoderOpt.numBands);
        vocoder->setBandWidth(vocoderOpt.bandWidth);
        vocoder->setEnvelopeAttack(vocoderOpt.envelopeAttack);
        vocoder->setEnvelopeRelease(vocoderOpt.envelopeRelease);
    }

    // Voice Editor Optimierung
    if (voiceEditor) {
        voiceEditor->setPitchShift(editorOpt.pitchShift);
        voiceEditor->setFormantShift(editorOpt.formantShift);
        voiceEditor->setTimbreShift(editorOpt.timbreShift);
        voiceEditor->setBreathiness(editorOpt.breathiness);
        voiceEditor->setRoughness(editorOpt.roughness);
        voiceEditor->setClarity(editorOpt.clarity);
        voiceEditor->setPresence(editorOpt.presence);
        voiceEditor->setWarmth(editorOpt.warmth);
    }

    // KI-Modell Optimierung
    if (voiceModel) {
        voiceModel->setLearningRate(kiOpt.learningRate);
        voiceModel->setDropoutRate(kiOpt.dropoutRate);
        voiceModel->setHiddenLayers(kiOpt.hiddenLayers);
        voiceModel->setNeuronsPerLayer(kiOpt.neuronsPerLayer);
        voiceModel->setRegularizationStrength(kiOpt.regularizationStrength);
    }
}

bool VoiceSynthesizer::testVoiceQuality(const std::string& actorName) {
    if (voiceActorModels.find(actorName) == voiceActorModels.end()) {
        return false;
    }
    
    // Testphrasen für verschiedene Sprachmuster
    std::vector<std::string> testPhrases = {
        "Dies ist ein Test der Sprachqualität.",
        "Wie klingt meine Stimme jetzt?",
        "Ich hoffe, die Aussprache ist klar und deutlich.",
        "Test der Tonhöhe und Modulation.",
        "Überprüfung der Sprachflüssigkeit."
    };
    
    bool allTestsPassed = true;
    
    for (const auto& phrase : testPhrases) {
        // Synthese testen
        auto audioBuffer = synthesize(phrase);
        
        // Qualitätsprüfungen
        if (!checkAudioQuality(audioBuffer)) {
            allTestsPassed = false;
            break;
        }
        
        // Kurze Pause zwischen Tests
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return allTestsPassed;
}

bool VoiceSynthesizer::checkAudioQuality(const juce::AudioBuffer<float>& buffer) {
    // Überprüfung der Audioqualität
    const float minAmplitude = 0.01f;
    const float maxAmplitude = 0.95f;
    const float minFrequency = 80.0f;  // Hz
    const float maxFrequency = 8000.0f; // Hz
    
    // Amplitude prüfen
    float maxSample = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            maxSample = std::max(maxSample, std::abs(buffer.getSample(channel, sample)));
        }
    }
    
    if (maxSample < minAmplitude || maxSample > maxAmplitude) {
        return false;
    }
    
    // Frequenzanalyse
    // Hier würde eine FFT-Analyse implementiert werden
    // Für jetzt nur eine einfache Prüfung
    
    return true;
}

void VoiceSynthesizer::runComprehensiveTest() {
    std::cout << "Starte umfassenden Test der Sprachsynthese...\n";
    
    // Liste aller verfügbaren Stimmen
    auto actors = getAvailableVoiceActors();
    
    int passedTests = 0;
    int totalTests = actors.size();
    
    for (const auto& actor : actors) {
        std::cout << "Teste Stimme: " << actor << "\n";
        
        if (testVoiceQuality(actor)) {
            std::cout << "✓ " << actor << " - Test bestanden\n";
            passedTests++;
        } else {
            std::cout << "✗ " << actor << " - Test fehlgeschlagen\n";
        }
        
        // Kurze Pause zwischen den Stimmen
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "\nTestabschluss:\n";
    std::cout << "Bestandene Tests: " << passedTests << "/" << totalTests << "\n";
    std::cout << "Erfolgsrate: " << (passedTests * 100.0f / totalTests) << "%\n";
}

void VoiceSynthesizer::initializeVocoder() {
    vocoder = std::make_unique<Vocoder>();
    vocoder->setSampleRate(realtimeParams.sampleRate);
    vocoder->setBufferSize(realtimeParams.bufferSize);
    vocoder->setNumBands(realtimeParams.vocoderParams.numBands);
}

void VoiceSynthesizer::setVocoderMode(const std::string& mode) {
    if (!vocoder) return;
    
    if (mode == "Robot") {
        realtimeParams.vocoderParams.formantShift = 1.5f;
        realtimeParams.vocoderParams.pitchShift = 1.2f;
    } else if (mode == "Choir") {
        realtimeParams.vocoderParams.formantShift = 1.2f;
        realtimeParams.vocoderParams.pitchShift = 1.1f;
    } else if (mode == "Whisper") {
        realtimeParams.vocoderParams.formantShift = 0.8f;
        realtimeParams.vocoderParams.pitchShift = 0.9f;
    }
    
    vocoder->setFormantShift(realtimeParams.vocoderParams.formantShift);
    vocoder->setPitchShift(realtimeParams.vocoderParams.pitchShift);
}

void VoiceSynthesizer::setVocoderCarrier(const std::string& voiceActor) {
    if (!vocoder || voiceActorModels.find(voiceActor) == voiceActorModels.end()) return;
    
    auto& params = voiceActorModels[voiceActor]->getParameters();
    vocoder->setCarrierParameters(params);
}

void VoiceSynthesizer::processVocoderInput(const juce::AudioBuffer<float>& inputBuffer) {
    if (!vocoder) return;
    
    // Eingangssignal optimieren
    auto optimizedBuffer = inputBuffer;
    
    // Rauschunterdrückung
    applyNoiseReduction(optimizedBuffer);
    
    // Dynamikkompression
    applyCompression(optimizedBuffer);
    
    // EQ-Anpassung
    applyEQ(optimizedBuffer);
    
    // Eingangssignal verarbeiten
    vocoder->process(optimizedBuffer);
    
    // Carrier-Signal mit optimierter Synchronsprecher-Stimme mischen
    if (!currentVoiceActor.empty() && voiceActorModels.find(currentVoiceActor) != voiceActorModels.end()) {
        auto carrierBuffer = voiceActorModels[currentVoiceActor]->synthesize(""); // Leerer Text für Carrier
        vocoder->setCarrierSignal(carrierBuffer);
    }
}

juce::AudioBuffer<float> VoiceSynthesizer::getVocoderOutput() const {
    if (!vocoder) return juce::AudioBuffer<float>();
    return vocoder->getOutput();
}

void VoiceSynthesizer::initializeVoiceEditor() {
    voiceEditor = std::make_unique<VoiceEditor>();
    voiceEditor->setSampleRate(realtimeParams.sampleRate);
    voiceEditor->setBufferSize(realtimeParams.bufferSize);
}

void VoiceSynthesizer::setVoiceEditorMode(const std::string& mode) {
    if (!voiceEditor) return;
    
    if (mode == "Pitch") {
        voiceEditor->setPitchMode();
    } else if (mode == "Formant") {
        voiceEditor->setFormantMode();
    } else if (mode == "Timbre") {
        voiceEditor->setTimbreMode();
    }
}

void VoiceSynthesizer::setVoiceEditorTarget(const std::string& voiceActor) {
    if (!voiceEditor || voiceActorModels.find(voiceActor) == voiceActorModels.end()) return;
    
    auto& params = voiceActorModels[voiceActor]->getParameters();
    voiceEditor->setTargetParameters(params);
}

void VoiceSynthesizer::processVoiceEditorInput(const juce::AudioBuffer<float>& inputBuffer) {
    if (!voiceEditor) return;
    
    // Eingangssignal optimieren
    auto optimizedBuffer = inputBuffer;
    
    // Rauschunterdrückung
    applyNoiseReduction(optimizedBuffer);
    
    // Dynamikkompression
    applyCompression(optimizedBuffer);
    
    // EQ-Anpassung
    applyEQ(optimizedBuffer);
    
    // Eingangssignal verarbeiten
    voiceEditor->process(optimizedBuffer);
    
    // Zielparameter anwenden
    if (!currentVoiceActor.empty() && voiceActorModels.find(currentVoiceActor) != voiceActorModels.end()) {
        auto& targetParams = voiceActorModels[currentVoiceActor]->getParameters();
        voiceEditor->applyTargetParameters(targetParams);
    }
}

juce::AudioBuffer<float> VoiceSynthesizer::getVoiceEditorOutput() const {
    if (!voiceEditor) return juce::AudioBuffer<float>();
    return voiceEditor->getOutput();
}

void VoiceSynthesizer::startRealtimeProcessing() {
    isRealtimeProcessing = true;
    realtimeParams.isActive = true;
    initializeRealtimeProcessing();
}

void VoiceSynthesizer::stopRealtimeProcessing() {
    isRealtimeProcessing = false;
    realtimeParams.isActive = false;
}

void VoiceSynthesizer::setRealtimeMode(const std::string& mode) {
    realtimeMode = mode;
    realtimeParams.currentMode = mode;
}

void VoiceSynthesizer::processRealtimeInput(const juce::AudioBuffer<float>& inputBuffer) {
    if (!isRealtimeProcessing) return;
    
    // Verzögerung simulieren
    std::this_thread::sleep_for(std::chrono::milliseconds(realtimeLatency));
    
    // Buffer verarbeiten
    processRealtimeBuffer(inputBuffer);
    
    // Effekte anwenden
    auto outputBuffer = inputBuffer;
    applyRealtimeEffects(outputBuffer);
    
    // Callback aufrufen
    if (realtimeCallback) {
        realtimeCallback(outputBuffer);
    }
}

void VoiceSynthesizer::initializeRealtimeProcessing() {
    // Vocoder initialisieren
    initializeVocoder();
    
    // Voice Editor initialisieren
    initializeVoiceEditor();
    
    // Standardparameter setzen
    setRealtimeLatency(10); // 10ms Latenz
    setRealtimeBufferSize(1024);
    setRealtimeSampleRate(44100.0);
}

void VoiceSynthesizer::processRealtimeBuffer(const juce::AudioBuffer<float>& inputBuffer) {
    if (realtimeMode == "Vocoder" || realtimeMode == "Both") {
        processVocoderInput(inputBuffer);
    }
    
    if (realtimeMode == "Editor" || realtimeMode == "Both") {
        processVoiceEditorInput(inputBuffer);
    }
}

void VoiceSynthesizer::applyRealtimeEffects(juce::AudioBuffer<float>& buffer) {
    // Vocoder-Effekte
    if (realtimeMode == "Vocoder" || realtimeMode == "Both") {
        auto vocoderOutput = getVocoderOutput();
        buffer.addFrom(0, 0, vocoderOutput, 0, 0, buffer.getNumSamples(), realtimeParams.vocoderParams.carrierMix);
    }
    
    // Voice Editor Effekte
    if (realtimeMode == "Editor" || realtimeMode == "Both") {
        auto editorOutput = getVoiceEditorOutput();
        buffer.addFrom(0, 0, editorOutput, 0, 0, buffer.getNumSamples(), 1.0f - realtimeParams.vocoderParams.carrierMix);
    }
}

void VoiceSynthesizer::setRealtimeLatency(int milliseconds) {
    realtimeLatency = milliseconds;
    realtimeParams.processingLatency = milliseconds / 1000.0f;
}

void VoiceSynthesizer::setRealtimeBufferSize(int samples) {
    realtimeBufferSize = samples;
    realtimeParams.bufferSize = samples;
    
    if (vocoder) vocoder->setBufferSize(samples);
    if (voiceEditor) voiceEditor->setBufferSize(samples);
}

void VoiceSynthesizer::setRealtimeSampleRate(double sampleRate) {
    realtimeSampleRate = sampleRate;
    realtimeParams.sampleRate = sampleRate;
    
    if (vocoder) vocoder->setSampleRate(sampleRate);
    if (voiceEditor) voiceEditor->setSampleRate(sampleRate);
}

void VoiceSynthesizer::setRealtimeCallback(RealtimeCallback callback) {
    realtimeCallback = std::move(callback);
}

void VoiceSynthesizer::setVoiceLanguage(const std::string& actorName, VoiceLanguage language) {
    if (voiceActorData.find(actorName) == voiceActorData.end()) {
        return;
    }
    
    auto& data = voiceActorData[actorName];
    data.currentLanguage = language;
    
    // Sprachmodelle laden
    loadLanguageModel(actorName, language);
    
    // Parameter anwenden
    applyLanguageParameters(actorName, data.languageParams);
    
    // Sprachmischung aktualisieren
    if (language == VoiceLanguage::Both) {
        blendLanguages(actorName, data.languageParams.languageBlend);
    }
}

VoiceSynthesizer::VoiceLanguage VoiceSynthesizer::getVoiceLanguage(const std::string& actorName) const {
    if (voiceActorData.find(actorName) == voiceActorData.end()) {
        return VoiceLanguage::German;
    }
    return voiceActorData.at(actorName).currentLanguage;
}

std::vector<std::string> VoiceSynthesizer::getAvailableLanguages(const std::string& actorName) const {
    std::vector<std::string> languages;
    
    if (voiceActorData.find(actorName) != voiceActorData.end()) {
        const auto& data = voiceActorData.at(actorName);
        languages.push_back("Deutsch");
        languages.push_back(data.originalLanguage);
        languages.push_back("Beide");
    }
    
    return languages;
}

void VoiceSynthesizer::setLanguageParameters(const std::string& actorName, const LanguageParameters& params) {
    if (voiceActorData.find(actorName) == voiceActorData.end()) {
        return;
    }
    
    auto& data = voiceActorData[actorName];
    data.languageParams = params;
    applyLanguageParameters(actorName, params);
}

VoiceSynthesizer::LanguageParameters VoiceSynthesizer::getLanguageParameters(const std::string& actorName) const {
    if (voiceActorData.find(actorName) == voiceActorData.end()) {
        return LanguageParameters();
    }
    return voiceActorData.at(actorName).languageParams;
}

void VoiceSynthesizer::initializeLanguageModels(const std::string& actorName) {
    VoiceActorData data;
    
    // Originalsprache basierend auf Schauspieler setzen
    if (actorName.find("Robert Downey Jr.") != std::string::npos ||
        actorName.find("Chris Evans") != std::string::npos ||
        actorName.find("Scarlett Johansson") != std::string::npos ||
        actorName.find("Chris Hemsworth") != std::string::npos ||
        actorName.find("James Earl Jones") != std::string::npos ||
        actorName.find("Mark Hamill") != std::string::npos ||
        actorName.find("Ewan McGregor") != std::string::npos ||
        actorName.find("Peter Dinklage") != std::string::npos ||
        actorName.find("Emilia Clarke") != std::string::npos ||
        actorName.find("Kit Harington") != std::string::npos ||
        actorName.find("Bryan Cranston") != std::string::npos ||
        actorName.find("Aaron Paul") != std::string::npos ||
        actorName.find("Henry Cavill") != std::string::npos ||
        actorName.find("Anya Chalotra") != std::string::npos ||
        actorName.find("Pedro Pascal") != std::string::npos ||
        actorName.find("Bella Ramsey") != std::string::npos) {
        data.originalLanguage = "English";
    }
    
    // Sprachmodelle initialisieren
    data.languageModels[VoiceLanguage::German] = std::make_unique<VoiceModel>();
    data.languageModels[VoiceLanguage::Original] = std::make_unique<VoiceModel>();
    
    // Parameter setzen
    data.languageParams.accentStrength = 0.0f;
    data.languageParams.pronunciationAccuracy = 1.0f;
    data.languageParams.languageBlend = 0.0f;
    
    voiceActorData[actorName] = std::move(data);
}

void VoiceSynthesizer::loadLanguageModel(const std::string& actorName, VoiceLanguage language) {
    auto& data = voiceActorData[actorName];
    
    // Modellpfad basierend auf Sprache und Schauspieler
    std::string modelPath = "models/voice_actors/" + actorName + "/";
    if (language == VoiceLanguage::German) {
        modelPath += "german.bin";
    } else if (language == VoiceLanguage::Original) {
        modelPath += data.originalLanguage + ".bin";
    }
    
    // Modell laden
    if (data.languageModels[language]) {
        data.languageModels[language]->load(modelPath);
    }
}

void VoiceSynthesizer::blendLanguages(const std::string& actorName, float blendFactor) {
    auto& data = voiceActorData[actorName];
    
    if (data.currentLanguage == VoiceLanguage::Both) {
        // Beide Sprachen mischen
        auto germanModel = data.languageModels[VoiceLanguage::German].get();
        auto originalModel = data.languageModels[VoiceLanguage::Original].get();
        
        if (germanModel && originalModel) {
            // Parameter mischen
            auto germanParams = germanModel->getParameters();
            auto originalParams = originalModel->getParameters();
            
            // Gewichtete Mischung
            auto blendedParams = blendFactor * germanParams + (1.0f - blendFactor) * originalParams;
            
            // Gemischte Parameter anwenden
            if (voiceModel) {
                voiceModel->setParameters(blendedParams);
            }
        }
    }
}

void VoiceSynthesizer::applyLanguageParameters(const std::string& actorName, const LanguageParameters& params) {
    auto& data = voiceActorData[actorName];
    
    // Akzent-Stärke anwenden
    if (data.currentLanguage == VoiceLanguage::Original) {
        // Originalstimme mit deutschem Akzent
        if (voiceModel) {
            voiceModel->setAccentStrength(params.accentStrength);
        }
    }
    
    // Aussprachegenauigkeit anwenden
    if (voiceModel) {
        voiceModel->setPronunciationAccuracy(params.pronunciationAccuracy);
    }
    
    // Sprachmischung anwenden
    if (data.currentLanguage == VoiceLanguage::Both) {
        blendLanguages(actorName, params.languageBlend);
    }
}

// Neue Hilfsfunktionen für die Signaloptimierung
void VoiceSynthesizer::applyNoiseReduction(juce::AudioBuffer<float>& buffer) {
    const float noiseFloor = 0.001f;
    const float reductionStrength = 0.8f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sampleValue = buffer.getSample(channel, sample);
            if (std::abs(sampleValue) < noiseFloor) {
                buffer.setSample(channel, sample, sampleValue * (1.0f - reductionStrength));
            }
        }
    }
}

void VoiceSynthesizer::applyCompression(juce::AudioBuffer<float>& buffer) {
    const float threshold = 0.5f;
    const float ratio = 0.6f;
    const float attack = 0.01f;
    const float release = 0.1f;
    
    float envelope = 0.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sampleValue = buffer.getSample(channel, sample);
            float absValue = std::abs(sampleValue);
            
            // Hüllkurve berechnen
            if (absValue > envelope) {
                envelope = absValue * attack + envelope * (1.0f - attack);
            } else {
                envelope = absValue * release + envelope * (1.0f - release);
            }
            
            // Kompression anwenden
            if (envelope > threshold) {
                float gain = threshold + (envelope - threshold) * ratio;
                buffer.setSample(channel, sample, sampleValue * (gain / envelope));
            }
        }
    }
}

void VoiceSynthesizer::applyEQ(juce::AudioBuffer<float>& buffer) {
    // Einfacher 3-Band EQ
    const float lowGain = 1.2f;    // Bass-Verstärkung
    const float midGain = 1.0f;    // Mitten-Bereich
    const float highGain = 1.1f;   // Höhen-Verstärkung
    
    // Frequenzbänder
    const float lowFreq = 200.0f;   // Hz
    const float highFreq = 2000.0f; // Hz
    
    // Hier würde eine FFT-Analyse und -Synthese implementiert werden
    // Für jetzt nur eine einfache Simulation
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sampleValue = buffer.getSample(channel, sample);
            
            // Vereinfachte Frequenzband-Simulation
            float lowComponent = sampleValue * 0.3f;
            float midComponent = sampleValue * 0.4f;
            float highComponent = sampleValue * 0.3f;
            
            // Gain anwenden
            float processedValue = lowComponent * lowGain +
                                 midComponent * midGain +
                                 highComponent * highGain;
            
            buffer.setSample(channel, sample, processedValue);
        }
    }
}

} // namespace VR_DAW 