# Sparse Voxel Octree (Vulkan Version)
A GPU SVO Builder using rasterization pipeline, a efficient SVO ray marcher and a simple SVO path tracer.  
If you want an OpenGL version, check [OpenGL branch](https://github.com/AdamYuan/SparseVoxelOctree/tree/opengl).

## Compilation
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Built With
* [volk](https://github.com/zeux/volk) - Vulkan meta-loader
* [GLFW](http://www.glfw.org/) - Window creation and management
* [GLM](https://glm.g-truc.net/) - Maths calculations
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Image loading
* [TinyOBJLoader](https://github.com/syoyo/tinyobjloader) - Obj loading
* [TinyEXR](https://github.com/syoyo/tinyexr) - EXR file saving
* [meshoptimizer](https://github.com/zeux/meshoptimizer) - Optimize mesh
* [ImGui](https://github.com/ocornut/imgui) - UI rendering
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - Call native file dialog (require Zenity on linux)
* [spdlog](https://github.com/gabime/spdlog) - Logging system

## Performance
The 

## Reference
* https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-SparseVoxelization.pdf - Voxelization and SVO building
* https://research.nvidia.com/publication/efficient-sparse-voxel-octrees - SVO ray march

## Screenshots
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/0.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/1.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/2.png)
