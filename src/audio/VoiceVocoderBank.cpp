#include "VoiceVocoderBank.hpp"
#include <juce_audio_formats/juce_audio_formats.h>

namespace VR_DAW {

VoiceVocoderBank::VoiceVocoderBank() : currentMode(VocoderMode::Classic) {
    initialize();
}

VoiceVocoderBank::~VoiceVocoderBank() {
    shutdown();
}

void VoiceVocoderBank::initialize() {
    initializeDSP();
}

void VoiceVocoderBank::shutdown() {
    dsp.fft.reset();
    dsp.vocoder.reset();
    dsp.harmonizer.reset();
    dsp.formantShifter.reset();
    dsp.granularProcessor.reset();
    dsp.spectralProcessor.reset();
    dsp.phaseVocoder.reset();
    dsp.morphingProcessor.reset();
    dsp.neuralProcessor.reset();
}

void VoiceVocoderBank::initializeDSP() {
    // FFT für Spektralanalyse
    dsp.fft = std::make_unique<juce::dsp::FFT>(10); // 1024 Punkte
    
    // Vocoder-Komponenten initialisieren
    dsp.vocoder = std::make_unique<juce::dsp::Vocoder>();
    dsp.vocoder->setNumBands(parameters.classic.numBands);
    dsp.vocoder->setBandwidth(parameters.classic.bandwidth);
    dsp.vocoder->setAttackTime(parameters.classic.attack);
    dsp.vocoder->setReleaseTime(parameters.classic.release);
    
    dsp.harmonizer = std::make_unique<juce::dsp::Harmonizer>();
    dsp.formantShifter = std::make_unique<juce::dsp::FormantShifter>();
    dsp.granularProcessor = std::make_unique<juce::dsp::GranularProcessor>();
    dsp.spectralProcessor = std::make_unique<juce::dsp::SpectralProcessor>();
    dsp.phaseVocoder = std::make_unique<juce::dsp::PhaseVocoder>();
    dsp.morphingProcessor = std::make_unique<juce::dsp::MorphingProcessor>();
    dsp.neuralProcessor = std::make_unique<juce::dsp::NeuralProcessor>();
}

void VoiceVocoderBank::setMode(VocoderMode mode) {
    currentMode = mode;
}

VocoderMode VoiceVocoderBank::getCurrentMode() const {
    return currentMode;
}

void VoiceVocoderBank::addCustomMode(const std::string& name, const std::vector<float>& parameters) {
    customModes[name] = parameters;
}

void VoiceVocoderBank::removeCustomMode(const std::string& name) {
    customModes.erase(name);
}

void VoiceVocoderBank::processBlock(juce::AudioBuffer<float>& buffer) {
    switch (currentMode) {
        case VocoderMode::Classic:
            processClassicVocoder(buffer);
            break;
        case VocoderMode::Robot:
            processRobotVocoder(buffer);
            break;
        case VocoderMode::Choir:
            processChoirVocoder(buffer);
            break;
        case VocoderMode::Harmony:
            processHarmonyVocoder(buffer);
            break;
        case VocoderMode::Formant:
            processFormantVocoder(buffer);
            break;
        case VocoderMode::Granular:
            processGranularVocoder(buffer);
            break;
        case VocoderMode::Spectral:
            processSpectralVocoder(buffer);
            break;
        case VocoderMode::PhaseVocoder:
            processPhaseVocoder(buffer);
            break;
        case VocoderMode::Morphing:
            processMorphingVocoder(buffer);
            break;
        case VocoderMode::Neural:
            processNeuralVocoder(buffer);
            break;
        case VocoderMode::Koto:
            processKotoVocoder(buffer);
            break;
        case VocoderMode::Tuvan:
            processTuvanVocoder(buffer);
            break;
        case VocoderMode::Mongolian:
            processMongolianVocoder(buffer);
            break;
        case VocoderMode::Gregorian:
            processGregorianVocoder(buffer);
            break;
        case VocoderMode::Opera:
            processOperaVocoder(buffer);
            break;
        case VocoderMode::Bollywood:
            processBollywoodVocoder(buffer);
            break;
        case VocoderMode::Flamenco:
            processFlamencoVocoder(buffer);
            break;
        case VocoderMode::Blues:
            processBluesVocoder(buffer);
            break;
        case VocoderMode::Reggae:
            processReggaeVocoder(buffer);
            break;
        case VocoderMode::Gospel:
            processGospelVocoder(buffer);
            break;
        case VocoderMode::Jazz:
            processJazzVocoder(buffer);
            break;
        case VocoderMode::Raga:
            processRagaVocoder(buffer);
            break;
        case VocoderMode::Sufi:
            processSufiVocoder(buffer);
            break;
        case VocoderMode::Celtic:
            processCelticVocoder(buffer);
            break;
        case VocoderMode::African:
            processAfricanVocoder(buffer);
            break;
        case VocoderMode::Arabic:
            processArabicVocoder(buffer);
            break;
        case VocoderMode::Chinese:
            processChineseVocoder(buffer);
            break;
        case VocoderMode::Korean:
            processKoreanVocoder(buffer);
            break;
        case VocoderMode::Vietnamese:
            processVietnameseVocoder(buffer);
            break;
        case VocoderMode::Thai:
            processThaiVocoder(buffer);
            break;
        case VocoderMode::Psytrance:
            processPsytranceVocoder(buffer);
            break;
        case VocoderMode::GoaTrance:
            processGoaTranceVocoder(buffer);
            break;
        case VocoderMode::GoaGenre:
            processGoaGenreVocoder(buffer);
            break;
        case VocoderMode::GenreVocoder:
            processGenreVocoder(buffer);
            break;
    }
}

void VoiceVocoderBank::processFile(const std::string& inputPath, const std::string& outputPath) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(juce::File(inputPath)));
    if (!reader) return;
    
    juce::AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);
    
    processBlock(buffer);
    
    std::unique_ptr<juce::AudioFormatWriter> writer(formatManager.findFormatForFileExtension(juce::File(outputPath).getFileExtension())
        ->createWriterFor(new juce::FileOutputStream(juce::File(outputPath)), reader->sampleRate, reader->numChannels, 16, {}, 0));
    
    if (writer) {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
}

