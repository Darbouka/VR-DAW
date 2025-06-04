#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace VR_DAW {

class VoiceVocoderBank {
public:
    VoiceVocoderBank();
    ~VoiceVocoderBank();
    
    // Initialisierung und Shutdown
    void initialize();
    void shutdown();
    
    // Vocoder-Modi
    enum class VocoderMode {
        Classic,        // Klassischer Vocoder
        Robot,          // Roboter-Stimme
        Choir,          // Chor-Effekt
        Harmony,        // Harmonisierung
        Formant,        // Formant-Verschiebung
        Granular,       // Granularer Vocoder
        Spectral,       // Spektraler Vocoder
        PhaseVocoder,   // Phase-Vocoder
        Morphing,       // Stimmen-Morphing
        Neural,         // KI-basierter Vocoder
        GoaGenre,       // Goa-Genre Vocoder
        GenreVocoder,   // Umfassender Genre-Vocoder
        
        // Neue internationale Modi
        Koto,           // Japanischer Koto-Effekt
        Tuvan,          // Tuwinischer Kehlkopfgesang
        Mongolian,      // Mongolischer Kehlkopfgesang
        Gregorian,      // Gregorianischer Choral
        Opera,          // Opernstimme
        Bollywood,      // Bollywood-Gesang
        Flamenco,       // Flamenco-Gesang
        Blues,          // Blues-Gesang
        Reggae,         // Reggae-Gesang
        Gospel,         // Gospel-Gesang
        Jazz,           // Jazz-Gesang
        Raga,           // Indischer Raga-Gesang
        Sufi,           // Sufi-Gesang
        Celtic,         // Keltischer Gesang
        African,        // Afrikanischer Gesang
        Arabic,         // Arabischer Gesang
        Chinese,        // Chinesischer Gesang
        Korean,         // Koreanischer Gesang
        Vietnamese,     // Vietnamesischer Gesang
        Thai,           // Thailändischer Gesang
        Psytrance      // Psychedelischer Trance
    };
    
    // Modus-Verwaltung
    void setMode(VocoderMode mode);
    VocoderMode getCurrentMode() const;
    void addCustomMode(const std::string& name, const std::vector<float>& parameters);
    void removeCustomMode(const std::string& name);
    
    // Parameter-Steuerung
    void setParameter(const std::string& name, float value);
    float getParameter(const std::string& name) const;
    
    // Audio-Verarbeitung
    void processBlock(juce::AudioBuffer<float>& buffer);
    void processFile(const std::string& inputPath, const std::string& outputPath);
    
    // Modell-Analyse
    void analyzeModel(const std::string& modelPath);
    void saveModel(const std::string& outputPath);
    void loadModel(const std::string& inputPath);
    
    // Preset-Verwaltung
    void savePreset(const std::string& name);
    void loadPreset(const std::string& name);
    void deletePreset(const std::string& name);
    std::vector<std::string> getAvailablePresets() const;
    
private:
    // DSP-Komponenten
    struct DSPComponents {
        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::Vocoder> vocoder;
        std::unique_ptr<juce::dsp::Harmonizer> harmonizer;
        std::unique_ptr<juce::dsp::FormantShifter> formantShifter;
        std::unique_ptr<juce::dsp::GranularProcessor> granularProcessor;
        std::unique_ptr<juce::dsp::SpectralProcessor> spectralProcessor;
        std::unique_ptr<juce::dsp::PhaseVocoder> phaseVocoder;
        std::unique_ptr<juce::dsp::MorphingProcessor> morphingProcessor;
        std::unique_ptr<juce::dsp::NeuralProcessor> neuralProcessor;
    } dsp;
    
    // Verarbeitungsparameter
    struct ProcessingParameters {
        // Allgemeine Parameter
        float carrierLevel = 1.0f;
        float modulatorLevel = 1.0f;
        float dryWet = 0.5f;
        
