add_library(core
    CoreSystem.cpp
    CoreSystem.hpp
    Configuration.cpp
    Configuration.hpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core
    PRIVATE
    ${OPENXR_LIBRARY}
    ${VULKAN_LIBRARY}
    ${EIGEN3_LIBRARIES}
    ${BOOST_LIBRARIES}
) 