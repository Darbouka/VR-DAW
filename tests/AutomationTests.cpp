#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWAutomationTest : public ::testing::Test {
protected:
    void SetUp() override {
        daw = std::make_unique<VRDAW>();
        daw->initialize();
    }
    
    void TearDown() override {
        daw->shutdown();
    }
    
    std::unique_ptr<VRDAW> daw;
};

// Automatisierung-Erstellung Test
TEST_F(VRDAWAutomationTest, AutomationCreation) {
    auto projectId = daw->createProject("Automation Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Erstelle Automatisierung
    auto automationId = daw->createAutomation(trackId, "volume");
    EXPECT_TRUE(daw->isAutomationValid(automationId));
    
    // Überprüfe Automatisierungs-Informationen
    auto automationInfo = daw->getAutomationInfo(automationId);
    EXPECT_EQ(automationInfo.parameter, "volume");
    EXPECT_EQ(automationInfo.pointCount, 0);
    EXPECT_FALSE(automationInfo.isEnabled);
}

// Automatisierungs-Punkte Test
TEST_F(VRDAWAutomationTest, AutomationPoints) {
    auto projectId = daw->createProject("Points Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    auto automationId = daw->createAutomation(trackId, "pan");
    
    // Füge Automatisierungs-Punkte hinzu
    daw->addAutomationPoint(automationId, 0.0f, 0.0f); // Start
    daw->addAutomationPoint(automationId, 0.5f, 1.0f); // Mitte
    daw->addAutomationPoint(automationId, 1.0f, 0.0f); // Ende
    
    // Überprüfe Punkte
    auto points = daw->getAutomationPoints(automationId);
    EXPECT_EQ(points.size(), 3);
    
    // Überprüfe Punkt-Werte
    EXPECT_FLOAT_EQ(points[0].time, 0.0f);
    EXPECT_FLOAT_EQ(points[0].value, 0.0f);
    EXPECT_FLOAT_EQ(points[1].time, 0.5f);
    EXPECT_FLOAT_EQ(points[1].value, 1.0f);
    EXPECT_FLOAT_EQ(points[2].time, 1.0f);
    EXPECT_FLOAT_EQ(points[2].value, 0.0f);
}

// Automatisierungs-Kurven Test
TEST_F(VRDAWAutomationTest, AutomationCurves) {
    auto projectId = daw->createProject("Curves Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    auto automationId = daw->createAutomation(trackId, "filter");
    
    // Füge Punkte mit verschiedenen Kurven hinzu
    daw->addAutomationPoint(automationId, 0.0f, 0.0f, "linear");
    daw->addAutomationPoint(automationId, 0.5f, 1.0f, "exponential");
    daw->addAutomationPoint(automationId, 1.0f, 0.0f, "sine");
    
    // Aktiviere Automatisierung
    daw->setAutomationEnabled(automationId, true);
    
    // Überprüfe Kurven-Interpolation
    auto values = daw->getAutomationValues(automationId, 0.25f);
    EXPECT_GT(values[0], 0.0f);
    EXPECT_LT(values[0], 1.0f);
}

// Automatisierungs-Performance Test
TEST_F(VRDAWAutomationTest, AutomationPerformance) {
    const int DURATION_SECONDS = 30;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int operationCount = 0;
    int failedOperations = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Erstelle Test-Automatisierung
        auto projectId = daw->createProject("Performance Test");
        auto trackId = daw->createTrack(projectId, "Test Track");
        auto automationId = daw->createAutomation(trackId, "volume");
        
        // Füge viele Punkte hinzu
        for (int i = 0; i < 100; ++i) {
            float time = i / 100.0f;
            float value = std::sin(time * 2 * M_PI);
            daw->addAutomationPoint(automationId, time, value);
        }
        
        operationCount++;
        
        if (!daw->isAutomationValid(automationId)) {
            failedOperations++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getAutomationMetrics(automationId);
        EXPECT_LT(metrics.cpuUsage, 30.0f); // Maximal 30% CPU
        EXPECT_LT(metrics.memoryUsage, 100); // Maximal 100MB RAM
        EXPECT_LT(metrics.calculationTime, 1.0f); // Maximal 1ms Berechnungszeit
    }
    
    // Überprüfe Gesamtperformance
    float successRate = 1.0f - (float)failedOperations / operationCount;
    EXPECT_GT(successRate, 0.99f); // Mindestens 99% Erfolgsrate
}

// Automatisierungs-Fehlerbehandlung Test
TEST_F(VRDAWAutomationTest, AutomationErrorHandling) {
    // Teste ungültige Automatisierung
    auto invalidAutomation = daw->createAutomation("invalid_track", "volume");
    EXPECT_FALSE(daw->isAutomationValid(invalidAutomation));
    
    // Teste ungültige Punkte
    auto projectId = daw->createProject("Error Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    auto automationId = daw->createAutomation(trackId, "volume");
    
    daw->addAutomationPoint(automationId, -1.0f, 0.0f); // Ungültige Zeit
    daw->addAutomationPoint(automationId, 0.0f, 2.0f); // Ungültiger Wert
    
    auto points = daw->getAutomationPoints(automationId);
    EXPECT_EQ(points.size(), 0); // Punkte sollten abgelehnt werden
    
    // Teste ungültige Kurven
    daw->addAutomationPoint(automationId, 0.0f, 0.0f, "invalid_curve");
    EXPECT_FALSE(daw->isAutomationValid(automationId));
}

// Automatisierungs-Integration Test
TEST_F(VRDAWAutomationTest, AutomationIntegration) {
    auto projectId = daw->createProject("Integration Test");
    auto trackId = daw->createTrack(projectId, "Test Track");
    
    // Erstelle Automatisierungen für verschiedene Parameter
    auto volumeAutomation = daw->createAutomation(trackId, "volume");
    auto panAutomation = daw->createAutomation(trackId, "pan");
    auto filterAutomation = daw->createAutomation(trackId, "filter");
    
    // Füge Punkte hinzu
    daw->addAutomationPoint(volumeAutomation, 0.0f, 0.0f);
    daw->addAutomationPoint(volumeAutomation, 1.0f, 1.0f);
    
    daw->addAutomationPoint(panAutomation, 0.0f, -1.0f);
    daw->addAutomationPoint(panAutomation, 1.0f, 1.0f);
    
    daw->addAutomationPoint(filterAutomation, 0.0f, 20.0f);
    daw->addAutomationPoint(filterAutomation, 1.0f, 20000.0f);
    
    // Aktiviere Automatisierungen
    daw->setAutomationEnabled(volumeAutomation, true);
    daw->setAutomationEnabled(panAutomation, true);
    daw->setAutomationEnabled(filterAutomation, true);
    
    // Überprüfe Integration
    auto trackInfo = daw->getTrackInfo(trackId);
    EXPECT_EQ(trackInfo.automationCount, 3);
    
    // Überprüfe Parameter-Werte
    auto values = daw->getTrackParameterValues(trackId, 0.5f);
    EXPECT_GT(values.volume, 0.0f);
    EXPECT_LT(values.volume, 1.0f);
    EXPECT_GT(values.pan, -1.0f);
    EXPECT_LT(values.pan, 1.0f);
    EXPECT_GT(values.filter, 20.0f);
    EXPECT_LT(values.filter, 20000.0f);
}

} // namespace Tests
} // namespace VR_DAW 