        // Modus-spezifische Parameter
        struct {
            int numBands = 16;
            float bandwidth = 0.5f;
            float attack = 0.01f;
            float release = 0.1f;
        } classic;
        
        struct {
            float pitchShift = 0.0f;
            float formantShift = 0.0f;
            float metallicAmount = 0.5f;
        } robot;
        
        struct {
            int numVoices = 4;
            float detune = 0.1f;
            float spread = 0.5f;
        } choir;
        
        struct {
            std::vector<float> harmonies;
            float blend = 0.5f;
            float pitchTracking = 0.8f;
        } harmony;
        
        struct {
            float formantShift = 0.0f;
            float formantScale = 1.0f;
            float formantPreserve = 0.5f;
        } formant;
        
        struct {
            float grainSize = 0.1f;
            float density = 0.5f;
            float pitch = 0.0f;
        } granular;
        
        struct {
            float spectralShift = 0.0f;
            float spectralScale = 1.0f;
            float spectralPreserve = 0.5f;
        } spectral;
        
        struct {
            float timeStretch = 1.0f;
            float pitchShift = 0.0f;
            float phasePreserve = 0.5f;
        } phase;
        
        struct {
            float morphAmount = 0.0f;
            std::string targetVoice;
            float formantPreserve = 0.5f;
        } morphing;
        
        struct {
            float styleStrength = 0.5f;
            std::string styleModel;
            float creativity = 0.5f;
        } neural;

        // Neue modus-spezifische Parameter
        struct {
            float stringResonance = 0.5f;
            float pluckIntensity = 0.5f;
            float kotoBlend = 0.5f;
        } koto;

        struct {
            float throatTension = 0.5f;
            float harmonicBalance = 0.5f;
            float droneIntensity = 0.5f;
        } tuvan;

        struct {
            float chestVoice = 0.5f;
            float headVoice = 0.5f;
            float throatSinging = 0.5f;
        } mongolian;

        struct {
            float reverbSize = 0.5f;
            float choirSize = 0.5f;
            float modalVoice = 0.5f;
        } gregorian;

        struct {
            float vibratoDepth = 0.5f;
            float vibratoRate = 0.5f;
            float operaticResonance = 0.5f;
        } opera;

        struct {
            float melisma = 0.5f;
            float ornamentation = 0.5f;
            float tablaSync = 0.5f;
        } bollywood;

        struct {
            float flamencoStyle = 0.5f;
            float palmasSync = 0.5f;
            float guitarraSync = 0.5f;
        } flamenco;

        struct {
            float blueNotes = 0.5f;
            float slideIntensity = 0.5f;
            float soulIntensity = 0.5f;
        } blues;

        struct {
            float offbeatSync = 0.5f;
            float dubEcho = 0.5f;
            float skankIntensity = 0.5f;
        } reggae;

        struct {
            float choirSize = 0.5f;
            float callResponse = 0.5f;
            float spiritualIntensity = 0.5f;
        } gospel;

        struct {
            float swingAmount = 0.5f;
            float scatIntensity = 0.5f;
            float jazzVibrato = 0.5f;
        } jazz;

        struct {
            float ragaScale = 0.5f;
            float droneIntensity = 0.5f;
            float ornamentation = 0.5f;
        } raga;

        struct {
            float tranceIntensity = 0.5f;
            float spiritualEcho = 0.5f;
            float whirlingIntensity = 0.5f;
        } sufi;

        struct {
            float modalVoice = 0.5f;
            float ornamentation = 0.5f;
            float harpSync = 0.5f;
        } celtic;

        struct {
            float polyrhythm = 0.5f;
            float callResponse = 0.5f;
            float percussionSync = 0.5f;
        } african;

        struct {
            float maqamScale = 0.5f;
            float ornamentation = 0.5f;
            float oudSync = 0.5f;
        } arabic;

        struct {
            float pentatonicScale = 0.5f;
            float ornamentation = 0.5f;
            float guzhengSync = 0.5f;
        } chinese;

