cmake_minimum_required(VERSION 3.10)
project(VR-DAW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Finde JUCE
find_package(JUCE REQUIRED)

# Füge Unterverzeichnisse hinzu
add_subdirectory(src)

# Erstelle das Hauptprojekt
add_executable(VR-DAW main.cpp)

# Verknüpfe mit JUCE
target_link_libraries(VR-DAW PRIVATE juce::juce_audio_basics juce::juce_audio_formats juce::juce_audio_processors)

# Füge Include-Verzeichnisse hinzu
target_include_directories(VR-DAW PRIVATE ${CMAKE_SOURCE_DIR}/src) 