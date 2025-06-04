cmake_minimum_required(VERSION 3.20)
project(VR_DAW VERSION 1.0.0 LANGUAGES CXX)

# C++-Standard setzen
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE-Framework einbinden
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.5
)
FetchContent_MakeAvailable(JUCE)

# OpenXR einbinden
find_package(OpenXR REQUIRED)

# GLM einbinden
find_package(glm REQUIRED)

# Dolby Atmos SDK einbinden
if(WIN32)
    set(DOLBY_ATMOS_SDK_DIR "$ENV{ProgramFiles}/Dolby/Atmos/SDK")
elseif(APPLE)
    set(DOLBY_ATMOS_SDK_DIR "/Library/Dolby/Atmos/SDK")
endif()

if(EXISTS "${DOLBY_ATMOS_SDK_DIR}")
    add_subdirectory(${DOLBY_ATMOS_SDK_DIR} ${CMAKE_BINARY_DIR}/dolby_atmos)
endif()

# Unterverzeichnisse einbinden
add_subdirectory(src/core)
add_subdirectory(src/plugins)
add_subdirectory(src/ui)
add_subdirectory(src/vr)
add_subdirectory(src/audio)
add_subdirectory(src/video)
add_subdirectory(src/utils)

# Plattformspezifische Konfiguration
if(APPLE)
    # macOS-spezifische Einstellungen
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version")
    
    # AudioUnit und CoreAudio für macOS
    find_library(AUDIOUNIT AudioUnit REQUIRED)
    find_library(COREAUDIO CoreAudio REQUIRED)
    
    # JUCE-Module für macOS
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_basics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_devices)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_processors)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_utils)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_core)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_data_structures)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_events)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_graphics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_gui_basics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_gui_extra)
    
elseif(WIN32)
    # Windows-spezifische Einstellungen
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    # VST3 SDK für Windows
    set(VST3_SDK_DIR "$ENV{ProgramFiles}/VST3 SDK" CACHE PATH "VST3 SDK directory")
    if(EXISTS "${VST3_SDK_DIR}")
        add_subdirectory(${VST3_SDK_DIR} ${CMAKE_BINARY_DIR}/vst3)
    endif()
    
    # JUCE-Module für Windows
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_basics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_devices)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_processors)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_audio_utils)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_core)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_data_structures)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_events)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_graphics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_gui_basics)
    juce_add_module(${JUCE_SOURCE_DIR}/modules/juce_gui_extra)
endif()

# Hauptziel definieren
add_executable(VR_DAW
    src/main.cpp
)

# Abhängigkeiten verknüpfen
target_link_libraries(VR_DAW
    PRIVATE
    core
    plugins
    ui
    vr
    audio
    video
    utils
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    OpenXR::openxr_loader
    glm::glm
)

if(APPLE)
    target_link_libraries(VR_DAW
        PRIVATE
        ${AUDIOUNIT}
        ${COREAUDIO}
    )
endif()

if(EXISTS "${DOLBY_ATMOS_SDK_DIR}")
    target_link_libraries(VR_DAW
        PRIVATE
        dolby_atmos
    )
endif()

# Installation konfigurieren
install(TARGETS VR_DAW
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Ressourcen installieren
install(DIRECTORY resources/
    DESTINATION share/VR_DAW
    FILES_MATCHING PATTERN "*.json"
    PATTERN "*.xml"
    PATTERN "*.png"
    PATTERN "*.jpg"
    PATTERN "*.wav"
    PATTERN "*.mp3"
)

# Tests aktivieren
enable_testing()
add_subdirectory(tests) 