        struct {
            float pansoriStyle = 0.5f;
            float ornamentation = 0.5f;
            float gayageumSync = 0.5f;
        } korean;

        struct {
            float tonalLanguage = 0.5f;
            float ornamentation = 0.5f;
            float danTranhSync = 0.5f;
        } vietnamese;

        struct {
            float pentatonicScale = 0.5f;
            float ornamentation = 0.5f;
            float khimSync = 0.5f;
        } thai;

        struct {
            float acidFilter = 0.5f;
            float resonance = 0.5f;
            float lfoRate = 0.5f;
            float lfoDepth = 0.5f;
            float delayFeedback = 0.5f;
            float reverbSize = 0.5f;
            float distortion = 0.5f;
            float stereoWidth = 0.5f;
            float filterModulation = 0.5f;
            float glitchIntensity = 0.5f;
        } psytrance;

        struct {
            float acidFilter = 0.7f;
            float resonance = 0.8f;
            float lfoRate = 0.6f;
            float lfoDepth = 0.7f;
            float delayFeedback = 0.6f;
            float reverbSize = 0.8f;
            float distortion = 0.4f;
            float stereoWidth = 0.9f;
            float filterModulation = 0.8f;
            float glitchIntensity = 0.3f;
            float phaserRate = 0.5f;
            float phaserDepth = 0.6f;
            float flangerRate = 0.4f;
            float flangerDepth = 0.5f;
            float chorusRate = 0.3f;
            float chorusDepth = 0.4f;
            float sitarResonance = 0.6f;
            float tablaSync = 0.5f;
            float droneIntensity = 0.7f;
            float spiritualEcho = 0.8f;
        } goaTrance;

        struct {
            // Allgemeine Goa-Parameter
            float acidFilter = 0.7f;
            float resonance = 0.8f;
            float lfoRate = 0.6f;
            float lfoDepth = 0.7f;
            float delayFeedback = 0.6f;
            float reverbSize = 0.8f;
            float distortion = 0.4f;
            float stereoWidth = 0.9f;
            float filterModulation = 0.8f;
            float glitchIntensity = 0.3f;
            
            // Klassischer Goa
            struct {
                float sitarResonance = 0.8f;
                float tablaSync = 0.7f;
                float droneIntensity = 0.9f;
                float spiritualEcho = 0.8f;
                float indianScale = 0.9f;
            } classicGoa;
            
            // Progressive Goa
            struct {
                float progressiveFilter = 0.7f;
                float grooveIntensity = 0.8f;
                float bassModulation = 0.6f;
                float atmosphericDepth = 0.7f;
                float melodicProgression = 0.8f;
            } progressiveGoa;
            
            // Dark Goa
            struct {
                float darkAtmosphere = 0.9f;
                float industrialElements = 0.8f;
                float distortionAmount = 0.7f;
                float darkReverb = 0.8f;
                float metallicResonance = 0.7f;
            } darkGoa;
            
            // Forest Goa
            struct {
                float organicElements = 0.9f;
                float natureSounds = 0.8f;
                float tribalRhythms = 0.7f;
                float shamanicEcho = 0.8f;
                float forestAtmosphere = 0.9f;
            } forestGoa;
            
            // Psychedelic Goa
            struct {
                float psychedelicFilter = 0.8f;
                float mindExpansion = 0.9f;
                float trippyEffects = 0.8f;
                float consciousnessShift = 0.7f;
                float spiritualDepth = 0.8f;
            } psychedelicGoa;
            
            // Suomisaundi
            struct {
                float experimentalElements = 0.9f;
                float glitchIntensity = 0.8f;
                float acidModulation = 0.7f;
                float experimentalEffects = 0.8f;
                float creativeDistortion = 0.7f;
            } suomisaundi;
            
