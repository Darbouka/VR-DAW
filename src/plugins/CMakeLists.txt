add_library(plugins STATIC
    PluginManager.cpp
    VST3Plugin.cpp
    AUPlugin.cpp
    PluginParameter.cpp
    PluginPreset.cpp
    PluginScanner.cpp
    PluginValidator.cpp
)

target_include_directories(plugins
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(plugins
    PUBLIC
    core
    juce::juce_audio_processors
    juce::juce_audio_utils
    PRIVATE
    juce::juce_data_structures
)

if(APPLE)
    target_link_libraries(plugins
        PRIVATE
        ${AUDIOUNIT}
        ${COREAUDIO}
    )
endif()

if(WIN32)
    if(EXISTS "${VST3_SDK_DIR}")
        target_link_libraries(plugins
            PRIVATE
            vst3
        )
    endif()
endif() 