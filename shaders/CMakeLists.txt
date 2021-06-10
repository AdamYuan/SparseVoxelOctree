cmake_minimum_required(VERSION 3.15)
project(shaders)

find_program(GLSLC "glslc")
IF(GLSLC)
	function(add_spirv_shader INPUT_FILE OUTPUT_FILE)
		add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/${OUTPUT_FILE}
			COMMAND glslc ${INPUT_FILE} -o ${OUTPUT_FILE} -O -Os -mfmt=num
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
			DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${INPUT_FILE}
			)
	endfunction()

	add_spirv_shader(quad.vert include/spirv/quad.vert.u32)

	add_spirv_shader(sobol.comp include/spirv/sobol.comp.u32)

	add_spirv_shader(voxelizer.vert include/spirv/voxelizer.vert.u32)
	add_spirv_shader(voxelizer.frag include/spirv/voxelizer.frag.u32)
	add_spirv_shader(voxelizer.geom include/spirv/voxelizer.geom.u32)

	add_spirv_shader(octree_tag_node.comp include/spirv/octree_tag_node.comp.u32)
	add_spirv_shader(octree_init_node.comp include/spirv/octree_init_node.comp.u32)
	add_spirv_shader(octree_alloc_node.comp include/spirv/octree_alloc_node.comp.u32)
	add_spirv_shader(octree_modify_arg.comp include/spirv/octree_modify_arg.comp.u32)

	add_spirv_shader(octree_tracer.frag include/spirv/octree_tracer.frag.u32)
	add_spirv_shader(octree_tracer_beam.frag include/spirv/octree_tracer_beam.frag.u32)

	add_spirv_shader(path_tracer.comp include/spirv/path_tracer.comp.u32)
	add_spirv_shader(path_tracer_viewer_gen.frag include/spirv/path_tracer_viewer_gen.frag.u32)
	add_spirv_shader(path_tracer_viewer_main.vert include/spirv/path_tracer_viewer_main.vert.u32)
	add_spirv_shader(path_tracer_viewer_main.frag include/spirv/path_tracer_viewer_main.frag.u32)
ENDIF()

add_custom_target(
	shaders_gen ALL DEPENDS
	include/spirv/quad.vert.u32
	include/spirv/sobol.comp.u32

	include/spirv/voxelizer.vert.u32
	include/spirv/voxelizer.frag.u32
	include/spirv/voxelizer.geom.u32

	include/spirv/octree_tag_node.comp.u32
	include/spirv/octree_init_node.comp.u32
	include/spirv/octree_alloc_node.comp.u32
	include/spirv/octree_modify_arg.comp.u32

	include/spirv/octree_tracer.frag.u32
	include/spirv/octree_tracer_beam.frag.u32

	include/spirv/path_tracer.comp.u32
	include/spirv/path_tracer_viewer_gen.frag.u32
	include/spirv/path_tracer_viewer_main.vert.u32
	include/spirv/path_tracer_viewer_main.frag.u32
	)
add_library(shaders INTERFACE)
target_include_directories(shaders INTERFACE include)
target_link_libraries(shaders INTERFACE ${SHADERS_LIBRARIES})
