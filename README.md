## State of the Project
  **Heavily W.I.P**\
  Visit later
## Dependencies
  This project uses cmake as its build system, so you should have it installed.\
  Additionally, you should have all the [dependencies required for GLFW](https://www.glfw.org/docs/3.3/compile.html#compile_deps).\
  Also, your system should support a relatively modern version of OpenGL. Anything above 3.3 will probably work, although the renderer is currently only tested on version 4.5.
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


You have probably run into an error at the final step, or any of the steps before it. 


This project is **heavily a work in progress** and so far is only maintained for Linux. \
GCC compiles the project with no issue. The Visual Studio compiler is known to throw many errors.
We have not tested with MinGW, but you can try compiling with it if you are on Windows. \
Whether or not it will compile for macOS, I do not know.


In any case, this project is still in very early development, and we will consider extending support for more systems once the project reaches a mature state. 

### Check Out This Cool Bug That I got
The bugs are a big part of what makes me love graphics.

![ezgif-4-d1f9f9570a](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/89135a95-0c12-46be-ae08-1ce1f8c873f2) ![ezgif-4-de017a9ae1](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/29af8ebc-d648-4e9a-9e3d-1bb119e73fc0) 

What's really cool here is that there is no texture informatio at all. This is a one-line fragment shader that's outputting memory garbage.
