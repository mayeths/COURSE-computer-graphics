# OpenGL Boilerplate for Mac

**Apple Silicon (e.g. ARM64-based M1, M2) supported!**

This repository provides a boilerplate for OpenGL project with [GLFW](https://www.glfw.org/), [GLAD](https://glad.dav1d.de/), [GLM](https://glm.g-truc.net/), [SOIL](https://www.lonesock.net/soil.html) and [FreeType](https://www.freetype.org/) on Mac OS. It also provides a
`.vscode/c_cpp_properties.json` file for VSCode.


## Structure

This is the structure of the boilerplate.

```
.
├─.vscode
├─external
│  ├─freetype
│  │  ├─include
│  │  ├─lib-arm64
│  │  ├─lib-universal
│  │  └─lib-x86_64
│  ├─glad
│  │  ├─include
│  │  └─src
│  ├─glfw
│  │  ├─include
│  │  ├─lib-arm64
│  │  ├─lib-universal
│  │  └─lib-x86_64
│  ├─glm
│  │  └─glm
│  └─soil
│      ├─include
│      ├─lib-arm64
│      ├─lib-universal
│      └─lib-x86_64
└─src
```

### Libraries

We provide `GLAD`, `GLFW`, `GLM`, `SOIL` and `FreeType` for this project. We have already placed `GLFW` (version 3.3.8), `GLAD` (verison 4.6) and `GLM` (version 0.9.9.8) files under the `external` folder. We have also pre-compiled SOIL (version July 7, 2008) and FreeType (version 2.10.0), and placed their files likewise.

If you want libraries of certain versions, you can still keep the structure of this project, and

- go [here](http://www.glfw.org/download.html) to download the pre-compiled binaries of GLFW
- go [here](https://glad.dav1d.de/) to generate GLAD source of your preferred version
- go [here](https://github.com/g-truc/glm/releases) to download GLM
- go [here](http://www.lonesock.net/soil.html) to download and compile SOIL.
  - Tips: You can try [this project](https://github.com/DeVaukz/SOIL) to compile SOIL.
- go [here](https://www.freetype.org) to download and compile FreeType.

### Sources

You will need to place your own source files under the `src` folder. We have placed a simple file which will create a window.

### Additional Setup

Your program may depend on external resources. You can create a `resources` directory at the root of this project, and append the following lines to the `CMakeLists.txt`:

```cmake
# Copy to bin
add_custom_target(copy-runtime-files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/bin/resources
)
add_dependencies(${EXECUTABLE_NAME} copy-runtime-files)
```

Then you can refer to your resource file as `resources/foo` in your code.


## Developing

We suggest using VSCode with [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension so that you can build or debug simply by just clicking buttons in the status bar after proper configuration.

Of course you can also run:

```bash
# Create the build directory and navigate into it
mkdir build
cd build

# Configure the CMake project
cmake  ..
# Additionally, you can specify the target CPU architecture by passing
# `-DARCH=<ARCH>`. <ARCH> can be `universal` (default), `arm64`, or `x86_64`

# Build the executables
make  # or `cmake --build .`
```

The generated executable will be placed at `build/bin` .


## Reminder

You will always need to run the program from your terminal rather than double clicking. Otherwise, the program will get a wrong working directory.

Please note that Mac OS currently only support `OpenGL<=4.1`.

For developing OpenGL programs on Windows, you can refer to [RainEggplant/opengl-mingw-boilerplate](https://github.com/RainEggplant/opengl-mingw-boilerplate).