void VoiceVocoderBank::processClassicVocoder(juce::AudioBuffer<float>& buffer) {
    // Carrier-Signal erstellen (Synthesizer oder andere Quelle)
    juce::AudioBuffer<float> carrier(buffer.getNumChannels(), buffer.getNumSamples());
    // TODO: Carrier-Signal generieren
    
    // Vocoder anwenden
    dsp.vocoder->process(buffer, carrier);
    
    // Dry/Wet-Mix
    buffer.applyGain(1.0f - parameters.dryWet);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        buffer.addFrom(channel, 0, carrier, channel, 0, buffer.getNumSamples(), parameters.dryWet);
    }
}

void VoiceVocoderBank::processRobotVocoder(juce::AudioBuffer<float>& buffer) {
    // Pitch-Shift
    dsp.phaseVocoder->setPitchShift(parameters.robot.pitchShift);
    dsp.phaseVocoder->process(buffer);
    
    // Formant-Shift
    dsp.formantShifter->setFormantShift(parameters.robot.formantShift);
    dsp.formantShifter->process(buffer);
    
    // Metallischer Klang
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Resonanzfilter für metallischen Klang
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(44100, 2000, 0.5f);
    filter.process(context);
    
    // Dry/Wet-Mix
    buffer.applyGain(1.0f - parameters.dryWet);
    buffer.applyGain(parameters.dryWet * parameters.robot.metallicAmount);
}

void VoiceVocoderBank::processChoirVocoder(juce::AudioBuffer<float>& buffer) {
    juce::AudioBuffer<float> processed(buffer.getNumChannels(), buffer.getNumSamples());
    processed.makeCopyOf(buffer);
    
    // Mehrere Stimmen mit leichtem Pitch-Offset
    for (int i = 0; i < parameters.choir.numVoices; ++i) {
        float detune = parameters.choir.detune * (i - (parameters.choir.numVoices - 1) / 2.0f);
        
        dsp.phaseVocoder->setPitchShift(detune);
        dsp.phaseVocoder->process(processed);
        
        // Stereo-Spread
        float pan = parameters.choir.spread * (i / (parameters.choir.numVoices - 1.0f) * 2.0f - 1.0f);
        processed.applyGain(0, 0, processed.getNumSamples(), 1.0f - pan);
        processed.applyGain(1, 0, processed.getNumSamples(), 1.0f + pan);
        
        buffer.addFrom(0, 0, processed, 0, 0, processed.getNumSamples(), 1.0f / parameters.choir.numVoices);
        buffer.addFrom(1, 0, processed, 1, 0, processed.getNumSamples(), 1.0f / parameters.choir.numVoices);
    }
}

void VoiceVocoderBank::processHarmonyVocoder(juce::AudioBuffer<float>& buffer) {
    juce::AudioBuffer<float> processed(buffer.getNumChannels(), buffer.getNumSamples());
    
    // Harmonien anwenden
    for (float harmony : parameters.harmony.harmonies) {
        processed.makeCopyOf(buffer);
        dsp.harmonizer->setHarmony(harmony);
        dsp.harmonizer->process(processed);
        
        buffer.addFrom(0, 0, processed, 0, 0, processed.getNumSamples(), parameters.harmony.blend);
        buffer.addFrom(1, 0, processed, 1, 0, processed.getNumSamples(), parameters.harmony.blend);
    }
}

void VoiceVocoderBank::processFormantVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.formantShifter->setFormantShift(parameters.formant.formantShift);
    dsp.formantShifter->setFormantScale(parameters.formant.formantScale);
    dsp.formantShifter->setFormantPreserve(parameters.formant.formantPreserve);
    dsp.formantShifter->process(buffer);
}

void VoiceVocoderBank::processGranularVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.granularProcessor->setGrainSize(parameters.granular.grainSize);
    dsp.granularProcessor->setDensity(parameters.granular.density);
    dsp.granularProcessor->setPitch(parameters.granular.pitch);
    dsp.granularProcessor->process(buffer);
}

void VoiceVocoderBank::processSpectralVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.spectralProcessor->setSpectralShift(parameters.spectral.spectralShift);
    dsp.spectralProcessor->setSpectralScale(parameters.spectral.spectralScale);
    dsp.spectralProcessor->setSpectralPreserve(parameters.spectral.spectralPreserve);
    dsp.spectralProcessor->process(buffer);
}

void VoiceVocoderBank::processPhaseVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.phaseVocoder->setTimeStretch(parameters.phase.timeStretch);
    dsp.phaseVocoder->setPitchShift(parameters.phase.pitchShift);
    dsp.phaseVocoder->setPhasePreserve(parameters.phase.phasePreserve);
    dsp.phaseVocoder->process(buffer);
}

void VoiceVocoderBank::processMorphingVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.morphingProcessor->setMorphAmount(parameters.morphing.morphAmount);
    dsp.morphingProcessor->setTargetVoice(parameters.morphing.targetVoice);
    dsp.morphingProcessor->setFormantPreserve(parameters.morphing.formantPreserve);
    dsp.morphingProcessor->process(buffer);
}

void VoiceVocoderBank::processNeuralVocoder(juce::AudioBuffer<float>& buffer) {
    dsp.neuralProcessor->setStyleStrength(parameters.neural.styleStrength);
    dsp.neuralProcessor->setStyleModel(parameters.neural.styleModel);
    dsp.neuralProcessor->setCreativity(parameters.neural.creativity);
    dsp.neuralProcessor->process(buffer);
}

void VoiceVocoderBank::processKotoVocoder(juce::AudioBuffer<float>& buffer) {
    // Koto-ähnliche Saitenresonanz
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Resonanzfilter für Koto-Klang
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(44100, 2000, 0.5f);
    filter.process(context);
    
    // Pluck-Effekt
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float pluck = std::exp(-sample * parameters.koto.pluckIntensity);
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * pluck);
        }
    }
    
    // Koto-Blend
    buffer.applyGain(parameters.koto.kotoBlend);
}

