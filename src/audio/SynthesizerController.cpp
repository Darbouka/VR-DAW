#include "SynthesizerController.hpp"
#include <algorithm>

namespace VR_DAW {

SynthesizerController::SynthesizerController(std::shared_ptr<AudioTrack> track, VRUI* ui)
    : track(track)
    , ui(ui)
    , view(nullptr)
{
    initializeParameterMapping();
    createUI();
}

SynthesizerController::~SynthesizerController() {
    destroyUI();
}

void SynthesizerController::initializeParameterMapping() {
    // Oszillator-Parameter
    parameterMapping["oscillator_type"] = "oscillator_type";
    parameterMapping["oscillator_mix"] = "oscillator_mix";
    parameterMapping["oscillator_detune"] = "oscillator_detune";

    // Filter-Parameter
    parameterMapping["filter_type"] = "filter_type";
    parameterMapping["filter_cutoff"] = "filter_cutoff";
    parameterMapping["filter_resonance"] = "filter_resonance";

    // Envelope-Parameter
    parameterMapping["envelope_attack"] = "envelope_attack";
    parameterMapping["envelope_decay"] = "envelope_decay";
    parameterMapping["envelope_sustain"] = "envelope_sustain";
    parameterMapping["envelope_release"] = "envelope_release";

    // LFO-Parameter
    parameterMapping["lfo_rate"] = "lfo_rate";
    parameterMapping["lfo_depth"] = "lfo_depth";
    parameterMapping["lfo_waveform"] = "lfo_waveform";
}

void SynthesizerController::createUI() {
    if (!track || !ui) return;

    view = ui->createSynthesizerView(track->getTrackId(), track->getName());
    createOscillatorUI();
    createFilterUI();
    createEnvelopeUI();
    createLFOUI();
}

void SynthesizerController::destroyUI() {
    if (view && ui) {
        ui->deleteSynthesizerView(view);
        view = nullptr;
    }
}

void SynthesizerController::updateUI() {
    if (!track || !view) return;

    std::map<std::string, float> parameters;
    
    // Synthesizer-Parameter sammeln
    for (const auto& [uiParam, synthParam] : parameterMapping) {
        parameters[uiParam] = track->getSynthesizerParameter(synthParam);
    }

    // UI aktualisieren
    ui->updateSynthesizerView(view, parameters);
}

void SynthesizerController::handleParameterChange(const std::string& paramId, float value) {
    if (!track) return;

    auto it = parameterMapping.find(paramId);
    if (it != parameterMapping.end()) {
        track->setSynthesizerParameter(it->second, value);
    }
}

void SynthesizerController::createOscillatorUI() {
    if (!view || !ui) return;

    // Oszillator-UI-Elemente werden bereits in VRUI::createSynthesizerView erstellt
    // Hier können zusätzliche Oszillator-spezifische UI-Elemente hinzugefügt werden
}

void SynthesizerController::createFilterUI() {
    if (!view || !ui) return;

    // Filter-UI-Elemente werden bereits in VRUI::createSynthesizerView erstellt
    // Hier können zusätzliche Filter-spezifische UI-Elemente hinzugefügt werden
}

void SynthesizerController::createEnvelopeUI() {
    if (!view || !ui) return;

    // Envelope-UI-Elemente werden bereits in VRUI::createSynthesizerView erstellt
    // Hier können zusätzliche Envelope-spezifische UI-Elemente hinzugefügt werden
}

void SynthesizerController::createLFOUI() {
    if (!view || !ui) return;

    // LFO-UI-Elemente werden bereits in VRUI::createSynthesizerView erstellt
    // Hier können zusätzliche LFO-spezifische UI-Elemente hinzugefügt werden
}

void SynthesizerController::updateParameter(const std::string& paramId, float value) {
    if (!track) return;

    auto it = parameterMapping.find(paramId);
    if (it != parameterMapping.end()) {
        track->setSynthesizerParameter(it->second, value);
    }
}

} // namespace VR_DAW 