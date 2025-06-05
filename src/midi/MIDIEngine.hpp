#pragma once

#include <cstdint>

namespace VR_DAW {

struct MIDIMessage {
    enum class Type {
        NoteOn,
        NoteOff,
        ControlChange,
        PitchBend,
        ProgramChange,
        Aftertouch,
        PolyAftertouch
    };

    Type type;
    uint8_t channel;
    uint8_t value1;
    uint8_t value2;
};

} // namespace VR_DAW 