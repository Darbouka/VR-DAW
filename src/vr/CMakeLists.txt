add_library(vr STATIC
    VRInterface.cpp
    Controller.cpp
    HandTracking.cpp
    RoomTracking.cpp
    Rendering.cpp
    Interaction.cpp
    SpatialAudio.cpp
    Haptics.cpp
    Gestures.cpp
    Calibration.cpp
    Safety.cpp
)

target_include_directories(vr
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vr
    PUBLIC
    core
    juce::juce_core
    OpenXR::openxr_loader
    PRIVATE
    juce::juce_events
    glm::glm
)

if(APPLE)
    target_link_libraries(vr
        PRIVATE
        ${COREAUDIO}
    )
endif() 