#pragma once

#include <memory>
#include <vector>
#include <complex>
#include <juce_audio_basics/juce_audio_basics.h>

namespace VR_DAW {

class SpectralAnalyzer {
public:
    static SpectralAnalyzer& getInstance();
    
    enum class Mode {
        RealTime,
        Offline,
        Hybrid
    };
    
    // Initialisierung
    void initialize();
    void shutdown();
    
    // Modus-Steuerung
    void setMode(Mode mode);
    Mode getMode() const;
    
    // Analyse-Funktionen
    void analyzeBuffer(const juce::AudioBuffer<float>& buffer);
    void analyzeFile(const std::string& filePath);
    
    // Spektrum-Zugriff
    std::vector<std::complex<float>> getSpectrum() const;
    std::vector<float> getMagnitudeSpectrum() const;
    std::vector<float> getPhaseSpectrum() const;
    
    // Analyse-Parameter
    void setFFTSize(int size);
    void setWindowType(const std::string& type);
    void setOverlap(float overlap);
    
    // Echtzeit-Analyse
    void startRealTimeAnalysis();
    void stopRealTimeAnalysis();
    bool isRealTimeAnalysisActive() const;
    
    // Offline-Analyse
    void startOfflineAnalysis();
    void stopOfflineAnalysis();
    bool isOfflineAnalysisActive() const;
    
    // Hybrid-Analyse
    void startHybridAnalysis();
    void stopHybridAnalysis();
    bool isHybridAnalysisActive() const;
    
    // Analyse-Ergebnisse
    struct AnalysisResults {
        std::vector<float> frequencies;
        std::vector<float> magnitudes;
        std::vector<float> phases;
        float peakFrequency;
        float peakMagnitude;
        float rms;
        float crestFactor;
        float spectralCentroid;
        float spectralSpread;
        float spectralFlatness;
        float spectralRolloff;
    };
    
    AnalysisResults getAnalysisResults() const;
    
private:
    SpectralAnalyzer() = default;
    ~SpectralAnalyzer() = default;
    
    SpectralAnalyzer(const SpectralAnalyzer&) = delete;
    SpectralAnalyzer& operator=(const SpectralAnalyzer&) = delete;
    
    // Interne Zustandsvariablen
    Mode currentMode = Mode::RealTime;
    int fftSize = 2048;
    std::string windowType = "Hann";
    float overlap = 0.5f;
    bool realTimeActive = false;
    bool offlineActive = false;
    bool hybridActive = false;
    
    // FFT-Komponenten
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<float> window;
    std::vector<std::complex<float>> spectrum;
    std::vector<float> magnitudeSpectrum;
    std::vector<float> phaseSpectrum;
    
    // Analyse-Ergebnisse
    AnalysisResults currentResults;
    
    // Interne Hilfsfunktionen
    void applyWindow(std::vector<float>& buffer);
    void calculateSpectrum(const std::vector<float>& buffer);
    void updateAnalysisResults();
    void calculateSpectralFeatures();
};

} // namespace VR_DAW 