void VoiceVocoderBank::processTuvanVocoder(juce::AudioBuffer<float>& buffer) {
    // Kehlkopfgesang-Simulation
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Harmonische Verstärkung
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 10.0f);
    filter.process(context);
    
    // Drone-Effekt
    juce::AudioBuffer<float> drone(buffer.getNumChannels(), buffer.getNumSamples());
    drone.makeCopyOf(buffer);
    drone.applyGain(parameters.tuvan.droneIntensity);
    
    buffer.addFrom(0, 0, drone, 0, 0, buffer.getNumSamples(), 1.0f);
}

void VoiceVocoderBank::processMongolianVocoder(juce::AudioBuffer<float>& buffer) {
    // Bruststimme
    juce::dsp::AudioBlock<float> chestBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> chestContext(chestBlock);
    juce::dsp::IIR::Filter<float> chestFilter;
    chestFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 500);
    chestFilter.process(chestContext);
    
    // Kopfstimme
    juce::AudioBuffer<float> headVoice(buffer.getNumChannels(), buffer.getNumSamples());
    headVoice.makeCopyOf(buffer);
    juce::dsp::AudioBlock<float> headBlock(headVoice);
    juce::dsp::ProcessContextReplacing<float> headContext(headBlock);
    juce::dsp::IIR::Filter<float> headFilter;
    headFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000);
    headFilter.process(headContext);
    
    // Mix
    buffer.applyGain(parameters.mongolian.chestVoice);
    buffer.addFrom(0, 0, headVoice, 0, 0, buffer.getNumSamples(), parameters.mongolian.headVoice);
}

void VoiceVocoderBank::processGregorianVocoder(juce::AudioBuffer<float>& buffer) {
    // Reverb für Kirchenakustik
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = parameters.gregorian.reverbSize;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.8f;
    reverbParams.dryLevel = 0.2f;
    reverb.setParameters(reverbParams);
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
    
    // Chor-Effekt
    juce::AudioBuffer<float> choir(buffer.getNumChannels(), buffer.getNumSamples());
    choir.makeCopyOf(buffer);
    dsp.phaseVocoder->setPitchShift(0.1f);
    dsp.phaseVocoder->process(choir);
    
    buffer.addFrom(0, 0, choir, 0, 0, buffer.getNumSamples(), parameters.gregorian.choirSize);
}

void VoiceVocoderBank::processOperaVocoder(juce::AudioBuffer<float>& buffer) {
    // Vibrato
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float vibrato = std::sin(2.0f * M_PI * parameters.opera.vibratoRate * sample / 44100.0f);
            float delay = parameters.opera.vibratoDepth * vibrato;
            int delaySamples = static_cast<int>(delay * 44100.0f);
            if (sample + delaySamples < buffer.getNumSamples()) {
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) + 
                    buffer.getSample(channel, sample + delaySamples));
            }
        }
    }
    
    // Opernresonanz
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2500, 0.5f, 10.0f);
    filter.process(context);
}

void VoiceVocoderBank::processBollywoodVocoder(juce::AudioBuffer<float>& buffer) {
    // Melisma-Effekt
    juce::AudioBuffer<float> melisma(buffer.getNumChannels(), buffer.getNumSamples());
    melisma.makeCopyOf(buffer);
    dsp.phaseVocoder->setPitchShift(0.2f);
    dsp.phaseVocoder->process(melisma);
    
    buffer.addFrom(0, 0, melisma, 0, 0, buffer.getNumSamples(), parameters.bollywood.melisma);
    
    // Ornamentation
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, 8.0f);
    filter.process(context);
}

void VoiceVocoderBank::processFlamencoVocoder(juce::AudioBuffer<float>& buffer) {
    // Flamenco-Stil
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Gitarren-ähnliche Resonanz
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 6.0f);
    filter.process(context);
    
    // Palmas-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 10ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.flamenco.palmasSync));
            }
        }
    }
}

void VoiceVocoderBank::processBluesVocoder(juce::AudioBuffer<float>& buffer) {
    // Blue Notes
    juce::AudioBuffer<float> blueNotes(buffer.getNumChannels(), buffer.getNumSamples());
    blueNotes.makeCopyOf(buffer);
    dsp.phaseVocoder->setPitchShift(-0.25f); // Blue Note Shift
    dsp.phaseVocoder->process(blueNotes);
    
    buffer.addFrom(0, 0, blueNotes, 0, 0, buffer.getNumSamples(), parameters.blues.blueNotes);
    
    // Slide-Effekt
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float slide = std::exp(-sample * parameters.blues.slideIntensity);
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * slide);
        }
    }
}

void VoiceVocoderBank::processReggaeVocoder(juce::AudioBuffer<float>& buffer) {
    // Offbeat-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 8820 == 4410) { // Offbeat
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.reggae.offbeatSync));
            }
        }
    }
    
    // Dub-Echo
    juce::dsp::Delay<float> delay;
    delay.setDelay(0.3f); // 300ms
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    delay.process(context);
}

void VoiceVocoderBank::processGospelVocoder(juce::AudioBuffer<float>& buffer) {
    // Chor-Effekt
    juce::AudioBuffer<float> choir(buffer.getNumChannels(), buffer.getNumSamples());
    choir.makeCopyOf(buffer);
    dsp.phaseVocoder->setPitchShift(0.1f);
    dsp.phaseVocoder->process(choir);
    
    buffer.addFrom(0, 0, choir, 0, 0, buffer.getNumSamples(), parameters.gospel.choirSize);
    
    // Call & Response
    juce::AudioBuffer<float> response(buffer.getNumChannels(), buffer.getNumSamples());
    response.makeCopyOf(buffer);
    response.applyGain(parameters.gospel.callResponse);
    response.applyGain(0.5f); // Halbe Lautstärke für Response
    
    buffer.addFrom(0, buffer.getNumSamples()/2, response, 0, 0, buffer.getNumSamples()/2, 1.0f);
}

void VoiceVocoderBank::processJazzVocoder(juce::AudioBuffer<float>& buffer) {
    // Swing-Effekt
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 2205) { // Swing
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.jazz.swingAmount));
            }
        }
    }
    
    // Scat-Effekt
    juce::AudioBuffer<float> scat(buffer.getNumChannels(), buffer.getNumSamples());
    scat.makeCopyOf(buffer);
    dsp.phaseVocoder->setPitchShift(0.2f);
    dsp.phaseVocoder->process(scat);
    
    buffer.addFrom(0, 0, scat, 0, 0, buffer.getNumSamples(), parameters.jazz.scatIntensity);
}

