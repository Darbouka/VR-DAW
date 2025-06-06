#pragma once

#include "AudioTrack.hpp"
#include "VRUI.hpp"
#include <memory>
#include <map>
#include <string>

namespace VR_DAW {

class SynthesizerController {
public:
    SynthesizerController(std::shared_ptr<AudioTrack> track, VRUI* ui);
    ~SynthesizerController();

    void updateUI();
    void handleParameterChange(const std::string& paramId, float value);
    void createUI();
    void destroyUI();

private:
    std::shared_ptr<AudioTrack> track;
    VRUI* ui;
    VRUI::SynthesizerView* view;
    std::map<std::string, std::string> parameterMapping;

    void initializeParameterMapping();
    void updateParameter(const std::string& paramId, float value);
    void createOscillatorUI();
    void createFilterUI();
    void createEnvelopeUI();
    void createLFOUI();
};

} // namespace VR_DAW 