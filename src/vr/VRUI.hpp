#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include "midi/MIDIEngine.hpp"
#include "audio/AudioEvent.hpp"
#include "audio/SynthesizerConfig.hpp"
#include "audio/AudioEngine.hpp"

// JACK-Audio-System optional machen
#ifdef USE_JACK
#include <jack/jack.h>
#endif

namespace VR_DAW {

class VRUI {
public:
    struct UIElement {
        enum class Type {
            Button,
            Slider,
            Knob,
            Waveform,
            Text
        };

        Type type;
        std::string id;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        bool visible;
        bool interactive;
        std::string text;
        float value;
        std::function<void()> onClick;
    };

    struct TrackView {
        int trackId;
        std::string name;
        glm::vec3 position;
        glm::vec3 size;
        std::vector<UIElement> controls;
    };

    struct PluginView {
        int pluginId;
        std::string name;
        glm::vec3 position;
        glm::vec3 size;
        std::vector<UIElement> parameters;
    };

    struct TextElement {
        std::string text;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec4 color;
        float fontSize;
        std::string fontName;
    };

    struct Animation {
        enum class Type {
            Position,
            Scale,
            Rotation
        };

        Type type;
        glm::vec3 startValue;
        glm::vec3 endValue;
        float duration;
        float elapsed;
        std::function<float(float)> easingFunction;
    };

    // Synthesizer-spezifische UI-Elemente
    struct SynthesizerView {
        int trackId;
        std::string name;
        glm::vec3 position;
        glm::vec3 size;
        std::vector<UIElement> controls;
        std::vector<UIElement> oscillators;
        std::vector<UIElement> filters;
        std::vector<UIElement> envelopes;
        std::vector<UIElement> lfos;
        std::vector<UIElement> effects;
    };

    struct ParameterRange {
        float min;
        float max;
        float defaultValue;
        std::string unit;
    };

    struct ParameterValidation {
        static ParameterRange getRange(const std::string& paramName);
        static bool validateValue(const std::string& paramName, float value);
        static float normalizeValue(const std::string& paramName, float value);
        static float denormalizeValue(const std::string& paramName, float normalizedValue);
    };

    VRUI();
    ~VRUI();

    bool initialize();
    void shutdown();
    void update();
    void render();

    UIElement* createButton(const std::string& id, const glm::vec3& position, const glm::vec3& scale);
    UIElement* createSlider(const std::string& id, const glm::vec3& position, const glm::vec3& scale);
    UIElement* createKnob(const std::string& id, const glm::vec3& position, float radius);
    UIElement* createWaveform(const std::string& id, const glm::vec3& position, const glm::vec3& size);
    TrackView* createTrackView(int trackId, const std::string& name);
    void updateTrackView(TrackView* view, const std::vector<float>& waveform);
    void deleteTrackView(TrackView* view);

    void handleControllerInput(const glm::vec3& position, const glm::vec3& direction);
    void handleGesture(const std::string& gestureType, const glm::vec3& position);
    void handleVoiceCommand(const std::string& command);

    void setLayout(const std::string& layoutType);
    void updateLayout();
    void arrangeTracks();
    void arrangePlugins();

    void focusElement(const std::string& elementId);
    void renderElement(const UIElement& element);
    void renderTrackView(const TrackView& view);
    void renderPluginView(const PluginView& view);

    UIElement* findElementAtPosition(const glm::vec3& position);
    bool isPointInElement(const glm::vec3& point, const UIElement& element);

    void updateAnimations();
    void animateElement(UIElement& element, const glm::vec3& targetPosition, float duration);
    glm::mat4 calculateModelMatrix(const UIElement& element) const;
    void updateElementTransform(UIElement& element);

    void initializeFontSystem();
    void loadFont(const std::string& fontName);
    TextElement* createText(const std::string& text, const glm::vec3& position, float fontSize);
    void updateText(TextElement* text, const std::string& newText);
    void renderText(const TextElement& text);

    void addAnimation(UIElement& element, const Animation& animation);
    void removeAnimation(UIElement& element, Animation::Type type);
    void updateAnimations(float deltaTime);
    void setEasingFunction(Animation& animation, const std::string& functionName);
    float applyEasing(float t, const std::function<float(float)>& easing);
    void updateElementAnimation(UIElement& element, const Animation& animation, float t);
    glm::vec3 interpolate(const glm::vec3& start, const glm::vec3& end, float t);

    glm::vec2 calculateTextBounds(const TextElement& text);
    void setWaveformData(const std::string& elementId, const std::vector<float>& data);
    void updateElementTransforms();
    void processInteractions();
    void renderDebugInfo();

    void setRenderScale(float scale);
    void setRenderQuality(int quality);
    void enableDebugRendering(bool enable);

    SynthesizerView* createSynthesizerView(int trackId, const std::string& name);
    void updateSynthesizerView(SynthesizerView* view, const SynthesizerConfig& config);
    void deleteSynthesizerView(SynthesizerView* view);
    void arrangeSynthesizers();

    // Konfigurationsfunktionen
    void setLayout(const std::string& layout);
    void setUIScale(float scale);
    void setDefaultPosition(const glm::vec3& position);

    // MIDI-Callbacks
    void handleMIDIMessage(const MIDIMessage& msg);
    void setAudioCallback(std::function<void(const AudioEvent&)> callback);

    void renderSynthesizerView(const SynthesizerView& view);

    // Fehlerbehandlung
    std::string getLastError() const;
    bool hasError() const;
    std::vector<std::string> getErrorLog() const;
    void clearErrors();

    // Debug-Funktionen
    void setDebugOptions(bool showFPS, bool showMemory, bool showPerformance, bool showElementBounds);

    // Audio-System-Integration
    static int processAudio(jack_nframes_t nframes, void* arg);
    void updateAudioVisualization(const float* audioData, size_t numFrames);
    void handleAudioEvent(const AudioEvent& event);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    bool initialized;
    bool debugEnabled;
    float renderScale;
    int renderQuality;

#ifdef USE_JACK
    jack_client_t* jackClient;
    jack_port_t* inputPort;
    jack_port_t* outputPort;
#endif

    // Synthesizer-spezifische private Methoden
    void createOscillatorControls(SynthesizerView& view, const SynthesizerConfig::OscillatorConfig& config, int index);
    void createFilterControls(SynthesizerView& view, const SynthesizerConfig::FilterConfig& config);
    void createEnvelopeControls(SynthesizerView& view, const SynthesizerConfig::EnvelopeConfig& config, const std::string& name);
    void createLFOControls(SynthesizerView& view, const SynthesizerConfig::LFOConfig& config, int index);
    void createEffectControls(SynthesizerView& view, const SynthesizerConfig::EffectConfig& config, int index);
    void updateOscillatorControls(UIElement& element, const SynthesizerConfig::OscillatorConfig& config);
    void updateFilterControls(UIElement& element, const SynthesizerConfig::FilterConfig& config);
    void updateEnvelopeControls(UIElement& element, const SynthesizerConfig::EnvelopeConfig& config);
    void updateLFOControls(UIElement& element, const SynthesizerConfig::LFOConfig& config);
    void updateEffectControls(UIElement& element, const SynthesizerConfig::EffectConfig& config);

    // Neue private Hilfsfunktionen
    void initializeEasingFunctions();
    void initializeAudioSystem();
    void initializeRenderingSystem();
    void logError(const std::string& error);
    void renderElementBounds(const UIElement& element);

    // Audio-System-Initialisierung
    void initializeAudioSystem();
    void initializeRenderingSystem();
};

} // namespace VR_DAW 