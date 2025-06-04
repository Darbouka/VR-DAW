#include <gtest/gtest.h>
#include <chrono>
#include "../src/VRDAW.hpp"

namespace VR_DAW {
namespace Tests {

class VRDAWUITest : public ::testing::Test {
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

// UI-Initialisierung Test
TEST_F(VRDAWUITest, UIInitialization) {
    auto uiConfig = daw->createUIConfig();
    uiConfig.theme = "Dark";
    uiConfig.scale = 1.0f;
    uiConfig.language = "de";
    
    auto initResult = daw->initializeUI(uiConfig);
    EXPECT_TRUE(initResult.success);
    
    auto uiInfo = daw->getUIInfo();
    EXPECT_EQ(uiInfo.theme, "Dark");
    EXPECT_EQ(uiInfo.scale, 1.0f);
    EXPECT_EQ(uiInfo.language, "de");
}

// Fenster-Management Test
TEST_F(VRDAWUITest, WindowManagement) {
    daw->initializeUI(daw->createUIConfig());
    
    // Erstelle Fenster
    auto windowResult = daw->createUIWindow("Test Window", 800, 600);
    EXPECT_TRUE(windowResult.success);
    
    // Überprüfe Fenster
    auto windowInfo = daw->getWindowInfo(windowResult.windowId);
    EXPECT_EQ(windowInfo.title, "Test Window");
    EXPECT_EQ(windowInfo.width, 800);
    EXPECT_EQ(windowInfo.height, 600);
    
    // Ändere Fenster
    daw->resizeWindow(windowResult.windowId, 1024, 768);
    windowInfo = daw->getWindowInfo(windowResult.windowId);
    EXPECT_EQ(windowInfo.width, 1024);
    EXPECT_EQ(windowInfo.height, 768);
}

// UI-Komponenten Test
TEST_F(VRDAWUITest, UIComponents) {
    daw->initializeUI(daw->createUIConfig());
    auto windowId = daw->createUIWindow("Component Test", 800, 600).windowId;
    
    // Erstelle Komponenten
    auto buttonId = daw->createButton(windowId, "Test Button", 100, 100);
    auto sliderId = daw->createSlider(windowId, "Test Slider", 100, 200);
    auto textId = daw->createText(windowId, "Test Text", 100, 300);
    
    // Überprüfe Komponenten
    EXPECT_TRUE(daw->isComponentValid(buttonId));
    EXPECT_TRUE(daw->isComponentValid(sliderId));
    EXPECT_TRUE(daw->isComponentValid(textId));
    
    // Teste Interaktionen
    daw->simulateClick(buttonId);
    daw->simulateDrag(sliderId, 0.5f);
    
    // Überprüfe Zustände
    EXPECT_TRUE(daw->getButtonState(buttonId));
    EXPECT_EQ(daw->getSliderValue(sliderId), 0.5f);
}

// UI-Performance Test
TEST_F(VRDAWUITest, UIPerformance) {
    const int DURATION_SECONDS = 30;
    
    daw->initializeUI(daw->createUIConfig());
    auto windowId = daw->createUIWindow("Performance Test", 800, 600).windowId;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(DURATION_SECONDS);
    
    int frameCount = 0;
    int droppedFrames = 0;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        // Rendere Frame
        auto renderResult = daw->renderUIFrame(windowId);
        frameCount++;
        
        if (!renderResult.success) {
            droppedFrames++;
        }
        
        // Überprüfe Performance
        auto metrics = daw->getUIMetrics();
        EXPECT_LT(metrics.cpuUsage, 30.0f); // Maximal 30% CPU
        EXPECT_LT(metrics.memoryUsage, 512); // Maximal 512MB RAM
        EXPECT_LT(metrics.frameTime, 16.67f); // Maximal 16.67ms pro Frame (60 FPS)
    }
    
