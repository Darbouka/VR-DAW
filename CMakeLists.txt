cmake_minimum_required(VERSION 3.15)
project(VR_DAW VERSION 1.0)

# Compiler-Flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionen
option(USE_JACK "Enable JACK Audio support" ON)
option(USE_OPENGL "Use OpenGL for rendering" ON)
option(USE_VULKAN "Use Vulkan for rendering" OFF)
option(USE_OPENVR "Enable OpenVR support" ON)

# GLM finden
find_package(glm REQUIRED)

# OpenGL finden
if(USE_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(GLFW3 REQUIRED)
    find_package(glad REQUIRED)
endif()

# Vulkan finden
if(USE_VULKAN)
    find_package(Vulkan REQUIRED)
endif()

# OpenVR finden
if(USE_OPENVR)
    # OpenVR SDK Pfad
    set(OPENVR_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/openvr" CACHE PATH "Path to OpenVR SDK")
    if(NOT EXISTS "${OPENVR_SDK_PATH}")
        message(FATAL_ERROR "OpenVR SDK not found at ${OPENVR_SDK_PATH}. Please download it from https://github.com/ValveSoftware/openvr/releases")
    endif()
    set(OPENVR_INCLUDE_DIRS "${OPENVR_SDK_PATH}/headers")
    set(OPENVR_LIBRARIES "${OPENVR_SDK_PATH}/lib/macos64/libopenvr_api.dylib")
endif()

# PortAudio finden
find_package(PortAudio REQUIRED)

# JACK optional finden
if(USE_JACK)
    find_package(JACK QUIET)
    add_definitions(-DUSE_JACK)
endif()

# Externe Bibliotheken
include(FetchContent)

# stb_image
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glad)

# FreeType
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Include-Verzeichnisse
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

if(USE_JACK)
    if(JACK_FOUND)
        include_directories(${JACK_INCLUDE_DIRS})
        add_definitions(-DUSE_JACK)
    else()
        message(WARNING "JACK Audio not found. Audio features will be disabled.")
    endif()
endif()

# Quelldateien
set(SOURCES
    src/vr/VRUI.cpp
    src/vr/TextRenderer.cpp
    src/vr/TextureManager.cpp
)

# Header-Dateien
set(HEADERS
    src/vr/VRUI.hpp
    src/vr/TextRenderer.hpp
    src/vr/TextureManager.hpp
)

# Shader-Dateien
set(SHADERS
    src/vr/shaders/text_vertex.glsl
    src/vr/shaders/text_fragment.glsl
)

# Executable erstellen
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

# Include-Verzeichnisse
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLM_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
    ${OPENVR_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
    ${stb_SOURCE_DIR}
    ${glad_SOURCE_DIR}/include
    ${FREETYPE_INCLUDE_DIRS}
)

# Bibliotheken verlinken
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${GLAD_LIBRARIES}
    ${VULKAN_LIBRARIES}
    ${OPENVR_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    glad
    ${FREETYPE_LIBRARIES}
)

if(USE_JACK AND JACK_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JACK_LIBRARIES})
endif()

# Compiler-Flags
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_OPENGL}>:USE_OPENGL>
    $<$<BOOL:${USE_VULKAN}>:USE_VULKAN>
    $<$<BOOL:${USE_OPENVR}>:USE_OPENVR>
)

# Installationsregeln
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Shader-Dateien installieren
install(FILES ${SHADERS} DESTINATION share/${PROJECT_NAME}/shaders) 