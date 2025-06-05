add_library(ui
    UISystem.cpp
    UISystem.hpp
    UIComponents.cpp
    UIComponents.hpp
)

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ui
    PRIVATE
    ${SDL2_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${GLFW_LIB}
    ${GLEW_LIB}
) 