void VoiceVocoderBank::processRagaVocoder(juce::AudioBuffer<float>& buffer) {
    // Raga-Skala
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 8.0f);
    filter.process(context);
    
    // Drone-Effekt
    juce::AudioBuffer<float> drone(buffer.getNumChannels(), buffer.getNumSamples());
    drone.makeCopyOf(buffer);
    drone.applyGain(parameters.raga.droneIntensity);
    
    buffer.addFrom(0, 0, drone, 0, 0, buffer.getNumSamples(), 1.0f);
}

void VoiceVocoderBank::processSufiVocoder(juce::AudioBuffer<float>& buffer) {
    // Trance-Effekt
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, 10.0f);
    filter.process(context);
    
    // Spiritueller Echo
    juce::dsp::Delay<float> delay;
    delay.setDelay(0.5f); // 500ms
    delay.process(context);
}

void VoiceVocoderBank::processCelticVocoder(juce::AudioBuffer<float>& buffer) {
    // Modal Voice
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2500, 0.5f, 6.0f);
    filter.process(context);
    
    // Harp-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.celtic.harpSync));
            }
        }
    }
}

void VoiceVocoderBank::processAfricanVocoder(juce::AudioBuffer<float>& buffer) {
    // Polyrhythm
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 2940 == 0) { // 3/4 Takt
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.african.polyrhythm));
            }
        }
    }
    
    // Call & Response
    juce::AudioBuffer<float> response(buffer.getNumChannels(), buffer.getNumSamples());
    response.makeCopyOf(buffer);
    response.applyGain(parameters.african.callResponse);
    
    buffer.addFrom(0, buffer.getNumSamples()/2, response, 0, 0, buffer.getNumSamples()/2, 1.0f);
}

void VoiceVocoderBank::processArabicVocoder(juce::AudioBuffer<float>& buffer) {
    // Maqam-Skala
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 8.0f);
    filter.process(context);
    
    // Oud-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.arabic.oudSync));
            }
        }
    }
}

void VoiceVocoderBank::processChineseVocoder(juce::AudioBuffer<float>& buffer) {
    // Pentatonische Skala
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2500, 0.5f, 6.0f);
    filter.process(context);
    
    // Guzheng-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.chinese.guzhengSync));
            }
        }
    }
}

void VoiceVocoderBank::processKoreanVocoder(juce::AudioBuffer<float>& buffer) {
    // Pansori-Stil
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 8.0f);
    filter.process(context);
    
    // Gayageum-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.korean.gayageumSync));
            }
        }
    }
}

void VoiceVocoderBank::processVietnameseVocoder(juce::AudioBuffer<float>& buffer) {
    // Tonale Sprache
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2500, 0.5f, 6.0f);
    filter.process(context);
    
    // Dan Tranh-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.vietnamese.danTranhSync));
            }
        }
    }
}

void VoiceVocoderBank::processThaiVocoder(juce::AudioBuffer<float>& buffer) {
    // Pentatonische Skala
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, 8.0f);
    filter.process(context);
    
    // Khim-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.thai.khimSync));
            }
        }
    }
}

void VoiceVocoderBank::processPsytranceVocoder(juce::AudioBuffer<float>& buffer) {
    // Acid-Filter mit hoher Resonanz
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Resonanzfilter für Acid-Sound
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.psytrance.resonance);
    filter.process(context);
    
    // LFO-Modulation des Filters
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float lfo = std::sin(2.0f * M_PI * parameters.psytrance.lfoRate * sample / 44100.0f);
            float modulation = parameters.psytrance.lfoDepth * lfo;
            
            // Filter-Frequenz modulieren
            float frequency = 2000.0f * (1.0f + modulation);
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, frequency, parameters.psytrance.resonance);
            filter.process(context);
        }
    }
    
    // Delay mit Feedback
    juce::dsp::Delay<float> delay;
    delay.setDelay(0.25f); // 250ms
    delay.setFeedback(parameters.psytrance.delayFeedback);
    delay.process(context);
    
    // Reverb für psychedelischen Raum
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = parameters.psytrance.reverbSize;
    reverbParams.damping = 0.3f;
    reverbParams.wetLevel = 0.6f;
    reverbParams.dryLevel = 0.4f;
    reverb.setParameters(reverbParams);
    reverb.process(context);
    
    // Distortion für aggressiven Sound
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float sample = buffer.getSample(channel, sample);
            float distorted = std::tanh(sample * (1.0f + parameters.psytrance.distortion));
            buffer.setSample(channel, sample, distorted);
        }
    }
    
    // Stereo-Width
    if (buffer.getNumChannels() == 2) {
        juce::AudioBuffer<float> midSide(buffer.getNumChannels(), buffer.getNumSamples());
        midSide.makeCopyOf(buffer);
        
        // Mid/Side-Processing
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float mid = (buffer.getSample(0, sample) + buffer.getSample(1, sample)) * 0.5f;
            float side = (buffer.getSample(0, sample) - buffer.getSample(1, sample)) * 0.5f;
            
            // Side-Signal verstärken
            side *= (1.0f + parameters.psytrance.stereoWidth);
            
            // Zurück zu L/R
            buffer.setSample(0, sample, mid + side);
            buffer.setSample(1, sample, mid - side);
        }
    }
    
    // Glitch-Effekte
    if (parameters.psytrance.glitchIntensity > 0.0f) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                if (std::rand() % 1000 < static_cast<int>(parameters.psytrance.glitchIntensity * 10)) {
                    // Zufällige Glitch-Effekte
                    switch (std::rand() % 4) {
                        case 0: // Sample-Repeat
                            if (sample > 100) {
                                buffer.setSample(channel, sample, buffer.getSample(channel, sample - 100));
                            }
                            break;
                        case 1: // Bit-Crush
                            buffer.setSample(channel, sample, std::round(buffer.getSample(channel, sample) * 16.0f) / 16.0f);
                            break;
                        case 2: // Sample-Reverse
                            if (sample > 0 && sample < buffer.getNumSamples() - 1) {
                                buffer.setSample(channel, sample, buffer.getSample(channel, sample + 1));
                            }
                            break;
                        case 3: // Noise-Burst
                            buffer.setSample(channel, sample, (std::rand() % 2000 - 1000) / 1000.0f);
                            break;
                    }
                }
            }
        }
    }
}

