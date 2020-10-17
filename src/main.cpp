#include "Application.hpp"
#include "Config.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

constexpr const char *kHelpStr =
    "AdamYuan's GPU Sparse Voxel Octree (Driven by Vulkan)\n"
    "\t-obj [WAVEFRONT OBJ FILENAME]\n"
    "\t-lvl [OCTREE LEVEL (%u <= lvl <= %u)]\n";

int main(int argc, char **argv) {
	spdlog::set_pattern("[%H:%M:%S.%e] [%n] [%^%l%$] [thread %t] %v");

	--argc;
	++argv;
	char **filename = nullptr;
	uint32_t octree_level = 0;
	for (int i = 0; i < argc; ++i) {
		if (i + 1 < argc && strcmp(argv[i], "-obj") == 0)
			filename = argv + i + 1, ++i;
		else if (i + 1 < argc && strcmp(argv[i], "-lvl") == 0)
			octree_level = std::stoi(argv[i + 1]), ++i;
		else {
			printf(kHelpStr, kOctreeLevelMin, kOctreeLevelMax);
			return EXIT_FAILURE;
		}
	}
	if ((filename != nullptr) !=
	    (kOctreeLevelMin <= octree_level && octree_level <= kOctreeLevelMax)) {
		printf(kHelpStr, kOctreeLevelMin, kOctreeLevelMax);
		return EXIT_FAILURE;
	}

	Application app{};
	if (filename)
		app.LoadScene(*filename, octree_level);
	app.Run();

	return EXIT_SUCCESS;
}
