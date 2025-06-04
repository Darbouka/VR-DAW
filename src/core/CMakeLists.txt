add_library(core STATIC
    AudioBuffer.cpp
    AudioProcessor.cpp
    AudioStream.cpp
    ConfigManager.cpp
    EventManager.cpp
    Logger.cpp
    MemoryManager.cpp
    PluginHost.cpp
    ProjectManager.cpp
    ResourceManager.cpp
    SettingsManager.cpp
    StateManager.cpp
    ThreadManager.cpp
    TimeManager.cpp
    Utility.cpp
)

target_include_directories(core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core
    PUBLIC
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_data_structures
    PRIVATE
    juce::juce_events
)

if(APPLE)
    target_link_libraries(core
        PRIVATE
        ${AUDIOUNIT}
        ${COREAUDIO}
    )
endif() 