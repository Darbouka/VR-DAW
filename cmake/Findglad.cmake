set(GLAD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")
set(GLAD_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glad DEFAULT_MSG GLAD_INCLUDE_DIR GLAD_LIBRARY)

mark_as_advanced(GLAD_INCLUDE_DIR GLAD_LIBRARY) 