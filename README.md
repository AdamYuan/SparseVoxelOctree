# Sparse Voxel Octree
A GPU SVO Builder using rasterization pipeline and a efficient SVO ray marcher

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

## Reference
* https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-SparseVoxelization.pdf - Voxelization and SVO building
* https://research.nvidia.com/publication/efficient-sparse-voxel-octrees - SVO ray march

## Screenshots
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/0.png)
![](https://raw.githubusercontent.com/AdamYuan/SparseVoxelOctree/master/screenshots/1.png)
