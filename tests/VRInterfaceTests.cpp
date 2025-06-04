#include <gtest/gtest.h>
#include <chrono>
#include "../src/vr/VRInterface.hpp"

namespace VR_DAW {
namespace Tests {

class VRInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        vr = std::make_unique<VRInterface>();
        vr->initialize();
    }
    
    void TearDown() override {
        vr->shutdown();
    }
    
    std::unique_ptr<VRInterface> vr;
};

// Initialisierung Tests
TEST_F(VRInterfaceTest, Initialization) {
    EXPECT_TRUE(vr->isInitialized());
    EXPECT_TRUE(vr->getDeviceInfo().isValid());
}

// Raum-Management Tests
TEST_F(VRInterfaceTest, RoomManagement) {
    // Erstelle Raum
    auto roomId = vr->createRoom("Test Room", glm::vec3(0.0f), glm::vec3(5.0f, 3.0f, 5.0f));
    EXPECT_TRUE(roomId != "");
    
    // Überprüfe Raum-Eigenschaften
    auto room = vr->getRoom(roomId);
    EXPECT_EQ(room.name, "Test Room");
    EXPECT_EQ(room.size, glm::vec3(5.0f, 3.0f, 5.0f));
    
    // Teste Raum-Änderungen
    vr->updateRoom(roomId, "Updated Room", glm::vec3(0.0f), glm::vec3(6.0f, 4.0f, 6.0f));
    room = vr->getRoom(roomId);
    EXPECT_EQ(room.name, "Updated Room");
    EXPECT_EQ(room.size, glm::vec3(6.0f, 4.0f, 6.0f));
    
    // Teste Raum-Löschung
    vr->deleteRoom(roomId);
    EXPECT_FALSE(vr->getRoom(roomId).isValid());
}

// Fenster-Management Tests
TEST_F(VRInterfaceTest, WindowManagement) {
    // Erstelle Fenster
    auto windowId = vr->createWindow("Test Window", glm::vec3(0.0f), glm::vec2(1.0f, 1.0f));
    EXPECT_TRUE(windowId != "");
    
    // Überprüfe Fenster-Eigenschaften
    auto window = vr->getWindow(windowId);
    EXPECT_EQ(window.title, "Test Window");
    EXPECT_EQ(window.size, glm::vec2(1.0f, 1.0f));
    
    // Teste Fenster-Änderungen
    vr->updateWindow(windowId, "Updated Window", glm::vec3(1.0f), glm::vec2(2.0f, 2.0f));
    window = vr->getWindow(windowId);
    EXPECT_EQ(window.title, "Updated Window");
    EXPECT_EQ(window.size, glm::vec2(2.0f, 2.0f));
    
    // Teste Fenster-Löschung
    vr->deleteWindow(windowId);
    EXPECT_FALSE(vr->getWindow(windowId).isValid());
}

// Rendering-Performance Tests
TEST_F(VRInterfaceTest, RenderingPerformance) {
    // Erstelle Test-Szene
    auto roomId = vr->createRoom("Test Room", glm::vec3(0.0f), glm::vec3(5.0f, 3.0f, 5.0f));
    
    for (int i = 0; i < 10; ++i) {
        vr->createWindow("Test Window " + std::to_string(i),
            glm::vec3(i * 0.5f, 0.0f, -2.0f),
            glm::vec2(1.0f, 1.0f));
    }
    
    // Teste Rendering-Performance
    auto start = std::chrono::high_resolution_clock::now();
    
    // Rendere 100 Frames
    for (int i = 0; i < 100; ++i) {
        vr->render();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 100 Frames
}

// Physik-Performance Tests
TEST_F(VRInterfaceTest, PhysicsPerformance) {
    // Erstelle Test-Objekte
    for (int i = 0; i < 100; ++i) {
        vr->createWindow("Test Window " + std::to_string(i),
            glm::vec3(i * 0.1f, 0.0f, -2.0f),
            glm::vec2(0.2f, 0.2f));
    }
    
    // Teste Physik-Performance
    auto start = std::chrono::high_resolution_clock::now();
    
    // Aktualisiere Physik 1000 mal
    for (int i = 0; i < 1000; ++i) {
        vr->updatePhysics(1.0f / 60.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 1000 Physik-Updates
}

// Motion-Tracking Tests
TEST_F(VRInterfaceTest, MotionTracking) {
    // Simuliere Controller-Bewegung
    VRInterface::MotionData motionData;
    motionData.position = glm::vec3(0.0f);
    motionData.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    motionData.velocity = glm::vec3(0.0f);
    motionData.angularVelocity = glm::vec3(0.0f);
    
    // Teste Motion-Tracking
    vr->updateMotionData(motionData);
    
    auto trackedData = vr->getMotionData();
    EXPECT_EQ(trackedData.position, motionData.position);
    EXPECT_EQ(trackedData.rotation, motionData.rotation);
    EXPECT_EQ(trackedData.velocity, motionData.velocity);
    EXPECT_EQ(trackedData.angularVelocity, motionData.angularVelocity);
}

// Kollisionserkennung Tests
TEST_F(VRInterfaceTest, CollisionDetection) {
    // Erstelle Test-Objekte
    auto roomId = vr->createRoom("Test Room", glm::vec3(0.0f), glm::vec3(5.0f, 3.0f, 5.0f));
    
    auto windowId1 = vr->createWindow("Window 1", glm::vec3(0.0f), glm::vec2(1.0f, 1.0f));
    auto windowId2 = vr->createWindow("Window 2", glm::vec3(1.0f), glm::vec2(1.0f, 1.0f));
    
    // Teste Kollisionserkennung
    auto collisions = vr->checkCollisions();
    EXPECT_FALSE(collisions.empty());
}

// Shader-Performance Tests
TEST_F(VRInterfaceTest, ShaderPerformance) {
    // Teste Shader-Performance
    auto start = std::chrono::high_resolution_clock::now();
    
    // Führe 1000 Shader-Operationen aus
    for (int i = 0; i < 1000; ++i) {
        vr->updateShaders();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Überprüfe Performance
    EXPECT_LT(duration.count(), 1000); // Maximal 1 Sekunde für 1000 Shader-Operationen
}

// Fehlerbehandlung Tests
TEST_F(VRInterfaceTest, ErrorHandling) {
    // Teste ungültige Raum-Parameter
    EXPECT_FALSE(vr->createRoom("", glm::vec3(0.0f), glm::vec3(0.0f)).isValid());
    EXPECT_FALSE(vr->createRoom("Test", glm::vec3(0.0f), glm::vec3(-1.0f)).isValid());
    
    // Teste ungültige Fenster-Parameter
    EXPECT_FALSE(vr->createWindow("", glm::vec3(0.0f), glm::vec2(0.0f)).isValid());
    EXPECT_FALSE(vr->createWindow("Test", glm::vec3(0.0f), glm::vec2(-1.0f)).isValid());
    
    // Teste ungültige Motion-Daten
    VRInterface::MotionData invalidData;
    invalidData.position = glm::vec3(std::numeric_limits<float>::infinity());
    EXPECT_FALSE(vr->updateMotionData(invalidData));
}

} // namespace Tests
} // namespace VR_DAW 