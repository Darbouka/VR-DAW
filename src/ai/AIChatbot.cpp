#include "AIChatbot.hpp"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace VR_DAW {

AIChatbot& AIChatbot::getInstance() {
    static AIChatbot instance;
    return instance;
}

void AIChatbot::initialize() {
    model = std::make_unique<NeuralNetwork>();
    textProcessor = std::make_unique<TextProcessor>();
    voiceSynthesizer = std::make_unique<VoiceSynthesizer>();
    voiceEditor = std::make_unique<VoiceEditor>();
    
    voiceEditor->initialize();
    initializeDSP();
}

void AIChatbot::shutdown() {
    model.reset();
    textProcessor.reset();
    voiceSynthesizer.reset();
    voiceEditor->shutdown();
    voiceEditor.reset();
}

std::string AIChatbot::processInput(const std::string& userInput) {
    if (!model || !textProcessor) return "Fehler: KI-System nicht initialisiert";
    
    // Text verarbeiten
    auto processedInput = textProcessor->process(userInput);
    
    // KI-Modell für Antwort generieren
    auto response = model->generateResponse(processedInput);
    
    // Antwort in Sprache umwandeln
    processTextToSpeech(response);
    
    return response;
}

void AIChatbot::setVoiceStyle(const std::string& style) {
    parameters.voiceStyle = style;
    if (voiceEditor) {
        voiceEditor->setVocoderMode(style);
    }
}

void AIChatbot::setLanguage(const std::string& language) {
    parameters.language = language;
    if (textProcessor) {
        textProcessor->setLanguage(language);
    }
}

void AIChatbot::setEmotion(const std::string& emotion) {
    parameters.emotion = emotion;
    if (voiceEditor) {
        // Emotionale Parameter anpassen
        if (emotion == "Freude") {
            voiceEditor->setPitchShiftAmount(2.0f);
            voiceEditor->setVocoderPitch(1.2f);
        } else if (emotion == "Trauer") {
            voiceEditor->setPitchShiftAmount(-2.0f);
            voiceEditor->setVocoderPitch(0.8f);
        } else if (emotion == "Wut") {
            voiceEditor->setVocoderPitch(1.5f);
            voiceEditor->setFormantShiftAmount(1.2f);
        } else {
            voiceEditor->setPitchShiftAmount(0.0f);
            voiceEditor->setVocoderPitch(1.0f);
            voiceEditor->setFormantShiftAmount(1.0f);
        }
    }
}

void AIChatbot::processAudioOutput(juce::AudioBuffer<float>& buffer) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    if (!audioQueue.empty()) {
        auto& audioData = audioQueue.front();
        buffer.makeCopyOf(audioData);
        audioQueue.pop();
    } else {
        buffer.clear();
    }
}

void AIChatbot::setVolume(float volume) {
    parameters.volume = volume;
}

void AIChatbot::setPitch(float pitch) {
    parameters.pitch = pitch;
    if (voiceEditor) {
        voiceEditor->setPitchShiftAmount(pitch - 1.0f);
    }
}

void AIChatbot::setSpeechRate(float rate) {
    parameters.speechRate = rate;
    if (voiceSynthesizer) {
        voiceSynthesizer->setSpeechRate(rate);
    }
}

void AIChatbot::loadModel(const std::string& modelPath) {
    if (model) {
        model->load(modelPath);
    }
}

void AIChatbot::saveModel(const std::string& modelPath) {
    if (model) {
        model->save(modelPath);
    }
}

void AIChatbot::trainModel(const std::string& trainingDataPath) {
    if (model && textProcessor) {
        // Trainingsdaten laden und verarbeiten
        // Hier würde die Implementierung des Trainings folgen
    }
}

void AIChatbot::initializeDSP() {
    if (voiceEditor) {
        voiceEditor->setVocoderEnabled(true);
        voiceEditor->setReverbEnabled(true);
        voiceEditor->setReverbAmount(0.2f);
        voiceEditor->setReverbSize(0.5f);
        voiceEditor->setReverbDamping(0.5f);
    }
}

void AIChatbot::processTextToSpeech(const std::string& text) {
    if (!voiceSynthesizer) return;
    
    // Text in Sprache umwandeln
    auto audioData = voiceSynthesizer->synthesize(text);
    
    // Voice-Effekte anwenden
    applyVoiceEffects(audioData);
    
    // In Queue speichern
    std::lock_guard<std::mutex> lock(queueMutex);
    audioQueue.push(std::move(audioData));
    queueCondition.notify_one();
}

void AIChatbot::synthesizeVoice(const std::string& text) {
    if (!voiceSynthesizer) return;
    
    // Sprachsynthese mit aktuellen Parametern
    voiceSynthesizer->setVoiceStyle(parameters.voiceStyle);
    voiceSynthesizer->setLanguage(parameters.language);
    voiceSynthesizer->setEmotion(parameters.emotion);
    voiceSynthesizer->setVolume(parameters.volume);
    voiceSynthesizer->setPitch(parameters.pitch);
    voiceSynthesizer->setSpeechRate(parameters.speechRate);
    
    auto audioData = voiceSynthesizer->synthesize(text);
    applyVoiceEffects(audioData);
    
    std::lock_guard<std::mutex> lock(queueMutex);
    audioQueue.push(std::move(audioData));
    queueCondition.notify_one();
}

void AIChatbot::applyVoiceEffects(juce::AudioBuffer<float>& buffer) {
    if (!voiceEditor) return;
    
    // Voice-Effekte anwenden
    voiceEditor->processBlock(buffer);
    
    // Lautstärke anpassen
    buffer.applyGain(parameters.volume);
}

} // namespace VR_DAW 