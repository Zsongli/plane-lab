![PlaneLab logo](./assets/planelab_logo.svg)
---
A simple C application for graphing geometric shapes using ImGui. Made for a university assignment.

## Preview
![Using the program](./assets/planelab_preview.gif)

## Features
- Interactive graphing of geometric shapes
- Utilizes ImGui for a user-friendly interface
- Built with GLFW and OpenGL for rendering
- Emphasis on cross-platform compatibility

## Running and usage

### Requirements
- A CPU and a GPU
- Operating System: Windows or Linux (MacOS is untested)

### Instructions
1. Download the pre-compiled binaries from the Releases section.
2. Run the executable file.

### User documentation
[Hungarian](./assets/user_documentation_hu.md)

## Building and development

### Requirements
- [Premake5](https://premake.github.io/) for generating build files
- A C compiler
- A C++ compiler (for building dependencies)
- OpenGL development libraries
- [GLFW](https://www.glfw.org/) (included as a submodule)
- [cimgui](https://github.com/cimgui/cimgui) (included as a submodule)
- [cimplot](https://github.com/cimplot/cimplot) (included as a submodule)
- [stb](https://github.com/nothings/stb) (included as a submodule)
- [nativefiledialog](https://github.com/mlabbe/nativefiledialog) (included as a submodule)

### Instructions
1. Clone the repository recursively
2. Navigate to the project directory and run:
   ```sh
   premake5 vs2022
   ```
   or for Linux:
   ```sh
   premake5 gmake2
   ```
3. Open the generated solution or Makefile in your preferred IDE and build the project.

### Developer documentation
[Hungarian](./assets/developer_documentation_hu.md)

## License
This project is licensed under the MIT License. See the LICENSE file for details.
