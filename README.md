# C++ OpenGL Engine
![banner](https://private-user-images.githubusercontent.com/50342436/307546816-aa99a9af-2613-4a7a-b8f1-c1901056d486.gif?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MDg4MDUzNzEsIm5iZiI6MTcwODgwNTA3MSwicGF0aCI6Ii81MDM0MjQzNi8zMDc1NDY4MTYtYWE5OWE5YWYtMjYxMy00YTdhLWI4ZjEtYzE5MDEwNTZkNDg2LmdpZj9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNDAyMjQlMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjQwMjI0VDIwMDQzMVomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTZmZmUwZWVlZjU4ZjgwOThhM2RjYjAwOTZlY2NjYmNhNzQ3YTY2MzI1NDVhNjIwNmMwMDFhZGFjNWY4MjljZWEmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0JmFjdG9yX2lkPTAma2V5X2lkPTAmcmVwb19pZD0wIn0.YOk9cXg__JybG2b6bdegm5N53WqidiXMJVcJvuTV2q0)
## State of the Project
  **Heavily W.I.P**\
  Check the [dev diary](https://github.com/mansen420/OpenGL-Renderer/blob/dev/dev_diary.md) for updates on progress!
  
  #### Features 
  - [x]  Runtime shader loading
  - [x]  Offscreen rendering
  - [x]  Viewing depth buffer
  - [x]  Variable rendering dimension with selectable behaviour
  - [x]  Basic GUI
  - [ ]  Shadow mapping
  - [ ]  A lot more
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



You have probably run into an error at the final step, or any of the steps before it. 


This project is **heavily a work in progress** and so far is only maintained for Linux. \
GCC compiles the project with no issue. The Visual Studio compiler is known to throw many errors.
We have not tested with MinGW, but you can try compiling with it if you are on Windows. \
Whether or not it will compile for macOS, I do not know.


In any case, this project is still in very early development, and we will consider extending support for more systems once the project reaches a mature state. 
