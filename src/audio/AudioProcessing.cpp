#include "AudioProcessing.hpp"
#include <cmath>
#include <algorithm>

namespace VR_DAW {

AudioProcessing::AudioProcessing()
    : delayWritePos(0)
    , lfoPhase(0.0f)
    , lfoRate(1.0f)
    , b0(1.0f), b1(0.0f), b2(0.0f)
    , a1(0.0f), a2(0.0f)
    , x1(0.0f), x2(0.0f)
    , y1(0.0f), y2(0.0f)
{
    // FFT-Buffer initialisieren
    fftBuffer.resize(2048);
    windowBuffer.resize(2048);
    
    // Delay-Line initialisieren (für 2 Sekunden bei 44.1kHz)
    delayBuffer.resize(88200);
    
    // Hann-Fenster initialisieren
    for (size_t i = 0; i < windowBuffer.size(); ++i) {
        windowBuffer[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (windowBuffer.size() - 1)));
    }
}

AudioProcessing::~AudioProcessing() = default;

void AudioProcessing::process(float* input, float* output, unsigned long framesPerBuffer) {
    // Kopiere Eingang in Ausgang
    std::copy(input, input + framesPerBuffer * 2, output);
    
    // Basis-Verarbeitung
    applyGain(output, framesPerBuffer, 1.0f);
    applyPan(output, framesPerBuffer, 0.0f);
}

void AudioProcessing::applyGain(float* buffer, unsigned long frames, float gain) {
    for (unsigned long i = 0; i < frames * 2; ++i) {
        buffer[i] *= gain;
    }
}

void AudioProcessing::applyPan(float* buffer, unsigned long frames, float pan) {
    float leftGain = std::sqrt(2.0f) / 2.0f * (1.0f - pan);
    float rightGain = std::sqrt(2.0f) / 2.0f * (1.0f + pan);
    
    for (unsigned long i = 0; i < frames; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        buffer[i * 2] = left * leftGain;
        buffer[i * 2 + 1] = right * rightGain;
    }
}

void AudioProcessing::applyEQ(float* buffer, unsigned long frames, float lowGain, float midGain, float highGain) {
    // Einfacher 3-Band EQ
    float lowCutoff = 200.0f;
    float highCutoff = 2000.0f;
    
    for (unsigned long i = 0; i < frames; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        // Tiefpass
        applyLowPass(&left, 1, lowCutoff);
        applyLowPass(&right, 1, lowCutoff);
        
        // Hochpass
        applyHighPass(&left, 1, highCutoff);
        applyHighPass(&right, 1, highCutoff);
        
        // Mittelband durch Subtraktion
        float midLeft = buffer[i * 2] - left - right;
        float midRight = buffer[i * 2 + 1] - left - right;
        
        // Gain anwenden
        buffer[i * 2] = left * lowGain + midLeft * midGain + right * highGain;
        buffer[i * 2 + 1] = left * lowGain + midRight * midGain + right * highGain;
    }
}

void AudioProcessing::applyCompression(float* buffer, unsigned long frames, float threshold, float ratio, float attack, float release) {
    float envelope = 0.0f;
    float attackCoeff = std::exp(-1.0f / (attack * 44100.0f));
    float releaseCoeff = std::exp(-1.0f / (release * 44100.0f));
    
    for (unsigned long i = 0; i < frames * 2; ++i) {
        float input = std::abs(buffer[i]);
        
        // Hüllkurve
        if (input > envelope) {
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * input;
        } else {
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * input;
        }
        
        // Kompression
        if (envelope > threshold) {
            float gain = threshold + (envelope - threshold) / ratio;
            buffer[i] *= gain / envelope;
        }
    }
}

void AudioProcessing::applyReverb(float* buffer, unsigned long frames, float mix, float time, float damping) {
    // Einfacher Feedback-Delay-Netzwerk Reverb
    float feedback = std::pow(0.001f, 1.0f / (time * 44100.0f));
    
    for (unsigned long i = 0; i < frames; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        // Delay-Line
        size_t delayPos = (delayWritePos + static_cast<size_t>(time * 44100.0f)) % delayBuffer.size();
        float delayedLeft = delayBuffer[delayPos];
        float delayedRight = delayBuffer[delayPos + 1];
        
        // Feedback mit Dämpfung
        delayBuffer[delayWritePos] = left + delayedLeft * feedback * (1.0f - damping);
        delayBuffer[delayWritePos + 1] = right + delayedRight * feedback * (1.0f - damping);
        
        // Mix
        buffer[i * 2] = left * (1.0f - mix) + delayedLeft * mix;
        buffer[i * 2 + 1] = right * (1.0f - mix) + delayedRight * mix;
        
        delayWritePos = (delayWritePos + 2) % delayBuffer.size();
    }
}

void AudioProcessing::applyDelay(float* buffer, unsigned long frames, float delayTime, float feedback) {
    size_t delaySamples = static_cast<size_t>(delayTime * 44100.0f);
    
    for (unsigned long i = 0; i < frames; ++i) {
        float left = buffer[i * 2];
        float right = buffer[i * 2 + 1];
        
        // Delay-Line
        size_t delayPos = (delayWritePos + delaySamples) % delayBuffer.size();
        float delayedLeft = delayBuffer[delayPos];
        float delayedRight = delayBuffer[delayPos + 1];
        
        // Feedback
        delayBuffer[delayWritePos] = left + delayedLeft * feedback;
        delayBuffer[delayWritePos + 1] = right + delayedRight * feedback;
        
        // Mix
        buffer[i * 2] = left + delayedLeft;
        buffer[i * 2 + 1] = right + delayedRight;
        
        delayWritePos = (delayWritePos + 2) % delayBuffer.size();
    }
}

void AudioProcessing::applyLowPass(float* buffer, unsigned long frames, float cutoff) {
    float w0 = 2.0f * M_PI * cutoff / 44100.0f;
    float alpha = std::sin(w0) / (2.0f * 0.707f);
    
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * std::cos(w0);
    float a2 = 1.0f - alpha;
    float b0 = (1.0f - std::cos(w0)) / 2.0f;
    float b1 = 1.0f - std::cos(w0);
    float b2 = (1.0f - std::cos(w0)) / 2.0f;
    
    for (unsigned long i = 0; i < frames; ++i) {
        float x = buffer[i];
        float y = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        buffer[i] = y;
    }
}

void AudioProcessing::applyHighPass(float* buffer, unsigned long frames, float cutoff) {
    float w0 = 2.0f * M_PI * cutoff / 44100.0f;
    float alpha = std::sin(w0) / (2.0f * 0.707f);
    
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * std::cos(w0);
    float a2 = 1.0f - alpha;
    float b0 = (1.0f + std::cos(w0)) / 2.0f;
    float b1 = -(1.0f + std::cos(w0));
    float b2 = (1.0f + std::cos(w0)) / 2.0f;
    
    for (unsigned long i = 0; i < frames; ++i) {
        float x = buffer[i];
        float y = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        buffer[i] = y;
    }
}

void AudioProcessing::applyBandPass(float* buffer, unsigned long frames, float centerFreq, float bandwidth) {
    float w0 = 2.0f * M_PI * centerFreq / 44100.0f;
    float alpha = std::sin(w0) * std::sinh(std::log(2.0f) / 2.0f * bandwidth * w0 / std::sin(w0));
    
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * std::cos(w0);
    float a2 = 1.0f - alpha;
    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    
    for (unsigned long i = 0; i < frames; ++i) {
        float x = buffer[i];
        float y = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        buffer[i] = y;
    }
}

} // namespace VR_DAW 