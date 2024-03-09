Welcome to the diary of this developer.
I will post interesting bits and pieces of my engine's development process here.

#
#### Memory Garbage
![ezgif-4-d1f9f9570a](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/89135a95-0c12-46be-ae08-1ce1f8c873f2) ![ezgif-4-de017a9ae1](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/29af8ebc-d648-4e9a-9e3d-1bb119e73fc0) 

What's really cool here is that there is no texture information at all. This is a one-line fragment shader that's outputting memory garbage.\
The bugs are a big part of what makes me love graphics.
#
#### Normals Generation
I implemented a surface normal generator and here is a cool bug : 

![ezgif-1-33eb8bea29](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/2d15188e-a6f1-4c7a-b4f2-b64b802be816)

Correct result : 

![Screencast from 17-02-24 17 24 50](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/17e3354c-a452-44a3-b4f9-d627923518c8)

#
### Runtime Shader Editing
Finally!

![shader_editing](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/945dc70f-1918-438c-8d40-bdc0430934aa)

#
### Camera and Object Operations
It's all coming together 

![ezgif-2-c1abe71fce](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/aa99a9af-2613-4a7a-b8f1-c1901056d486)

#
### Shadow Mapping
I have PTSD from messing the direct draw calls in OpenGL, and this time is no different. \
This took me a few hours of debugging, and part of my sanity, to stumble my way into making it work. But it works!
![Screenshot from 2024-03-09 02-08-19](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/980a54cb-03e8-4d2b-abad-2f1e619bef20)

![Screenshot from 2024-03-09 02-21-49](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/54f5a006-7b88-488d-bd90-a2285fefb551)

![Screenshot from 2024-03-09 03-00-27](https://github.com/mansen420/OpenGL-Renderer/assets/50342436/ecc5c314-e27f-410d-bccc-bfd59bd4ceec)
