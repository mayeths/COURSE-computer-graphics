cmake_minimum_required(VERSION 3.10)

# Define target CPU architecture.
# Usage: cmake -DARCH=<ARCH>
SET(ARCH "universal" CACHE STRING "Target CPU architecture.")
if (ARCH STREQUAL "universal")
  message("Target CPU architecture: x86_64;arm64 (universal)")
  SET(CMAKE_OSX_ARCHITECTURES x86_64;arm64)
elseif (ARCH STREQUAL "x86_64")
  message("Target CPU architecture: x86_64")
  SET(CMAKE_OSX_ARCHITECTURES x86_64)
elseif (ARCH STREQUAL "arm64")
  message("Target CPU architecture: arm64")
  SET(CMAKE_OSX_ARCHITECTURES arm64)
else()
  message(FATAL_ERROR "Unsupported CPU architecture!")
endif()


project(opengl-mac-boilerplate)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_NAME ${PROJECT_NAME}-${ARCH})

# Mac OS libraries
find_library(COCOA_LIBRARY Cocoa)
find_library(OpenGL_LIBRARY OpenGL)
find_library(IO_KIT_LIBRARY IOKit)

# Source and external package files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
aux_source_directory(${SRC_DIR} SOURCES)
set(GLFW_DIR "${EXT_DIR}/glfw")
set(GLAD_DIR "${EXT_DIR}/glad")
set(GLM_DIR "${EXT_DIR}/glm")
set(SOIL_DIR "${EXT_DIR}/soil")
set(FREETYPE_DIR "${EXT_DIR}/freetype")
link_directories(${GLFW_DIR}/lib-${ARCH} ${SOIL_DIR}/lib-${ARCH} ${FREETYPE_DIR}/lib-${ARCH})

# Add the executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 17)

# Mac OS Libraries
target_link_libraries(${EXECUTABLE_NAME} ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IO_KIT_LIBRARY})

# GLFW
target_link_libraries(${EXECUTABLE_NAME} glfw3)
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# GLAD
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${EXECUTABLE_NAME} "glad" "${CMAKE_DL_LIBS}")

# GLM
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${GLM_DIR}")

# SOIL
target_link_libraries(${EXECUTABLE_NAME} SOIL)
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${SOIL_DIR}/include")

# Freetype
target_link_libraries(${EXECUTABLE_NAME} freetype)
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${FREETYPE_DIR}/include")