    // Überprüfe Gesamtperformance
    float frameRate = (float)frameCount / DURATION_SECONDS;
    EXPECT_GT(frameRate, 55.0f); // Mindestens 55 FPS
    EXPECT_LT((float)droppedFrames / frameCount, 0.01f); // Maximal 1% Dropped Frames
}

// UI-Fehlerbehandlung Test
TEST_F(VRDAWUITest, UIErrorHandling) {
    daw->initializeUI(daw->createUIConfig());
    
    // Teste ungültige Fenster
    auto windowResult = daw->createUIWindow("", -1, -1);
    EXPECT_FALSE(windowResult.success);
    
    // Teste ungültige Komponenten
    auto buttonResult = daw->createButton(0, "", -1, -1);
    EXPECT_FALSE(buttonResult.success);
    
    // Teste ungültige Aktionen
    auto actionResult = daw->performUIAction(0, "invalid_action");
    EXPECT_FALSE(actionResult.success);
}

// UI-Integration Test
TEST_F(VRDAWUITest, UIIntegration) {
    daw->initializeUI(daw->createUIConfig());
    
    // Erstelle Hauptfenster
    auto mainWindowId = daw->createUIWindow("Main Window", 1024, 768).windowId;
    
    // Erstelle Menü
    auto menuId = daw->createMenu(mainWindowId, "Main Menu");
    daw->addMenuItem(menuId, "File");
    daw->addMenuItem(menuId, "Edit");
    daw->addMenuItem(menuId, "View");
    
    // Erstelle Toolbar
    auto toolbarId = daw->createToolbar(mainWindowId);
    daw->addToolbarButton(toolbarId, "New");
    daw->addToolbarButton(toolbarId, "Open");
    daw->addToolbarButton(toolbarId, "Save");
    
    // Überprüfe Integration
    auto uiInfo = daw->getUIInfo();
    EXPECT_EQ(uiInfo.windowCount, 1);
    EXPECT_EQ(uiInfo.menuCount, 1);
    EXPECT_EQ(uiInfo.toolbarCount, 1);
}

// UI-Theming Test
TEST_F(VRDAWUITest, UITheming) {
    daw->initializeUI(daw->createUIConfig());
    auto windowId = daw->createUIWindow("Theme Test", 800, 600).windowId;
    
    // Teste verschiedene Themes
    daw->setTheme("Dark");
    auto darkTheme = daw->getThemeInfo();
    EXPECT_EQ(darkTheme.name, "Dark");
    
    daw->setTheme("Light");
    auto lightTheme = daw->getThemeInfo();
    EXPECT_EQ(lightTheme.name, "Light");
    
    // Teste Theme-Komponenten
    daw->setComponentTheme(windowId, "Custom");
    auto componentTheme = daw->getComponentThemeInfo(windowId);
    EXPECT_EQ(componentTheme.name, "Custom");
}

// UI-Zugänglichkeit Test
TEST_F(VRDAWUITest, UIAccessibility) {
    daw->initializeUI(daw->createUIConfig());
    auto windowId = daw->createUIWindow("Accessibility Test", 800, 600).windowId;
    
    // Aktiviere Zugänglichkeit
    auto accessibilityConfig = daw->createAccessibilityConfig();
    accessibilityConfig.enableScreenReader = true;
    accessibilityConfig.enableHighContrast = true;
    accessibilityConfig.enableKeyboardNavigation = true;
    
    daw->setAccessibilityConfig(accessibilityConfig);
    
    // Überprüfe Zugänglichkeit
    auto accessibilityInfo = daw->getAccessibilityInfo();
    EXPECT_TRUE(accessibilityInfo.screenReaderEnabled);
    EXPECT_TRUE(accessibilityInfo.highContrastEnabled);
    EXPECT_TRUE(accessibilityInfo.keyboardNavigationEnabled);
    
    // Teste Zugänglichkeits-Funktionen
    daw->addAccessibilityLabel(windowId, "Test Window");
    daw->setTabOrder(windowId, 1);
    
    auto componentInfo = daw->getComponentAccessibilityInfo(windowId);
    EXPECT_EQ(componentInfo.label, "Test Window");
    EXPECT_EQ(componentInfo.tabOrder, 1);
}

} // namespace Tests
} // namespace VR_DAW 