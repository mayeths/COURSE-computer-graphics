cmake_minimum_required(VERSION 3.10)

# Define target CPU architecture.
SET(ARCH "universal" CACHE STRING "Target CPU architecture.")
if (ARCH STREQUAL "universal")
  message("Target CPU architecture: x86_64;arm64 (universal)")
#   SET(CMAKE_OSX_ARCHITECTURES arm64)
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

set(EXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external")

project(pyramid)
set(EXE ${PROJECT_NAME})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

### Source and external package files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(GLFW_DIR "${EXT_DIR}/glfw")
set(GLAD_DIR "${EXT_DIR}/glad")
set(GLM_DIR "${EXT_DIR}/glm")
set(SOIL_DIR "${EXT_DIR}/soil")
set(FREETYPE_DIR "${EXT_DIR}/freetype")

set(GLFW_DIR "${EXT_DIR}/glfw")
set(SOIL_DIR "${EXT_DIR}/soil")
set(FREETYPE_DIR "${EXT_DIR}/freetype")
link_directories(${GLFW_DIR}/lib-${ARCH} ${SOIL_DIR}/lib-${ARCH} ${FREETYPE_DIR}/lib-${ARCH})

### Target: executable
aux_source_directory(${SRC_DIR} SOURCES)
add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE "${INC_DIR}")
set_property(TARGET ${EXE} PROPERTY CXX_STANDARD 17)

### External Libraries
# OpenGL
find_library(OPENGL_LIB OpenGL)
target_link_libraries(${EXE} ${OPENGL_LIB})
# cocoa
find_library(COCOA_LIB Cocoa)
target_link_libraries(${EXE} ${COCOA_LIB})
# iokit
find_library(IO_KIT_LIB IOKit)
target_link_libraries(${EXE} ${IO_KIT_LIB})

# Mac OS Libraries
target_link_libraries(${EXE} ${COCOA_LIB} ${OpenGL_LIB} ${IO_KIT_LIB})

# GLFW
target_link_libraries(${EXE} glfw3)
target_include_directories(${EXE} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${EXE} PRIVATE "GLFW_INCLUDE_NONE")

# GLAD
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${EXE} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${EXE} "glad" "${CMAKE_DL_LIBS}")

# GLM
target_include_directories(${EXE} PRIVATE "${GLM_DIR}")

# SOIL
target_link_libraries(${EXE} SOIL)
target_include_directories(${EXE} PRIVATE "${SOIL_DIR}/include")

# Freetype
target_link_libraries(${EXE} freetype)
target_include_directories(${EXE} PRIVATE "${FREETYPE_DIR}/include")

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

target_link_libraries(${EXE} "imgui")

# # glfw
# target_compile_definitions(${EXE} PRIVATE "GLFW_INCLUDE_NONE")
# target_include_directories(${EXE} PRIVATE "${GLFW_DIR}/include")
# find_library(GLFW3_LIB glfw3 PATHS ${GLFW_DIR}/lib-${ARCH} NO_DEFAULT_PATH)
# target_link_libraries(${EXE} ${GLFW3_LIB})
# # glad
# set(GLAD_DIR "${EXT_DIR}/glad")
# add_library("glad" "${GLAD_DIR}/src/glad.c")
# target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
# target_include_directories(${EXE} PRIVATE "${GLAD_DIR}/include")
# target_link_libraries(${EXE} glad)
# # glm
# set(GLM_DIR "${EXT_DIR}/glm")
# target_include_directories(${EXE} PRIVATE "${GLM_DIR}")
# # soil
# target_include_directories(${EXE} PRIVATE "${SOIL_DIR}/include")
# find_library(SOIL_LIB SOIL PATHS ${SOIL_DIR}/lib-${ARCH} NO_DEFAULT_PATH)
# target_link_libraries(${EXE} ${SOIL_LIB})
# # freetype
# target_include_directories(${EXE} PRIVATE "${FREETYPE_DIR}/include")
# find_library(FREETYPE_LIB freetype PATHS ${FREETYPE_DIR}/lib-${ARCH} NO_DEFAULT_PATH)
# target_link_libraries(${EXE} ${FREETYPE_LIB})
# imgui
# set(IMGUI_DIR "${EXT_DIR}/imgui")
# add_library("imgui" STATIC)
# set_property(TARGET "imgui" PROPERTY CXX_STANDARD 17)
# target_sources("imgui"
#     PRIVATE
#     ${IMGUI_DIR}/imgui_demo.cpp
#     ${IMGUI_DIR}/imgui_draw.cpp
#     ${IMGUI_DIR}/imgui_tables.cpp
#     ${IMGUI_DIR}/imgui_widgets.cpp
#     ${IMGUI_DIR}/imgui.cpp
#     PRIVATE
#     ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
#     ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
# )
# target_include_directories("imgui" PRIVATE "${GLFW_DIR}/include")
# target_include_directories("imgui" PRIVATE "${GLAD_DIR}/include")
# target_include_directories( "imgui"
#     PUBLIC ${IMGUI_DIR}
#     PUBLIC ${IMGUI_DIR}/backends
# )
# target_link_libraries("imgui" ${OPENGL_LIB})
# target_link_libraries(${EXE} imgui)
