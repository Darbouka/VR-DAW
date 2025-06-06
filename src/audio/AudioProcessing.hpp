#pragma once

#include <vector>
#include <memory>

namespace VR_DAW {

class AudioProcessing {
public:
    AudioProcessing();
    ~AudioProcessing();

    void process(float* input, float* output, unsigned long framesPerBuffer);
    
    // DSP-Funktionen
    void applyGain(float* buffer, unsigned long frames, float gain);
    void applyPan(float* buffer, unsigned long frames, float pan);
    void applyEQ(float* buffer, unsigned long frames, float lowGain, float midGain, float highGain);
    void applyCompression(float* buffer, unsigned long frames, float threshold, float ratio, float attack, float release);
    void applyReverb(float* buffer, unsigned long frames, float mix, float time, float damping);
    
    // FFT-basierte Verarbeitung
    void applyFFT(float* buffer, unsigned long frames);
    void applyIFFT(float* buffer, unsigned long frames);
    
    // Zeitbasierte Effekte
    void applyDelay(float* buffer, unsigned long frames, float delayTime, float feedback);
    void applyChorus(float* buffer, unsigned long frames, float rate, float depth, float mix);
    void applyFlanger(float* buffer, unsigned long frames, float rate, float depth, float feedback);
    
    // Modulations-Effekte
    void applyTremolo(float* buffer, unsigned long frames, float rate, float depth);
    void applyPhaser(float* buffer, unsigned long frames, float rate, float depth, float feedback);
    
    // Filter
    void applyLowPass(float* buffer, unsigned long frames, float cutoff);
    void applyHighPass(float* buffer, unsigned long frames, float cutoff);
    void applyBandPass(float* buffer, unsigned long frames, float centerFreq, float bandwidth);

private:
    // FFT-Buffer
    std::vector<float> fftBuffer;
    std::vector<float> windowBuffer;
    
    // Delay-Line
    std::vector<float> delayBuffer;
    size_t delayWritePos;
    
    // LFO für Modulation
    float lfoPhase;
    float lfoRate;
    
    // Filter-Koeffizienten
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
    
    void initializeFFT();
    void applyWindow(float* buffer, unsigned long frames);
    void updateFilterCoefficients(float cutoff, float resonance);
};

} // namespace VR_DAW 