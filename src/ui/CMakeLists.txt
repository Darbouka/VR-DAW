add_library(ui STATIC
    VRUI.cpp
    Button.cpp
    Slider.cpp
    Knob.cpp
    Display.cpp
    Panel.cpp
    Menu.cpp
    List.cpp
    Grid.cpp
    Theme.cpp
    Animation.cpp
    Interaction.cpp
    Layout.cpp
    Rendering.cpp
)

target_include_directories(ui
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ui
    PUBLIC
    core
    vr
    juce::juce_gui_basics
    juce::juce_gui_extra
    PRIVATE
    juce::juce_graphics
    glm::glm
)

if(APPLE)
    target_link_libraries(ui
        PRIVATE
        ${COREAUDIO}
    )
endif() 