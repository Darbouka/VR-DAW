#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace VR_DAW {

struct AudioEvent {
    enum class Type {
        NoteOn,
        NoteOff,
        ControlChange,
        PitchBend,
        VolumeChange,
        PanChange,
        EffectChange,
        ParameterChange
    };

    Type type;
    int channel;
    int value1;  // z.B. Notennummer oder Controller-Nummer
    int value2;  // z.B. Velocity oder Controller-Wert
    float floatValue;  // Für Pitch-Bend und andere kontinuierliche Werte
    std::string parameterName;  // Für Parameter-Änderungen
    std::vector<float> data;  // Für komplexere Daten
    glm::vec3 position;  // 3D-Position im VR-Raum
};

} // namespace VR_DAW 