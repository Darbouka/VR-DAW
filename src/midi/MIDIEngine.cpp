#include "MIDIEngine.hpp"
#include <algorithm>
#include <chrono>
#include <thread>

namespace VR_DAW {

struct MIDIEngine::Impl {
    std::vector<MIDIDevice> devices;
    std::map<int, std::unique_ptr<RtMidiIn>> midiInputs;
    std::map<int, std::unique_ptr<RtMidiOut>> midiOutputs;
    std::queue<MIDIMessage> messageQueue;
    std::mutex queueMutex;
    std::atomic<bool> isClockRunning;
    double tempo;
    std::function<void(const MIDIMessage&)> messageCallback;
    std::function<void(double)> clockCallback;
    std::vector<MIDIMessage> recordedMessages;
    std::atomic<bool> isRecordingActive;
    std::thread clockThread;
};

MIDIEngine::MIDIEngine()
    : pImpl(std::make_unique<Impl>())
    , initialized(false)
    , isClockRunning(false)
    , tempo(120.0)
    , isRecordingActive(false)
{
}

MIDIEngine::~MIDIEngine() {
    shutdown();
}

void MIDIEngine::initialize() {
    try {
        // MIDI-Geräte initialisieren
        for (const auto& device : getAvailableDevices()) {
            if (!initializeDevice(device)) {
                throw MIDIError("Fehler beim Initialisieren des MIDI-Geräts: " + device.name);
            }
        }
        
        // MIDI-Callback registrieren
        if (!registerCallback()) {
            throw MIDIError("Fehler beim Registrieren des MIDI-Callbacks");
        }
        
        initialized = true;
    } catch (const MIDIError& e) {
        logError("MIDI-Initialisierungsfehler: " + std::string(e.what()));
        cleanup();
        throw;
    }
}

void MIDIEngine::shutdown() {
    if (!initialized) return;

    stopClock();
    stopRecording();

    for (auto& [id, input] : pImpl->midiInputs) {
        if (input && input->isPortOpen()) {
            input->closePort();
        }
    }

    for (auto& [id, output] : pImpl->midiOutputs) {
        if (output && output->isPortOpen()) {
            output->closePort();
        }
    }

    pImpl->midiInputs.clear();
    pImpl->midiOutputs.clear();
    initialized = false;
}

void MIDIEngine::scanDevices() {
    pImpl->devices.clear();

    // MIDI-Eingabegeräte scannen
    RtMidiIn midiIn;
    unsigned int numInputs = midiIn.getPortCount();
    for (unsigned int i = 0; i < numInputs; ++i) {
        MIDIDevice device;
        device.name = midiIn.getPortName(i);
        device.id = i;
        device.isInput = true;
        device.isOutput = false;
        device.isOpen = false;
        pImpl->devices.push_back(device);
    }

    // MIDI-Ausgabegeräte scannen
    RtMidiOut midiOut;
    unsigned int numOutputs = midiOut.getPortCount();
    for (unsigned int i = 0; i < numOutputs; ++i) {
        MIDIDevice device;
        device.name = midiOut.getPortName(i);
        device.id = i;
        device.isInput = false;
        device.isOutput = true;
        device.isOpen = false;
        pImpl->devices.push_back(device);
    }
}

std::vector<MIDIEngine::MIDIDevice> MIDIEngine::getAvailableDevices() const {
    return pImpl->devices;
}

bool MIDIEngine::openDevice(int deviceId) {
    auto it = std::find_if(pImpl->devices.begin(), pImpl->devices.end(),
        [deviceId](const MIDIDevice& d) { return d.id == deviceId; });

    if (it == pImpl->devices.end()) return false;

    try {
        if (it->isInput) {
            auto midiIn = std::make_unique<RtMidiIn>();
            midiIn->openPort(deviceId);
            midiIn->setCallback([this](double timeStamp, std::vector<unsigned char>* message, void* userData) {
                handleMIDIInput(timeStamp, message);
            });
            pImpl->midiInputs[deviceId] = std::move(midiIn);
        } else {
            auto midiOut = std::make_unique<RtMidiOut>();
            midiOut->openPort(deviceId);
            pImpl->midiOutputs[deviceId] = std::move(midiOut);
        }

        it->isOpen = true;
        return true;
    } catch (const std::exception& e) {
        // TODO: Fehlerbehandlung
        return false;
    }
}

void MIDIEngine::closeDevice(int deviceId) {
    if (pImpl->midiInputs.count(deviceId) > 0) {
        pImpl->midiInputs[deviceId]->closePort();
        pImpl->midiInputs.erase(deviceId);
    }

    if (pImpl->midiOutputs.count(deviceId) > 0) {
        pImpl->midiOutputs[deviceId]->closePort();
        pImpl->midiOutputs.erase(deviceId);
    }

    auto it = std::find_if(pImpl->devices.begin(), pImpl->devices.end(),
        [deviceId](const MIDIDevice& d) { return d.id == deviceId; });

    if (it != pImpl->devices.end()) {
        it->isOpen = false;
    }
}

bool MIDIEngine::isDeviceOpen(int deviceId) const {
    auto it = std::find_if(pImpl->devices.begin(), pImpl->devices.end(),
        [deviceId](const MIDIDevice& d) { return d.id == deviceId; });

    return it != pImpl->devices.end() && it->isOpen;
}

void MIDIEngine::sendMessage(const MIDIMessage& message) {
    std::vector<unsigned char> midiMessage;
    midiMessage.push_back(static_cast<unsigned char>(message.type) | message.channel);
    midiMessage.push_back(message.data1);
    midiMessage.push_back(message.data2);

    for (auto& [id, output] : pImpl->midiOutputs) {
        if (output && output->isPortOpen()) {
            output->sendMessage(&midiMessage);
        }
    }

    if (pImpl->isRecordingActive) {
        recordMessage(message);
    }
}

void MIDIEngine::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::NoteOn;
    message.channel = channel;
    message.data1 = note;
    message.data2 = velocity;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::NoteOff;
    message.channel = channel;
    message.data1 = note;
    message.data2 = velocity;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::sendControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::ControlChange;
    message.channel = channel;
    message.data1 = controller;
    message.data2 = value;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::sendProgramChange(uint8_t channel, uint8_t program) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::ProgramChange;
    message.channel = channel;
    message.data1 = program;
    message.data2 = 0;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::sendPitchBend(uint8_t channel, uint16_t value) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::PitchBend;
    message.channel = channel;
    message.data1 = value & 0x7F;
    message.data2 = (value >> 7) & 0x7F;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::sendAftertouch(uint8_t channel, uint8_t value) {
    MIDIMessage message;
    message.type = MIDIMessage::Type::Aftertouch;
    message.channel = channel;
    message.data1 = value;
    message.data2 = 0;
    message.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    sendMessage(message);
}

void MIDIEngine::setMessageCallback(std::function<void(const MIDIMessage&)> callback) {
    pImpl->messageCallback = callback;
}

void MIDIEngine::processMessages() {
    std::lock_guard<std::mutex> lock(pImpl->queueMutex);
    while (!pImpl->messageQueue.empty()) {
        MIDIMessage message = pImpl->messageQueue.front();
        pImpl->messageQueue.pop();

        if (pImpl->messageCallback) {
            pImpl->messageCallback(message);
        }

        if (pImpl->isRecordingActive) {
            recordMessage(message);
        }
    }
}

std::vector<MIDIEngine::MIDIMessage> MIDIEngine::getPendingMessages() {
    std::lock_guard<std::mutex> lock(pImpl->queueMutex);
    std::vector<MIDIMessage> messages;
    while (!pImpl->messageQueue.empty()) {
        messages.push_back(pImpl->messageQueue.front());
        pImpl->messageQueue.pop();
    }
    return messages;
}

void MIDIEngine::startClock() {
    if (pImpl->isClockRunning) return;

    pImpl->isClockRunning = true;
    pImpl->clockThread = std::thread([this]() {
        while (pImpl->isClockRunning) {
            if (pImpl->clockCallback) {
                pImpl->clockCallback(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<int>(60000.0 / (tempo * 24))));
        }
    });
}

void MIDIEngine::stopClock() {
    pImpl->isClockRunning = false;
    if (pImpl->clockThread.joinable()) {
        pImpl->clockThread.join();
    }
}

void MIDIEngine::setTempo(double bpm) {
    tempo = std::max(20.0, std::min(300.0, bpm));
}

double MIDIEngine::getTempo() const {
    return tempo;
}

void MIDIEngine::setClockCallback(std::function<void(double)> callback) {
    pImpl->clockCallback = callback;
}

void MIDIEngine::startRecording() {
    pImpl->isRecordingActive = true;
    pImpl->recordedMessages.clear();
}

void MIDIEngine::stopRecording() {
    pImpl->isRecordingActive = false;
}

void MIDIEngine::pauseRecording() {
    pImpl->isRecordingActive = false;
}

bool MIDIEngine::isRecording() const {
    return pImpl->isRecordingActive;
}

std::vector<MIDIEngine::MIDIMessage> MIDIEngine::getRecordedMessages() const {
    return pImpl->recordedMessages;
}

void MIDIEngine::clearRecording() {
    pImpl->recordedMessages.clear();
}

void MIDIEngine::handleMIDIInput(double timeStamp, std::vector<unsigned char>* message) {
    if (!message || message->empty()) return;

    MIDIMessage midiMessage;
    midiMessage.timestamp = timeStamp;

    uint8_t status = message->at(0);
    midiMessage.channel = status & 0x0F;
    midiMessage.type = static_cast<MIDIMessage::Type>(status & 0xF0);

    if (message->size() > 1) midiMessage.data1 = message->at(1);
    if (message->size() > 2) midiMessage.data2 = message->at(2);

    std::lock_guard<std::mutex> lock(pImpl->queueMutex);
    pImpl->messageQueue.push(midiMessage);
}

void MIDIEngine::recordMessage(const MIDIMessage& message) {
    pImpl->recordedMessages.push_back(message);
}

void MIDIEngine::processMIDIEvent(const MIDIMessage& message) {
    try {
        if (!initialized) {
            throw MIDIError("MIDI-Engine ist nicht initialisiert");
        }
        
        // MIDI-Nachricht validieren
        if (!validateMIDIMessage(message)) {
            throw MIDIError("Ungültige MIDI-Nachricht");
        }
        
        // MIDI-Nachricht verarbeiten
        switch (message.type) {
            case MIDIMessage::Type::NoteOn:
                handleNoteOn(message);
                break;
            case MIDIMessage::Type::NoteOff:
                handleNoteOff(message);
                break;
            case MIDIMessage::Type::ControlChange:
                handleControlChange(message);
                break;
            case MIDIMessage::Type::PitchBend:
                handlePitchBend(message);
                break;
            default:
                throw MIDIError("Nicht unterstützter MIDI-Nachrichtentyp");
        }
    } catch (const MIDIError& e) {
        logError("MIDI-Verarbeitungsfehler: " + std::string(e.what()));
        handleMIDIError(e);
    }
}

void MIDIEngine::handleMIDIError(const MIDIError& error) {
    // Fehler protokollieren
    logError("MIDI-Fehler: " + std::string(error.what()));
    
    // Fehlerbehandlung basierend auf Fehlertyp
    if (error.getType() == MIDIError::Type::DeviceError) {
        // Gerät neu initialisieren
        reinitializeDevice(error.getDeviceId());
    } else if (error.getType() == MIDIError::Type::MessageError) {
        // Ungültige Nachricht ignorieren
        return;
    } else if (error.getType() == MIDIError::Type::BufferError) {
        // Buffer leeren und neu initialisieren
        clearBuffer();
        initializeBuffer();
    }
    
    // Fehler an Error-Handler weiterleiten
    if (errorHandler) {
        errorHandler(error);
    }
}

bool MIDIEngine::validateMIDIMessage(const MIDIMessage& message) {
    // MIDI-Nachricht validieren
    if (message.channel < 0 || message.channel > 15) {
        return false;
    }
    
    switch (message.type) {
        case MIDIMessage::Type::NoteOn:
        case MIDIMessage::Type::NoteOff:
            return message.data1 >= 0 && message.data1 <= 127 &&
                   message.data2 >= 0 && message.data2 <= 127;
        case MIDIMessage::Type::ControlChange:
            return message.data1 >= 0 && message.data1 <= 127 &&
                   message.data2 >= 0 && message.data2 <= 127;
        case MIDIMessage::Type::PitchBend:
            return message.data1 >= 0 && message.data1 <= 127 &&
                   message.data2 >= 0 && message.data2 <= 127;
        default:
            return false;
    }
}

void MIDIEngine::reinitializeDevice(int deviceId) {
    try {
        // Gerät schließen
        closeDevice(deviceId);
        
        // Kurze Pause
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Gerät neu öffnen
        if (!openDevice(deviceId)) {
            throw MIDIError("Fehler beim erneuten Öffnen des MIDI-Geräts");
        }
        
        // Gerät neu initialisieren
        if (!initializeDevice(getDeviceInfo(deviceId))) {
            throw MIDIError("Fehler beim erneuten Initialisieren des MIDI-Geräts");
        }
    } catch (const MIDIError& e) {
        logError("Fehler beim erneuten Initialisieren des MIDI-Geräts: " + std::string(e.what()));
        throw;
    }
}

} // namespace VR_DAW 