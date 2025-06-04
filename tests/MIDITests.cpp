#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWMIDITest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
        daw->initializeMIDI();
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
};

// MIDI-Initialisierung Test
TEST_F(VRDAWMIDITest, MIDIInitialization) {
    EXPECT_TRUE(daw->isMIDIInitialized());
    EXPECT_TRUE(daw->isMIDIEngineActive());
    
    auto midiInfo = daw->getMIDIInfo();
    EXPECT_GT(midiInfo.inputDeviceCount, 0);
    EXPECT_GT(midiInfo.outputDeviceCount, 0);
    EXPECT_GT(midiInfo.supportedChannels, 0);
}

// MIDI-Eingabe Test
TEST_F(VRDAWMIDITest, MIDIInput) {
    // Simuliere MIDI-Eingabe
    auto inputDevice = daw->getMIDIInputDevice(0);
    EXPECT_TRUE(daw->isMIDIDeviceValid(inputDevice));
    
    // Simuliere Note-On
    daw->simulateMIDINoteOn(inputDevice, 60, 100); // Middle C, Velocity 100
    auto noteOnEvent = daw->getLastMIDIEvent();
    EXPECT_EQ(noteOnEvent.type, "note_on");
    EXPECT_EQ(noteOnEvent.note, 60);
    EXPECT_EQ(noteOnEvent.velocity, 100);
    
    // Simuliere Note-Off
    daw->simulateMIDINoteOff(inputDevice, 60, 0);
    auto noteOffEvent = daw->getLastMIDIEvent();
    EXPECT_EQ(noteOffEvent.type, "note_off");
    EXPECT_EQ(noteOffEvent.note, 60);
    EXPECT_EQ(noteOffEvent.velocity, 0);
}

// MIDI-Ausgabe Test
TEST_F(VRDAWMIDITest, MIDIOutput) {
    // Simuliere MIDI-Ausgabe
    auto outputDevice = daw->getMIDIOutputDevice(0);
    EXPECT_TRUE(daw->isMIDIDeviceValid(outputDevice));
    
    // Sende MIDI-Nachrichten
    daw->sendMIDINoteOn(outputDevice, 60, 100);
    daw->sendMIDINoteOff(outputDevice, 60, 0);
    
    // Überprüfe Ausgabe
    auto outputEvents = daw->getMIDIOutputEvents();
    EXPECT_EQ(outputEvents.size(), 2);
    EXPECT_EQ(outputEvents[0].type, "note_on");
    EXPECT_EQ(outputEvents[1].type, "note_off");
}

// MIDI-Clock Test
TEST_F(VRDAWMIDITest, MIDIClock) {
    // Starte MIDI-Clock
    daw->startMIDIClock();
    EXPECT_TRUE(daw->isMIDIClockRunning());
    
    // Überprüfe Clock-Events
    const int DURATION_SECONDS = 5;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int clockCount = 0;
    while (std::chrono::high_resolution_clock::now() < end) {
        if (daw->hasMIDIClockTick()) {
            clockCount++;
        }
    }
    
    // Überprüfe Clock-Rate (24 PPQN)
    float expectedClocks = DURATION_SECONDS * 24.0f;
    EXPECT_NEAR(clockCount, expectedClocks, expectedClocks * 0.1f); // 10% Toleranz
}

// MIDI-Performance Test
TEST_F(VRDAWMIDITest, MIDIPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int eventCount = 0;
    int failedEvents = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Simuliere MIDI-Events
        auto inputDevice = daw->getMIDIInputDevice(0);
        daw->simulateMIDINoteOn(inputDevice, 60, 100);
        daw->simulateMIDINoteOff(inputDevice, 60, 0);
        
        eventCount += 2;
        
        if (!daw->isMIDIEventValid(daw->getLastMIDIEvent())) {
            failedEvents++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getMIDIMetrics();
        EXPECT_LT(metrics.cpuUsage, 20.0f); // Maximal 20% CPU
        EXPECT_LT(metrics.memoryUsage, 50); // Maximal 50MB RAM
        EXPECT_LT(metrics.latency, 5.0f); // Maximal 5ms Latenz
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedEvents / eventCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// MIDI-Fehlerbehandlung Test
TEST_F(VRDAWMIDITest, MIDIErrorHandling) {
    // Teste ungültige MIDI-Geräte
    auto invalidInput = daw->getMIDIInputDevice(-1);
    EXPECT_FALSE(daw->isMIDIDeviceValid(invalidInput));
    
    auto invalidOutput = daw->getMIDIOutputDevice(-1);
    EXPECT_FALSE(daw->isMIDIDeviceValid(invalidOutput));
    
    // Teste ungültige MIDI-Events
    daw->simulateMIDINoteOn(invalidInput, 60, 100);
    EXPECT_FALSE(daw->isMIDIEventValid(daw->getLastMIDIEvent()));
    
    daw->sendMIDINoteOn(invalidOutput, 60, 100);
    EXPECT_FALSE(daw->isMIDIEventValid(daw->getLastMIDIEvent()));
    
    // Überprüfe Fehlerwiederherstellung
    EXPECT_TRUE(daw->isMIDIInitialized());
    EXPECT_TRUE(daw->isMIDIEngineActive());
}

// MIDI-Integration Test
TEST_F(VRDAWMIDITest, MIDIIntegration) {
    auto projectId = daw->createProject("MIDI Test");
    auto trackId = daw->createTrack(projectId, "MIDI Track");
    
    // Erstelle MIDI-Spur
    auto midiTrackId = daw->createMIDITrack(trackId);
    EXPECT_TRUE(daw->isMIDITrackValid(midiTrackId));
    
    // Füge MIDI-Noten hinzu
    daw->addMIDINote(midiTrackId, 60, 0.0f, 1.0f, 100); // Middle C, 1 Sekunde
    daw->addMIDINote(midiTrackId, 64, 1.0f, 1.0f, 100); // E4, 1 Sekunde
    daw->addMIDINote(midiTrackId, 67, 2.0f, 1.0f, 100); // G4, 1 Sekunde
    
    // Überprüfe MIDI-Spur
    auto midiTrackInfo = daw->getMIDITrackInfo(midiTrackId);
    EXPECT_EQ(midiTrackInfo.noteCount, 3);
    
    // Überprüfe MIDI-Noten
    auto notes = daw->getMIDINotes(midiTrackId);
    EXPECT_EQ(notes.size(), 3);
    EXPECT_EQ(notes[0].note, 60);
    EXPECT_EQ(notes[1].note, 64);
    EXPECT_EQ(notes[2].note, 67);
}

} // namespace Tests
} // namespace VR_DAW 