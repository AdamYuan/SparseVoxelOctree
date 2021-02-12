# Sparse Voxel Octree (Obsoleted OpenGL Version)
A GPU SVO Builder using rasterization pipeline, a efficient SVO ray marcher and a simple SVO path tracer.  

## Compilation
```bash
cmake . -DCMAKE_BUILD_TYPE=Release
make
```

## Built With
* [GL3W](https://github.com/skaslev/gl3w) - For modern OpenGL methods
* [GLFW](http://www.glfw.org/) - Window creation and management
* [GLM](https://glm.g-truc.net/) - Maths calculations
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Image loading
* [TinyOBJLoader](https://github.com/syoyo/tinyobjloader) - Obj loading
* [TinyEXR](https://github.com/syoyo/tinyexr) - EXR file saving
* [ImGui](https://github.com/ocornut/imgui) - UI rendering
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - Call native file dialog (require Zenity on linux)

## Reference
* https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-SparseVoxelization.pdf - Voxelization and SVO building
* https://research.nvidia.com/publication/efficient-sparse-voxel-octrees - SVO ray march

## Screenshots
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/opengl/screenshots/0.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/opengl/screenshots/1.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/opengl/screenshots/2.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/opengl/screenshots/3.png)
Image Denoised with OIDN
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/opengl/screenshots/4.png)
