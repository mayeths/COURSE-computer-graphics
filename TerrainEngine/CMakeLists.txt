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


project(TerrianEngine)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_NAME terrain)

# Mac OS libraries
find_library(COCOA_LIBRARY Cocoa)
find_library(OpenGL_LIBRARY OpenGL)
find_library(IO_KIT_LIBRARY IOKit)

# Source and external package files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(EXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external")
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
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${INC_DIR}")
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 11)

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

# imgui
# set(IMGUI_DIR "${EXT_DIR}/imgui")
# file(GLOB IMGUI_SRC "${IMGUI_DIR}/*.h" "${IMGUI_DIR}/*.cpp")
# add_library("imgui" ${IMGUI_SRC})
# set_property(TARGET "imgui" PROPERTY CXX_STANDARD 17)
# target_include_directories("imgui" PRIVATE "${IMGUI_DIR}")
# target_include_directories("imgui" PRIVATE "${IMGUI_DIR}/backends")
# target_include_directories("imgui" PRIVATE "${GLFW_DIR}/include")
# target_include_directories("imgui" PRIVATE "${GLAD_DIR}/include")

# target_include_directories(${EXECUTABLE_NAME} PRIVATE "${EXT_DIR}")
# target_include_directories(${EXECUTABLE_NAME} PRIVATE "${IMGUI_DIR}")
# target_include_directories(${EXECUTABLE_NAME} PRIVATE "${IMGUI_DIR}/backends")
# target_link_libraries(${EXECUTABLE_NAME} "imgui" "${CMAKE_DL_LIBS}")

set(IMGUI_DIR "${EXT_DIR}/imgui")
add_library("imgui" STATIC)

set_property(TARGET "imgui" PROPERTY CXX_STANDARD 17)
target_sources( "imgui"
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                )

target_include_directories("imgui" PRIVATE "${GLFW_DIR}/include")
target_include_directories("imgui" PRIVATE "${GLAD_DIR}/include")

target_include_directories( "imgui"
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            PUBLIC ${IMGUI_DIR}/backends
                            )

find_package(OpenGL REQUIRED)
target_link_libraries("imgui" PUBLIC ${OPENGL_LIBRARIES})
# target_link_libraries("imgui" PUBLIC SDL2main SDL2 ${CMAKE_DL_LIBS})

target_link_libraries(${EXECUTABLE_NAME} "imgui")

# resource
file(COPY ${RES_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
