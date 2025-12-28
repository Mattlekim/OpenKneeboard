cmake_minimum_required(VERSION 3.15)

project(SimpleOverlay VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SIMPLEOVERLAY_SOURCES
    SimpleOpenXROverlay.cpp
)

set(SIMPLEOVERLAY_HEADERS
    SimpleOpenXROverlay.h
)

# Create library target
add_library(SimpleOverlay STATIC
    ${SIMPLEOVERLAY_SOURCES}
    ${SIMPLEOVERLAY_HEADERS}
)

# Also create a shared library variant
add_library(SimpleOverlay_shared SHARED
    ${SIMPLEOVERLAY_SOURCES}
    ${SIMPLEOVERLAY_HEADERS}
)

set_target_properties(SimpleOverlay_shared PROPERTIES
    OUTPUT_NAME SimpleOverlay
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Include directories
target_include_directories(SimpleOverlay
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(SimpleOverlay_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find OpenXR
find_package(OpenXR QUIET)

if(OpenXR_FOUND)
    message(STATUS "Found OpenXR: ${OpenXR_INCLUDE_DIRS}")
    target_link_libraries(SimpleOverlay PUBLIC OpenXR::openxr_loader)
    target_link_libraries(SimpleOverlay_shared PUBLIC OpenXR::openxr_loader)
else()
    message(WARNING "OpenXR not found via find_package, attempting manual configuration")
    # Try to find OpenXR manually
    find_path(OPENXR_INCLUDE_DIR openxr/openxr.h)
    find_library(OPENXR_LIBRARY NAMES openxr_loader)
    
    if(OPENXR_INCLUDE_DIR AND OPENXR_LIBRARY)
        message(STATUS "Found OpenXR manually: ${OPENXR_INCLUDE_DIR}")
        target_include_directories(SimpleOverlay PUBLIC ${OPENXR_INCLUDE_DIR})
        target_include_directories(SimpleOverlay_shared PUBLIC ${OPENXR_INCLUDE_DIR})
        target_link_libraries(SimpleOverlay PUBLIC ${OPENXR_LIBRARY})
        target_link_libraries(SimpleOverlay_shared PUBLIC ${OPENXR_LIBRARY})
    else()
        message(WARNING "OpenXR not found. Please install OpenXR SDK or specify paths manually.")
    endif()
endif()

# Windows-specific libraries
if(WIN32)
    target_link_libraries(SimpleOverlay
        PUBLIC
            d3d11.lib
            d3d12.lib
            dxgi.lib
            windowsapp.lib
    )
    
    target_link_libraries(SimpleOverlay_shared
        PUBLIC
            d3d11.lib
            d3d12.lib
            dxgi.lib
            windowsapp.lib
    )
    
    # Enable Windows 10 features
    target_compile_definitions(SimpleOverlay PRIVATE
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
        UNICODE
        _UNICODE
    )
    
    target_compile_definitions(SimpleOverlay_shared PRIVATE
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
        UNICODE
        _UNICODE
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(SimpleOverlay PRIVATE /W4 /WX-)
    target_compile_options(SimpleOverlay_shared PRIVATE /W4 /WX-)
else()
    target_compile_options(SimpleOverlay PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(SimpleOverlay_shared PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation rules
install(TARGETS SimpleOverlay SimpleOverlay_shared
    EXPORT SimpleOverlayTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${SIMPLEOVERLAY_HEADERS}
    DESTINATION include/SimpleOverlay
)

# Export targets
install(EXPORT SimpleOverlayTargets
    FILE SimpleOverlayTargets.cmake
    NAMESPACE SimpleOverlay::
    DESTINATION lib/cmake/SimpleOverlay
)

# Create package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleOverlayConfig.cmake
    INSTALL_DESTINATION lib/cmake/SimpleOverlay
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleOverlayConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleOverlayConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SimpleOverlayConfigVersion.cmake
    DESTINATION lib/cmake/SimpleOverlay
)

# Optional: Build examples if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(SIMPLEOVERLAY_BUILD_EXAMPLES "Build SimpleOverlay examples" OFF)
    
    if(SIMPLEOVERLAY_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "SimpleOverlay Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
if(OpenXR_FOUND OR (OPENXR_INCLUDE_DIR AND OPENXR_LIBRARY))
    message(STATUS "  OpenXR: Found")
else()
    message(STATUS "  OpenXR: Not Found (may need manual configuration)")
endif()
message(STATUS "")
