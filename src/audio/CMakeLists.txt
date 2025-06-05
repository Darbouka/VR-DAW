add_library(audio
    AudioEngine.cpp
    AudioEngine.hpp
    AudioProcessing.cpp
    AudioProcessing.hpp
)

target_include_directories(audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(audio
    PRIVATE
    ${PORTAUDIO_LIB}
    ${SNDFILE_LIB}
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