void VoiceVocoderBank::processGoaTranceVocoder(juce::AudioBuffer<float>& buffer) {
    // Acid-Filter mit hoher Resonanz
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Resonanzfilter für Acid-Sound
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.goaTrance.resonance);
    filter.process(context);
    
    // LFO-Modulation des Filters
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float lfo = std::sin(2.0f * M_PI * parameters.goaTrance.lfoRate * sample / 44100.0f);
            float modulation = parameters.goaTrance.lfoDepth * lfo;
            
            // Filter-Frequenz modulieren
            float frequency = 2000.0f * (1.0f + modulation);
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, frequency, parameters.goaTrance.resonance);
            filter.process(context);
        }
    }
    
    // Phaser-Effekt
    juce::dsp::Phaser<float> phaser;
    phaser.setRate(parameters.goaTrance.phaserRate);
    phaser.setDepth(parameters.goaTrance.phaserDepth);
    phaser.process(context);
    
    // Flanger-Effekt
    juce::dsp::Flanger<float> flanger;
    flanger.setRate(parameters.goaTrance.flangerRate);
    flanger.setDepth(parameters.goaTrance.flangerDepth);
    flanger.process(context);
    
    // Chorus-Effekt
    juce::dsp::Chorus<float> chorus;
    chorus.setRate(parameters.goaTrance.chorusRate);
    chorus.setDepth(parameters.goaTrance.chorusDepth);
    chorus.process(context);
    
    // Sitar-ähnliche Resonanz
    juce::dsp::IIR::Filter<float> sitarFilter;
    sitarFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.goaTrance.sitarResonance);
    sitarFilter.process(context);
    
    // Tabla-Sync
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (sample % 4410 == 0) { // 100ms Intervalle
                buffer.setSample(channel, sample, 
                    buffer.getSample(channel, sample) * (1.0f + parameters.goaTrance.tablaSync));
            }
        }
    }
    
    // Drone-Effekt
    juce::AudioBuffer<float> drone(buffer.getNumChannels(), buffer.getNumSamples());
    drone.makeCopyOf(buffer);
    drone.applyGain(parameters.goaTrance.droneIntensity);
    
    buffer.addFrom(0, 0, drone, 0, 0, buffer.getNumSamples(), 1.0f);
    
    // Spiritueller Echo
    juce::dsp::Delay<float> delay;
    delay.setDelay(0.5f); // 500ms
    delay.setFeedback(parameters.goaTrance.spiritualEcho);
    delay.process(context);
    
    // Stereo-Width
    if (buffer.getNumChannels() == 2) {
        juce::AudioBuffer<float> midSide(buffer.getNumChannels(), buffer.getNumSamples());
        midSide.makeCopyOf(buffer);
        
        // Mid/Side-Processing
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float mid = (buffer.getSample(0, sample) + buffer.getSample(1, sample)) * 0.5f;
            float side = (buffer.getSample(0, sample) - buffer.getSample(1, sample)) * 0.5f;
            
            // Side-Signal verstärken
            side *= (1.0f + parameters.goaTrance.stereoWidth);
            
            // Zurück zu L/R
            buffer.setSample(0, sample, mid + side);
            buffer.setSample(1, sample, mid - side);
        }
    }
    
    // Glitch-Effekte
    if (parameters.goaTrance.glitchIntensity > 0.0f) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                if (std::rand() % 1000 < static_cast<int>(parameters.goaTrance.glitchIntensity * 10)) {
                    // Zufällige Glitch-Effekte
                    switch (std::rand() % 4) {
                        case 0: // Sample-Repeat
                            if (sample > 100) {
                                buffer.setSample(channel, sample, buffer.getSample(channel, sample - 100));
                            }
                            break;
                        case 1: // Bit-Crush
                            buffer.setSample(channel, sample, std::round(buffer.getSample(channel, sample) * 16.0f) / 16.0f);
                            break;
                        case 2: // Sample-Reverse
                            if (sample > 0 && sample < buffer.getNumSamples() - 1) {
                                buffer.setSample(channel, sample, buffer.getSample(channel, sample + 1));
                            }
                            break;
                        case 3: // Noise-Burst
                            buffer.setSample(channel, sample, (std::rand() % 2000 - 1000) / 1000.0f);
                            break;
                    }
                }
            }
        }
    }
}