            // Zenonesque
            struct {
                float minimalElements = 0.8f;
                float deepAtmosphere = 0.9f;
                float hypnoticRhythms = 0.7f;
                float meditativeSpace = 0.8f;
                float zenBalance = 0.9f;
            } zenonesque;
            
            // Twilight Goa
            struct {
                float twilightAtmosphere = 0.9f;
                float mysticalElements = 0.8f;
                float etherealSpace = 0.7f;
                float cosmicEcho = 0.8f;
                float spiritualJourney = 0.9f;
            } twilightGoa;
            
            // Morning Goa
            struct {
                float morningLight = 0.8f;
                float upliftingElements = 0.9f;
                float positiveEnergy = 0.7f;
                float sunriseAtmosphere = 0.8f;
                float awakeningVibes = 0.9f;
            } morningGoa;
            
            // Full On
            struct {
                float energyLevel = 0.9f;
                float drivingForce = 0.8f;
                float peakIntensity = 0.7f;
                float euphoricElements = 0.8f;
                float dancefloorPower = 0.9f;
            } fullOn;
        } goaGenre;

        struct {
            // Techno-Genres
            struct {
                // Minimal Techno
                struct {
                    float minimalElements = 0.8f;
                    float grooveIntensity = 0.7f;
                    float spaceAtmosphere = 0.6f;
                    float subtleModulation = 0.5f;
                    float hypnoticRhythms = 0.7f;
                } minimalTechno;
                
                // Industrial Techno
                struct {
                    float industrialElements = 0.9f;
                    float metallicResonance = 0.8f;
                    float darkAtmosphere = 0.9f;
                    float distortionAmount = 0.7f;
                    float mechanicalRhythms = 0.8f;
                } industrialTechno;
                
                // Acid Techno
                struct {
                    float acidFilter = 0.9f;
                    float resonance = 0.8f;
                    float lfoModulation = 0.7f;
                    float squelchIntensity = 0.8f;
                    float acidGroove = 0.7f;
                } acidTechno;
                
                // Detroit Techno
                struct {
                    float soulElements = 0.8f;
                    float funkGroove = 0.7f;
                    float spaceAtmosphere = 0.6f;
                    float melodicElements = 0.7f;
                    float futuristicVibes = 0.8f;
                } detroitTechno;
                
                // Berlin Techno
                struct {
                    float darkAtmosphere = 0.9f;
                    float industrialElements = 0.8f;
                    float hypnoticRhythms = 0.7f;
                    float spaceAtmosphere = 0.6f;
                    float undergroundVibes = 0.8f;
                } berlinTechno;
                
                // Hard Techno
                struct {
                    float energyLevel = 0.9f;
                    float distortionAmount = 0.8f;
                    float aggressiveElements = 0.9f;
                    float drivingForce = 0.8f;
                    float peakIntensity = 0.7f;
                } hardTechno;
            } techno;
            
            // House-Genres
            struct {
                // Deep House
                struct {
                    float deepAtmosphere = 0.8f;
                    float soulElements = 0.7f;
                    float grooveIntensity = 0.6f;
                    float spaceAtmosphere = 0.7f;
                    float jazzyElements = 0.6f;
                } deepHouse;
                
                // Tech House
                struct {
                    float technoElements = 0.7f;
                    float houseGroove = 0.8f;
                    float minimalElements = 0.6f;
                    float drivingForce = 0.7f;
                    float techAtmosphere = 0.8f;
                } techHouse;
                
                // Progressive House
                struct {
                    float progressiveElements = 0.8f;
                    float melodicProgression = 0.7f;
                    float atmosphericDepth = 0.8f;
                    float emotionalIntensity = 0.7f;
                    float journeyElements = 0.8f;
                } progressiveHouse;
                
                // Afro House
                struct {
                    float africanElements = 0.9f;
                    float tribalRhythms = 0.8f;
                    float organicElements = 0.7f;
                    float spiritualVibes = 0.8f;
                    float grooveIntensity = 0.7f;
                } afroHouse;
            } house;
            
