# Sparse Voxel Octree (Vulkan Version)
[![Linux](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/linux.yml/badge.svg)](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/linux.yml)
[![Windows](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/windows.yml/badge.svg)](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/windows.yml)
[![Mac OS](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/macos.yml/badge.svg)](https://github.com/AdamYuan/SparseVoxelOctree/actions/workflows/macos.yml)  
A GPU SVO Builder using the rasterization pipeline, an efficient SVO ray marcher and a simple SVO path tracer.  
If you want an OpenGL version, check [OpenGL branch](https://github.com/AdamYuan/SparseVoxelOctree/tree/opengl).

## Compilation
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Built With
* [volk](https://github.com/zeux/volk) - Vulkan meta-loader
* [VulkanMemoryAllocator](https://gpuopen.com/vulkan-memory-allocator/) - Vulkan memory allocation
* [GLFW](http://www.glfw.org/) - Window creation and management
* [GLM](https://glm.g-truc.net/) - Maths calculations
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Image loading
* [TinyOBJLoader](https://github.com/syoyo/tinyobjloader) - Obj loading
* [TinyEXR](https://github.com/syoyo/tinyexr) - EXR file saving
* [meshoptimizer](https://github.com/zeux/meshoptimizer) - Optimize mesh
* [ImGui](https://github.com/ocornut/imgui) - UI rendering
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - Call native file dialog (require Zenity on linux)
* [spdlog](https://github.com/gabime/spdlog) - Logging system
* [FontAwesome](https://fontawesome.com/) - Icon font

## Usage
* **Camera**
  * **W A S D** - move around (horizontally)
  * **SPACE** - go up
  * **LSHIFT** - go down
  * **Drag** - change perspective
* **X** - toggle ui display

## Improvements
The new Vulkan version is much faster than the old OpenGL version, given the comparison below:
#### GTX 1660 Ti

| SVO build time | Crytek Sponza (2^10) | San Miguel (2^11) | Living Room (2^12) |
| -------------- | -------------------- | ----------------- | ------------------ |
| Vulkan (new)   | **19 ms**            | **203 ms**        | **108 ms**         |
| OpenGL (old)   | 470 ms               | --                | --                 |


#### Quadro M1200

| SVO build time | Crytek Sponza (2^10) | San Miguel (2^11) | Living Room (2^12) |
| -------------- | -------------------- | ----------------- | ------------------ |
| Vulkan (new)   | **80 ms**            | **356 ms**        | **658 ms**         |
| OpenGL (old)   | 421 ms               | 1799 ms           | 3861 ms            |

In addition, the new Vulkan version has some advanced features such as asynchronous model loading and asynchronous path tracing.

## TODOs
### v1.0
- [x] Allow window resizing
- [x] Test queue ownership transfer
- [x] Environment map
### v2.0
- [ ] Voxel editor ?
- [ ] Gradient-domain path tracing ?
- [ ] Build SVO contours ?

## Reference
* https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-SparseVoxelization.pdf - Voxelization and SVO building
* https://research.nvidia.com/publication/efficient-sparse-voxel-octrees - SVO ray march

## Screenshots
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/0.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/1.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/2.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/3.png)