void VoiceVocoderBank::processGoaGenreVocoder(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Basis-Effekte für alle Goa-Subgenres
    // Acid-Filter
    juce::dsp::IIR::Filter<float> filter;
    filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.goaGenre.resonance);
    filter.process(context);
    
    // LFO-Modulation
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float lfo = std::sin(2.0f * M_PI * parameters.goaGenre.lfoRate * sample / 44100.0f);
            float modulation = parameters.goaGenre.lfoDepth * lfo;
            float frequency = 2000.0f * (1.0f + modulation);
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, frequency, parameters.goaGenre.resonance);
            filter.process(context);
        }
    }
    
    // Klassischer Goa
    {
        // Sitar-Resonanz
        juce::dsp::IIR::Filter<float> sitarFilter;
        sitarFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.goaGenre.classicGoa.sitarResonance);
        sitarFilter.process(context);
        
        // Tabla-Sync
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                if (sample % 4410 == 0) {
                    buffer.setSample(channel, sample, 
                        buffer.getSample(channel, sample) * (1.0f + parameters.goaGenre.classicGoa.tablaSync));
                }
            }
        }
        
        // Drone-Effekt
        juce::AudioBuffer<float> drone(buffer.getNumChannels(), buffer.getNumSamples());
        drone.makeCopyOf(buffer);
        drone.applyGain(parameters.goaGenre.classicGoa.droneIntensity);
        buffer.addFrom(0, 0, drone, 0, 0, buffer.getNumSamples(), 1.0f);
    }
    
    // Progressive Goa
    {
        // Progressive Filter
        juce::dsp::IIR::Filter<float> progFilter;
        progFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 3000, parameters.goaGenre.progressiveGoa.progressiveFilter);
        progFilter.process(context);
        
        // Bass-Modulation
        juce::dsp::IIR::Filter<float> bassFilter;
        bassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 200, parameters.goaGenre.progressiveGoa.bassModulation);
        bassFilter.process(context);
    }
    
    // Dark Goa
    {
        // Industrielle Elemente
        juce::dsp::IIR::Filter<float> darkFilter;
        darkFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000, parameters.goaGenre.darkGoa.darkAtmosphere);
        darkFilter.process(context);
        
        // Metallische Resonanz
        juce::dsp::IIR::Filter<float> metalFilter;
        metalFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, parameters.goaGenre.darkGoa.metallicResonance);
        metalFilter.process(context);
    }
    
    // Forest Goa
    {
        // Organische Elemente
        juce::dsp::IIR::Filter<float> organicFilter;
        organicFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(44100, 1000, 0.5f);
        organicFilter.process(context);
        
        // Schamanischer Echo
        juce::dsp::Delay<float> shamanicDelay;
        shamanicDelay.setDelay(0.3f);
        shamanicDelay.setFeedback(parameters.goaGenre.forestGoa.shamanicEcho);
        shamanicDelay.process(context);
    }
    
    // Psychedelic Goa
    {
        // Psychedelischer Filter
        juce::dsp::IIR::Filter<float> psyFilter;
        psyFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.goaGenre.psychedelicGoa.psychedelicFilter);
        psyFilter.process(context);
        
        // Trippy-Effekte
        juce::dsp::Phaser<float> phaser;
        phaser.setRate(parameters.goaGenre.psychedelicGoa.trippyEffects);
        phaser.setDepth(0.8f);
        phaser.process(context);
    }
    
    // Suomisaundi
    {
        // Experimentelle Effekte
        juce::dsp::IIR::Filter<float> expFilter;
        expFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.goaGenre.suomisaundi.experimentalElements);
        expFilter.process(context);
        
        // Kreative Verzerrung
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float sample = buffer.getSample(channel, sample);
                float distorted = std::tanh(sample * (1.0f + parameters.goaGenre.suomisaundi.creativeDistortion));
                buffer.setSample(channel, sample, distorted);
            }
        }
    }
    
    // Zenonesque
    {
        // Minimal-Elemente
        juce::dsp::IIR::Filter<float> minimalFilter;
        minimalFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 1000, parameters.goaGenre.zenonesque.minimalElements);
        minimalFilter.process(context);
        
        // Meditativer Raum
        juce::dsp::Reverb reverb;
        juce::dsp::Reverb::Parameters reverbParams;
        reverbParams.roomSize = parameters.goaGenre.zenonesque.meditativeSpace;
        reverbParams.damping = 0.5f;
        reverbParams.wetLevel = 0.7f;
        reverbParams.dryLevel = 0.3f;
        reverb.setParameters(reverbParams);
        reverb.process(context);
    }
    
    // Twilight Goa
    {
        // Mystische Elemente
        juce::dsp::IIR::Filter<float> mysticFilter;
        mysticFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.goaGenre.twilightGoa.mysticalElements);
        mysticFilter.process(context);
        
        // Kosmischer Echo
        juce::dsp::Delay<float> cosmicDelay;
        cosmicDelay.setDelay(0.4f);
        cosmicDelay.setFeedback(parameters.goaGenre.twilightGoa.cosmicEcho);
        cosmicDelay.process(context);
    }
    
    // Morning Goa
    {
        // Uplifting-Elemente
        juce::dsp::IIR::Filter<float> upliftingFilter;
        upliftingFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 500, parameters.goaGenre.morningGoa.upliftingElements);
        upliftingFilter.process(context);
        
        // Sonnenaufgang-Atmosphäre
        juce::dsp::Reverb reverb;
        juce::dsp::Reverb::Parameters reverbParams;
        reverbParams.roomSize = parameters.goaGenre.morningGoa.sunriseAtmosphere;
        reverbParams.damping = 0.3f;
        reverbParams.wetLevel = 0.6f;
        reverbParams.dryLevel = 0.4f;
        reverb.setParameters(reverbParams);
        reverb.process(context);
    }
    
    // Full On
    {
        // Energie-Level
        buffer.applyGain(1.0f + parameters.goaGenre.fullOn.energyLevel);
        
        // Euphorische Elemente
        juce::dsp::IIR::Filter<float> euphoricFilter;
        euphoricFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, parameters.goaGenre.fullOn.euphoricElements);
        euphoricFilter.process(context);
    }
    
    // Stereo-Width für alle Subgenres
    if (buffer.getNumChannels() == 2) {
        juce::AudioBuffer<float> midSide(buffer.getNumChannels(), buffer.getNumSamples());
        midSide.makeCopyOf(buffer);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float mid = (buffer.getSample(0, sample) + buffer.getSample(1, sample)) * 0.5f;
            float side = (buffer.getSample(0, sample) - buffer.getSample(1, sample)) * 0.5f;
            side *= (1.0f + parameters.goaGenre.stereoWidth);
            buffer.setSample(0, sample, mid + side);
            buffer.setSample(1, sample, mid - side);
        }
    }
}

