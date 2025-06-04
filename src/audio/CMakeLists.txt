add_library(audio STATIC
    AudioEngine.cpp
    DolbyAtmosManager.cpp
    Mixer.cpp
    Effects.cpp
    Recording.cpp
    Playback.cpp
    Synthesis.cpp
    Analysis.cpp
    Processing.cpp
    Spatialization.cpp
    Mastering.cpp
    Streaming.cpp
)

target_include_directories(audio
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(audio
    PUBLIC
    core
    plugins
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    PRIVATE
    juce::juce_data_structures
    glm::glm
)

if(APPLE)
    target_link_libraries(audio
        PRIVATE
        ${AUDIOUNIT}
        ${COREAUDIO}
    )
endif()

if(EXISTS "${DOLBY_ATMOS_SDK_DIR}")
    target_link_libraries(audio
        PRIVATE
        dolby_atmos
    )
endif() 