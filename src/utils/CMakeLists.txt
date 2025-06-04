add_library(utils STATIC
    Logger.cpp
    Config.cpp
    FileSystem.cpp
    Threading.cpp
    Memory.cpp
    Time.cpp
    Math.cpp
    String.cpp
    Network.cpp
    Serialization.cpp
    Compression.cpp
    Cryptography.cpp
)

target_include_directories(utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(utils
    PUBLIC
    core
    juce::juce_core
    PRIVATE
    juce::juce_data_structures
    glm::glm
)

if(APPLE)
    target_link_libraries(utils
        PRIVATE
        ${COREFOUNDATION}
    )
endif()

if(WIN32)
    target_link_libraries(utils
        PRIVATE
        ws2_32
        crypt32
    )
endif() 