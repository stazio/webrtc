# Setup Cmake
cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME webrtc-server)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_TYPE Debug)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Create Project & Settings
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
#set( CMAKE_VERBOSE_MAKEFILE on )

# Create the Executable
add_executable(${PROJECT_NAME} version.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DPROJECT_NAME="${PROJECT_NAME}")
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE .)

# Compile Options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
           ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

# ++ ENTER YOUR SOURCE FILES HERE!
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)

# Include librtcsdp
add_subdirectory(${LIBS_DIR}/libdatachannel)
target_link_libraries(${PROJECT_NAME} datachannel)

# Include seasocks
add_subdirectory(${LIBS_DIR}/seasocks)
target_link_libraries(${PROJECT_NAME} seasocks)

#include json
#add_subdirectory(${LIBS_DIR}/json)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)