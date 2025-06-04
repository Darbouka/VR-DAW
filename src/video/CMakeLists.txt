add_library(video STATIC
    VideoEngine.cpp
    Timeline.cpp
    Effects.cpp
    Transitions.cpp
    Rendering.cpp
    Export.cpp
    Import.cpp
    Processing.cpp
    Synchronization.cpp
    Visualization.cpp
    Compositing.cpp
    Streaming.cpp
)

target_include_directories(video
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(video
    PUBLIC
    core
    juce::juce_core
    juce::juce_graphics
    PRIVATE
    juce::juce_data_structures
    glm::glm
)

if(APPLE)
    target_link_libraries(video
        PRIVATE
        ${COREAUDIO}
        ${COREMEDIA}
        ${COREFOUNDATION}
    )
endif()

if(WIN32)
    target_link_libraries(video
        PRIVATE
        d3d11
        dxgi
    )
endif() 