void VoiceVocoderBank::processGenreVocoder(juce::AudioBuffer<float>& buffer) {
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Techno-Genres
    {
        // Minimal Techno
        {
            juce::dsp::IIR::Filter<float> minimalFilter;
            minimalFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 1000, parameters.genreVocoder.techno.minimalTechno.minimalElements);
            minimalFilter.process(context);
            
            // Subtile Modulation
            juce::dsp::LFO<float> lfo;
            lfo.setFrequency(parameters.genreVocoder.techno.minimalTechno.subtleModulation);
            lfo.process(context);
        }
        
        // Industrial Techno
        {
            // Industrielle Elemente
            juce::dsp::IIR::Filter<float> industrialFilter;
            industrialFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000, parameters.genreVocoder.techno.industrialTechno.industrialElements);
            industrialFilter.process(context);
            
            // Metallische Resonanz
            juce::dsp::IIR::Filter<float> metalFilter;
            metalFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, parameters.genreVocoder.techno.industrialTechno.metallicResonance);
            metalFilter.process(context);
        }
        
        // Acid Techno
        {
            // Acid-Filter
            juce::dsp::IIR::Filter<float> acidFilter;
            acidFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.genreVocoder.techno.acidTechno.resonance);
            acidFilter.process(context);
            
            // LFO-Modulation
            juce::dsp::LFO<float> lfo;
            lfo.setFrequency(parameters.genreVocoder.techno.acidTechno.lfoModulation);
            lfo.process(context);
        }
    }
    
    // House-Genres
    {
        // Deep House
        {
            // Tiefe Atmosphäre
            juce::dsp::IIR::Filter<float> deepFilter;
            deepFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 500, parameters.genreVocoder.house.deepHouse.deepAtmosphere);
            deepFilter.process(context);
            
            // Soul-Elemente
            juce::dsp::IIR::Filter<float> soulFilter;
            soulFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.house.deepHouse.soulElements);
            soulFilter.process(context);
        }
        
        // Tech House
        {
            // Techno-Elemente
            juce::dsp::IIR::Filter<float> techFilter;
            techFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000, parameters.genreVocoder.house.techHouse.technoElements);
            techFilter.process(context);
            
            // House-Groove
            juce::dsp::LFO<float> grooveLfo;
            grooveLfo.setFrequency(parameters.genreVocoder.house.techHouse.houseGroove);
            grooveLfo.process(context);
        }
    }
    
    // Drum & Bass
    {
        // Liquid DnB
        {
            // Flüssige Elemente
            juce::dsp::IIR::Filter<float> liquidFilter;
            liquidFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.genreVocoder.drumAndBass.liquidDnB.liquidElements);
            liquidFilter.process(context);
            
            // Soul-Elemente
            juce::dsp::IIR::Filter<float> soulFilter;
            soulFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.drumAndBass.liquidDnB.soulElements);
            soulFilter.process(context);
        }
        
        // Neurofunk
        {
            // Neuro-Elemente
            juce::dsp::IIR::Filter<float> neuroFilter;
            neuroFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000, parameters.genreVocoder.drumAndBass.neurofunk.neuroElements);
            neuroFilter.process(context);
            
            // Komplexe Rhythmen
            juce::dsp::LFO<float> rhythmLfo;
            rhythmLfo.setFrequency(parameters.genreVocoder.drumAndBass.neurofunk.complexRhythms);
            rhythmLfo.process(context);
        }
    }
    
    // Trance-Genres
    {
        // Progressive Trance
        {
            // Progressive Elemente
            juce::dsp::IIR::Filter<float> progFilter;
            progFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.genreVocoder.trance.progressiveTrance.progressiveElements);
            progFilter.process(context);
            
            // Melodische Progression
            juce::dsp::LFO<float> melodyLfo;
            melodyLfo.setFrequency(parameters.genreVocoder.trance.progressiveTrance.melodicProgression);
            melodyLfo.process(context);
        }
        
        // Uplifting Trance
        {
            // Uplifting Elemente
            juce::dsp::IIR::Filter<float> upliftingFilter;
            upliftingFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 500, parameters.genreVocoder.trance.upliftingTrance.upliftingElements);
            upliftingFilter.process(context);
            
            // Euphorische Elemente
            juce::dsp::IIR::Filter<float> euphoricFilter;
            euphoricFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 3000, 0.5f, parameters.genreVocoder.trance.upliftingTrance.euphoricElements);
            euphoricFilter.process(context);
        }
    }
    
    // Dubstep
    {
        // Melodic Dubstep
        {
            // Melodische Elemente
            juce::dsp::IIR::Filter<float> melodicFilter;
            melodicFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.genreVocoder.dubstep.melodicDubstep.melodicElements);
            melodicFilter.process(context);
            
            // Bass-Gewicht
            juce::dsp::IIR::Filter<float> bassFilter;
            bassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 200, parameters.genreVocoder.dubstep.melodicDubstep.bassWeight);
            bassFilter.process(context);
        }
        
        // Riddim
        {
            // Aggressive Elemente
            juce::dsp::IIR::Filter<float> aggressiveFilter;
            aggressiveFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 1000, parameters.genreVocoder.dubstep.riddim.aggressiveElements);
            aggressiveFilter.process(context);
            
            // Bass-Gewicht
            juce::dsp::IIR::Filter<float> bassFilter;
            bassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 200, parameters.genreVocoder.dubstep.riddim.bassWeight);
            bassFilter.process(context);
        }
    }
    
    // Weltweite elektronische Musik
    {
        // Afrobeat
        {
            // Afrikanische Elemente
            juce::dsp::IIR::Filter<float> africanFilter;
            africanFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(44100, 1000, 0.5f);
            africanFilter.process(context);
            
            // Tribale Rhythmen
            juce::dsp::LFO<float> tribalLfo;
            tribalLfo.setFrequency(parameters.genreVocoder.worldElectronic.afrobeat.tribalRhythms);
            tribalLfo.process(context);
        }
        
        // K-Pop
        {
            // Koreanische Elemente
            juce::dsp::IIR::Filter<float> koreanFilter;
            koreanFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.worldElectronic.kpop.koreanElements);
            koreanFilter.process(context);
            
            // Pop-Elemente
            juce::dsp::IIR::Filter<float> popFilter;
            popFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 2000, parameters.genreVocoder.worldElectronic.kpop.popElements);
            popFilter.process(context);
        }
    }
    
    // Traditionelle Musik
    {
        // Indische Klassik
        {
            // Raga-Elemente
            juce::dsp::IIR::Filter<float> ragaFilter;
            ragaFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.traditional.indianClassical.ragaElements);
            ragaFilter.process(context);
            
            // Sitar-Resonanz
            juce::dsp::IIR::Filter<float> sitarFilter;
            sitarFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.traditional.indianClassical.sitarResonance);
            sitarFilter.process(context);
        }
        
        // Arabische Musik
        {
            // Maqam-Elemente
            juce::dsp::IIR::Filter<float> maqamFilter;
            maqamFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.traditional.arabicMusic.maqamElements);
            maqamFilter.process(context);
            
            // Oud-Resonanz
            juce::dsp::IIR::Filter<float> oudFilter;
            oudFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 2000, 0.5f, parameters.genreVocoder.traditional.arabicMusic.oudResonance);
            oudFilter.process(context);
        }
    }
    
    // Stereo-Width für alle Genres
    if (buffer.getNumChannels() == 2) {
        juce::AudioBuffer<float> midSide(buffer.getNumChannels(), buffer.getNumSamples());
        midSide.makeCopyOf(buffer);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float mid = (buffer.getSample(0, sample) + buffer.getSample(1, sample)) * 0.5f;
            float side = (buffer.getSample(0, sample) - buffer.getSample(1, sample)) * 0.5f;
            side *= 1.5f; // Erhöhte Stereo-Breite
            buffer.setSample(0, sample, mid + side);
            buffer.setSample(1, sample, mid - side);
        }
    }
}