            // Drum & Bass
            struct {
                // Liquid DnB
                struct {
                    float liquidElements = 0.8f;
                    float soulElements = 0.7f;
                    float atmosphericDepth = 0.8f;
                    float melodicElements = 0.7f;
                    float smoothGroove = 0.8f;
                } liquidDnB;
                
                // Neurofunk
                struct {
                    float neuroElements = 0.9f;
                    float aggressiveElements = 0.8f;
                    float complexRhythms = 0.9f;
                    float darkAtmosphere = 0.8f;
                    float technicalElements = 0.9f;
                } neurofunk;
                
                // Jump Up
                struct {
                    float energyLevel = 0.9f;
                    float bouncyElements = 0.8f;
                    float aggressiveElements = 0.7f;
                    float drivingForce = 0.8f;
                    float peakIntensity = 0.7f;
                } jumpUp;
            } drumAndBass;
            
            // Trance-Genres
            struct {
                // Progressive Trance
                struct {
                    float progressiveElements = 0.8f;
                    float melodicProgression = 0.7f;
                    float atmosphericDepth = 0.8f;
                    float emotionalIntensity = 0.7f;
                    float journeyElements = 0.8f;
                } progressiveTrance;
                
                // Uplifting Trance
                struct {
                    float upliftingElements = 0.9f;
                    float emotionalIntensity = 0.8f;
                    float melodicElements = 0.9f;
                    float euphoricElements = 0.8f;
                    float peakIntensity = 0.9f;
                } upliftingTrance;
                
                // Psytrance
                struct {
                    float psychedelicElements = 0.9f;
                    float acidElements = 0.8f;
                    float spiritualElements = 0.7f;
                    float trippyEffects = 0.8f;
                    float mindExpansion = 0.9f;
                } psytrance;
            } trance;
            
            // Dubstep
            struct {
                // Melodic Dubstep
                struct {
                    float melodicElements = 0.8f;
                    float emotionalIntensity = 0.7f;
                    float atmosphericDepth = 0.8f;
                    float bassWeight = 0.7f;
                    float journeyElements = 0.8f;
                } melodicDubstep;
                
                // Riddim
                struct {
                    float aggressiveElements = 0.9f;
                    float bassWeight = 0.8f;
                    float minimalElements = 0.7f;
                    float drivingForce = 0.8f;
                    float peakIntensity = 0.7f;
                } riddim;
            } dubstep;
            
            // Weltweite elektronische Musik
            struct {
                // Afrobeat
                struct {
                    float africanElements = 0.9f;
                    float tribalRhythms = 0.8f;
                    float organicElements = 0.7f;
                    float spiritualVibes = 0.8f;
                    float grooveIntensity = 0.7f;
                } afrobeat;
                
                // Baile Funk
                struct {
                    float brazilianElements = 0.9f;
                    float funkElements = 0.8f;
                    float partyVibes = 0.7f;
                    float grooveIntensity = 0.8f;
                    float energyLevel = 0.7f;
                } baileFunk;
                
                // K-Pop
                struct {
                    float koreanElements = 0.8f;
                    float popElements = 0.7f;
                    float electronicElements = 0.8f;
                    float vocalProcessing = 0.7f;
                    float energyLevel = 0.8f;
                } kpop;
                
                // J-Pop
                struct {
                    float japaneseElements = 0.8f;
                    float popElements = 0.7f;
                    float electronicElements = 0.8f;
                    float vocalProcessing = 0.7f;
                    float energyLevel = 0.8f;
                } jpop;
            } worldElectronic;
            
            // Traditionelle Musik
            struct {
                // Indische Klassik
                struct {
                    float ragaElements = 0.9f;
                    float sitarResonance = 0.8f;
                    float tablaSync = 0.7f;
                    float droneIntensity = 0.8f;
                    float spiritualElements = 0.9f;
                } indianClassical;
                
