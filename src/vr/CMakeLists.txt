add_library(vr
    VRInput.cpp
    VRInput.hpp
    VRPhysics.cpp
    VRPhysics.hpp
    VRRenderer.cpp
    VRRenderer.hpp
    VRAudio.cpp
    VRAudio.hpp
    VRScene.cpp
    VRScene.hpp
    VRInterface.cpp
    VRInterface.hpp
    VRController.cpp
    VRController.hpp
    VRSystem.cpp
    VRSystem.hpp
    VRUI.cpp
    VRUI.hpp
    VRNetwork.cpp
    VRNetwork.hpp
)

target_include_directories(vr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vr
    # Keine JUCE-Abhängigkeiten mehr
)

if(APPLE)
    target_link_libraries(vr
        PRIVATE
        ${COREAUDIO}
    )
endif() 