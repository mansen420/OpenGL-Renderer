# C++ OpenGL Engine


![ezgif-3-95df610dc4](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/74d5e17e-4f5a-4fe6-af56-eda08ebe13b4)

[Sleeping Hermaphroditus](https://threedscans.com/uncategorized/sleeping-hermaphroditus/)

![ezgif-7-dadbc90cdc](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/32603ec7-2cb1-496f-ae9f-63f101c203db)

[BMW M3 model](https://rigmodels.com/model.php?view=BMW_M3_Car-3d-model__99WX7I7CRTWJU9MKXT2CL412A)
## State of the Project
  **Heavily W.I.P**\
  Check the [dev diary](https://github.com/mansen420/OpenGL-Renderer/blob/dev/dev_diary.md) for updates on progress!
  
  #### Features 
  - [x]  Runtime shader editing
  - [x]  Offscreen rendering
  - [x]  Depth buffer viewing
  - [x]  Resizable rendering dimension, with configurable render to view port transform
  - [x]  Basic camera with input handling and time synchronization
  - [x]  Simple object operations (scaling, displacement)
  - [x]  Multithreaded .obj file loading
  - [x]  GUI that exposes the entire engine state (ongoing)
  - [x]  Shadow mapping
  - [ ]  A lot more

![screenshot-from-2024-03-01-00-](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/fde54939-884a-49d8-a7fb-b1fbf205be6b)
## Dependencies
  This project uses cmake as its build system, so you should have it installed.\
  Additionally, you should have all the [dependencies required for GLFW](https://www.glfw.org/docs/3.3/compile.html#compile_deps).\
  Also, your system should support OpenGL 4.5+.
## Building this project
  Provided that you have cmake installed, building this project is the following process :

  
  First, clone the repository 
  ``` bash

  git --recursive clone https://github.com/mansen420/some-OPGL-stuff

```
  Then, generate the build files with cmake 

  ``` bash
cd path/to/repo
cmake -S . -B build
```
This will create a `` build `` directory inside the repository root directory.\
If you are on Linux or other systems, this will contain a `` Makefile `` for the project. On Windows, this will be Visual Studio project files.
If you do not want to compile and build your project manually, use cmake to do it
``` bash
cmake --build path/to/build
```
Typically you would build the project inside your `` build `` directory. \
This will compile and link with the GLFW library, and generate an executable called `` my_renderer `` inside ``src`` within the build directory.



#

This project is **heavily a work in progress** and so far is only maintained for Linux. \
GCC compiles the project with no issue. The Visual Studio compiler is known to throw many errors.
We have not tested with MinGW, but you can try compiling with it if you are on Windows. \
Whether or not it will compile for macOS, I do not know.


In any case, this project is still in very early development, and we will consider extending support for more systems once the project reaches a mature state. 
