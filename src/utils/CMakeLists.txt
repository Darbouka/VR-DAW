add_library(utils
    Logger.cpp
    Logger.hpp
    ConfigManager.cpp
    ConfigManager.hpp
)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(utils
    PRIVATE
    ${BOOST_LIBRARIES}
    nlohmann_json::nlohmann_json
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