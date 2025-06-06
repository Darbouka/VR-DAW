#pragma once

#include <string>
#include <vector>
#include <memory>

namespace VR_DAW {

class Effects {
public:
    Effects();
    virtual ~Effects() = default;

    // Basis-Funktionen
    virtual void process(float* buffer, unsigned long framesPerBuffer) = 0;
    virtual void setParameter(const std::string& name, float value) = 0;
    virtual float getParameter(const std::string& name) const = 0;
    virtual std::vector<std::string> getParameterNames() const = 0;
    
    // Effekt-spezifische Funktionen
    virtual void reset() = 0;
    virtual void setBypass(bool bypass) { this->bypass = bypass; }
    virtual bool isBypassed() const { return bypass; }
    
    // Effekt-Typ
    virtual std::string getType() const = 0;
    virtual std::string getName() const = 0;

protected:
    bool bypass;
    float sampleRate;
};

// Konkrete Effekt-Klassen
class ReverbEffect : public Effects {
public:
    ReverbEffect();
    void process(float* buffer, unsigned long framesPerBuffer) override;
    void setParameter(const std::string& name, float value) override;
    float getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getType() const override { return "Reverb"; }
    std::string getName() const override { return "Reverb"; }

private:
    float mix;
    float time;
    float damping;
    std::vector<float> delayBuffer;
    size_t writePos;
};

class DelayEffect : public Effects {
public:
    DelayEffect();
    void process(float* buffer, unsigned long framesPerBuffer) override;
    void setParameter(const std::string& name, float value) override;
    float getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getType() const override { return "Delay"; }
    std::string getName() const override { return "Delay"; }

private:
    float time;
    float feedback;
    float mix;
    std::vector<float> delayBuffer;
    size_t writePos;
};

class CompressorEffect : public Effects {
public:
    CompressorEffect();
    void process(float* buffer, unsigned long framesPerBuffer) override;
    void setParameter(const std::string& name, float value) override;
    float getParameter(const std::string& name) const override;
    std::vector<std::string> getParameterNames() const override;
    void reset() override;
    std::string getType() const override { return "Compressor"; }
    std::string getName() const override { return "Compressor"; }

private:
    float threshold;
    float ratio;
    float attack;
    float release;
    float envelope;
};

} // namespace VR_DAW 