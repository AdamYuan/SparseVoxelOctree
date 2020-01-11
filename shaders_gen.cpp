#include <iostream>
#include <fstream>

constexpr const char *kShaderPaths[][2] = {
	{ "shaders/octree_alloc_node.comp", "kOctreeAllocNodeCompStr" },
	{ "shaders/octree_tag_node.comp", "kOctreeTagNodeCompStr" },
	{ "shaders/octree_modify_arg.comp", "kOctreeModifyArgCompStr" },
	{ "shaders/octree_tracer.frag", "kOctreeTracerFragStr" },
	{ "shaders/octree_tracer_beam.frag", "kOctreeTracerBeamFragStr" },
	{ "shaders/pathtracer.frag", "kPathTracerFragStr" },
	{ "shaders/quad.vert", "kQuadVertStr" },
	{ "shaders/voxelizer.frag", "kVoxelizerFragStr" },
	{ "shaders/voxelizer.geom", "kVoxelizerGeomStr" },
	{ "shaders/voxelizer.vert", "kVoxelizerVertStr" }
};

int main(int argc, const char **argv) {
	--argc, ++argv;
	if(argc != 1) return EXIT_FAILURE;

	freopen(*argv, "w", stdout);
	printf("#ifndef SHADER_SRC_HPP\n#define SHADER_SRC_HPP\n");

	for(const char * const(&info)[2] : kShaderPaths) {
		printf("constexpr const char *%s = R\"(", info[1]);
		std::ifstream fin{ info[0] };
		std::string src; std::getline(fin, src, '\0');
		printf("%s", src.c_str());
		printf(")\";\n");
	}

	printf("#endif\n");
	fclose(stdout);

	return EXIT_SUCCESS;
}
