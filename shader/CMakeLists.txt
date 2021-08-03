cmake_minimum_required(VERSION 3.15)
project(shader)

set(SHADER_SRC

		quad.vert
		sobol.comp

		voxelizer.vert
		voxelizer.frag
		voxelizer.geom

		octree_tag_node.comp
		octree_init_node.comp
		octree_alloc_node.comp
		octree_modify_arg.comp

		octree_tracer.frag
		octree_tracer_beam.frag

		path_tracer.comp
		path_tracer_viewer_gen.frag
		path_tracer_viewer_main.vert
		path_tracer_viewer_main.frag
		)

find_program(GLSLC "glslc")
IF (GLSLC)
	function(add_spirv_shader INPUT_FILE)
		add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/include/spirv/${INPUT_FILE}.u32
				COMMAND glslc ${INPUT_FILE} -o include/spirv/${INPUT_FILE}.u32 -O -Os -mfmt=num
				WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
				DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${INPUT_FILE}
				)
	endfunction()

	foreach (X IN LISTS SHADER_SRC)
		add_spirv_shader(${X})
	endforeach ()
ENDIF ()

set(SHADER_U32)
foreach (X IN LISTS SHADER_SRC)
	list(APPEND SHADER_U32 include/spirv/${X}.u32)
endforeach ()

add_custom_target(
		shader_gen ALL DEPENDS
		${SHADER_U32}
)
add_library(shader INTERFACE)
target_include_directories(shader INTERFACE include)
target_link_libraries(shader INTERFACE ${SHADERS_LIBRARIES})