void VoiceVocoderBank::analyzeModel(const std::string& modelPath) {
    // TODO: Implementierung der Modell-Analyse
}

void VoiceVocoderBank::saveModel(const std::string& outputPath) {
    // TODO: Implementierung des Modell-Speicherns
}

void VoiceVocoderBank::loadModel(const std::string& inputPath) {
    // TODO: Implementierung des Modell-Ladens
}

void VoiceVocoderBank::savePreset(const std::string& name) {
    presets[name] = parameters;
}

void VoiceVocoderBank::loadPreset(const std::string& name) {
    auto it = presets.find(name);
    if (it != presets.end()) {
        parameters = it->second;
    }
}

void VoiceVocoderBank::deletePreset(const std::string& name) {
    presets.erase(name);
}

std::vector<std::string> VoiceVocoderBank::getAvailablePresets() const {
    std::vector<std::string> presetNames;
    for (const auto& preset : presets) {
        presetNames.push_back(preset.first);
    }
    return presetNames;
}

void VoiceVocoderBank::setParameter(const std::string& name, float value) {
    if (name == "carrier_level") parameters.carrierLevel = value;
    else if (name == "modulator_level") parameters.modulatorLevel = value;
    else if (name == "dry_wet") parameters.dryWet = value;
    // Modus-spezifische Parameter
    else if (name == "num_bands") parameters.classic.numBands = static_cast<int>(value);
    else if (name == "bandwidth") parameters.classic.bandwidth = value;
    else if (name == "attack") parameters.classic.attack = value;
    else if (name == "release") parameters.classic.release = value;
    else if (name == "pitch_shift") parameters.robot.pitchShift = value;
    else if (name == "formant_shift") parameters.robot.formantShift = value;
    else if (name == "metallic_amount") parameters.robot.metallicAmount = value;
    else if (name == "num_voices") parameters.choir.numVoices = static_cast<int>(value);
    else if (name == "detune") parameters.choir.detune = value;
    else if (name == "spread") parameters.choir.spread = value;
    else if (name == "blend") parameters.harmony.blend = value;
    else if (name == "pitch_tracking") parameters.harmony.pitchTracking = value;
    else if (name == "formant_scale") parameters.formant.formantScale = value;
    else if (name == "formant_preserve") parameters.formant.formantPreserve = value;
    else if (name == "grain_size") parameters.granular.grainSize = value;
    else if (name == "density") parameters.granular.density = value;
    else if (name == "pitch") parameters.granular.pitch = value;
    else if (name == "spectral_shift") parameters.spectral.spectralShift = value;
    else if (name == "spectral_scale") parameters.spectral.spectralScale = value;
    else if (name == "spectral_preserve") parameters.spectral.spectralPreserve = value;
    else if (name == "time_stretch") parameters.phase.timeStretch = value;
    else if (name == "phase_preserve") parameters.phase.phasePreserve = value;
    else if (name == "morph_amount") parameters.morphing.morphAmount = value;
    else if (name == "formant_preserve") parameters.morphing.formantPreserve = value;
    else if (name == "style_strength") parameters.neural.styleStrength = value;
    else if (name == "creativity") parameters.neural.creativity = value;
}

float VoiceVocoderBank::getParameter(const std::string& name) const {
    if (name == "carrier_level") return parameters.carrierLevel;
    else if (name == "modulator_level") return parameters.modulatorLevel;
    else if (name == "dry_wet") return parameters.dryWet;
    // Modus-spezifische Parameter
    else if (name == "num_bands") return static_cast<float>(parameters.classic.numBands);
    else if (name == "bandwidth") return parameters.classic.bandwidth;
    else if (name == "attack") return parameters.classic.attack;
    else if (name == "release") return parameters.classic.release;
    else if (name == "pitch_shift") return parameters.robot.pitchShift;
    else if (name == "formant_shift") return parameters.robot.formantShift;
    else if (name == "metallic_amount") return parameters.robot.metallicAmount;
    else if (name == "num_voices") return static_cast<float>(parameters.choir.numVoices);
    else if (name == "detune") return parameters.choir.detune;
    else if (name == "spread") return parameters.choir.spread;
    else if (name == "blend") return parameters.harmony.blend;
    else if (name == "pitch_tracking") return parameters.harmony.pitchTracking;
    else if (name == "formant_scale") return parameters.formant.formantScale;
    else if (name == "formant_preserve") return parameters.formant.formantPreserve;
    else if (name == "grain_size") return parameters.granular.grainSize;
    else if (name == "density") return parameters.granular.density;
    else if (name == "pitch") return parameters.granular.pitch;
    else if (name == "spectral_shift") return parameters.spectral.spectralShift;
    else if (name == "spectral_scale") return parameters.spectral.spectralScale;
    else if (name == "spectral_preserve") return parameters.spectral.spectralPreserve;
    else if (name == "time_stretch") return parameters.phase.timeStretch;
    else if (name == "phase_preserve") return parameters.phase.phasePreserve;
    else if (name == "morph_amount") return parameters.morphing.morphAmount;
    else if (name == "formant_preserve") return parameters.morphing.formantPreserve;
    else if (name == "style_strength") return parameters.neural.styleStrength;
    else if (name == "creativity") return parameters.neural.creativity;
    return 0.0f;
}

} // namespace VR_DAW 