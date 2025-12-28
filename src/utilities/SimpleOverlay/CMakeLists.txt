cmake_minimum_required(VERSION 3.20)

project(SimpleOverlay)

# Build as shared library (DLL)
add_library(SimpleOverlay SHARED
  SimpleOverlay.cpp
)

target_include_directories(SimpleOverlay
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Export symbols for DLL
if(MSVC)
  target_compile_definitions(SimpleOverlay PRIVATE SIMPLEOVERLAY_EXPORTS)
  target_compile_definitions(SimpleOverlay INTERFACE SIMPLEOVERLAY_IMPORTS)
endif()

# Set output properties
set_target_properties(SimpleOverlay PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Link against required libraries
target_link_libraries(SimpleOverlay
  PRIVATE
    d3d11
    dxgi
)

# Enable C++20
set_property(TARGET SimpleOverlay PROPERTY CXX_STANDARD 20)
set_property(TARGET SimpleOverlay PROPERTY CXX_STANDARD_REQUIRED ON)

# Install targets
install(TARGETS SimpleOverlay
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES SimpleOverlay.h
  DESTINATION include
)