                // Arabische Musik
                struct {
                    float maqamElements = 0.9f;
                    float oudResonance = 0.8f;
                    float percussionSync = 0.7f;
                    float spiritualElements = 0.8f;
                    float modalElements = 0.9f;
                } arabicMusic;
                
                // Chinesische Musik
                struct {
                    float pentatonicElements = 0.9f;
                    float guzhengResonance = 0.8f;
                    float erhuElements = 0.7f;
                    float traditionalElements = 0.8f;
                    float spiritualElements = 0.9f;
                } chineseMusic;
                
                // Afrikanische Musik
                struct {
                    float tribalElements = 0.9f;
                    float percussionSync = 0.8f;
                    float spiritualElements = 0.7f;
                    float organicElements = 0.8f;
                    float grooveIntensity = 0.9f;
                } africanMusic;
            } traditional;
        } genreVocoder;
    } parameters;
    
    // Interne Hilfsfunktionen
    void initializeDSP();
    void processClassicVocoder(juce::AudioBuffer<float>& buffer);
    void processRobotVocoder(juce::AudioBuffer<float>& buffer);
    void processChoirVocoder(juce::AudioBuffer<float>& buffer);
    void processHarmonyVocoder(juce::AudioBuffer<float>& buffer);
    void processFormantVocoder(juce::AudioBuffer<float>& buffer);
    void processGranularVocoder(juce::AudioBuffer<float>& buffer);
    void processSpectralVocoder(juce::AudioBuffer<float>& buffer);
    void processPhaseVocoder(juce::AudioBuffer<float>& buffer);
    void processMorphingVocoder(juce::AudioBuffer<float>& buffer);
    void processNeuralVocoder(juce::AudioBuffer<float>& buffer);
    
    // Neue Verarbeitungsfunktionen
    void processKotoVocoder(juce::AudioBuffer<float>& buffer);
    void processTuvanVocoder(juce::AudioBuffer<float>& buffer);
    void processMongolianVocoder(juce::AudioBuffer<float>& buffer);
    void processGregorianVocoder(juce::AudioBuffer<float>& buffer);
    void processOperaVocoder(juce::AudioBuffer<float>& buffer);
    void processBollywoodVocoder(juce::AudioBuffer<float>& buffer);
    void processFlamencoVocoder(juce::AudioBuffer<float>& buffer);
    void processBluesVocoder(juce::AudioBuffer<float>& buffer);
    void processReggaeVocoder(juce::AudioBuffer<float>& buffer);
    void processGospelVocoder(juce::AudioBuffer<float>& buffer);
    void processJazzVocoder(juce::AudioBuffer<float>& buffer);
    void processRagaVocoder(juce::AudioBuffer<float>& buffer);
    void processSufiVocoder(juce::AudioBuffer<float>& buffer);
    void processCelticVocoder(juce::AudioBuffer<float>& buffer);
    void processAfricanVocoder(juce::AudioBuffer<float>& buffer);
    void processArabicVocoder(juce::AudioBuffer<float>& buffer);
    void processChineseVocoder(juce::AudioBuffer<float>& buffer);
    void processKoreanVocoder(juce::AudioBuffer<float>& buffer);
    void processVietnameseVocoder(juce::AudioBuffer<float>& buffer);
    void processThaiVocoder(juce::AudioBuffer<float>& buffer);
    void processPsytranceVocoder(juce::AudioBuffer<float>& buffer);
    void processGoaTranceVocoder(juce::AudioBuffer<float>& buffer);
    void processGoaGenreVocoder(juce::AudioBuffer<float>& buffer);
    void processGenreVocoder(juce::AudioBuffer<float>& buffer);
    
    // Modus und Presets
    VocoderMode currentMode;
    std::map<std::string, std::vector<float>> customModes;
    std::map<std::string, ProcessingParameters> presets;
};

} // namespace VR_DAW 