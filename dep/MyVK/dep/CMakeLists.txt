cmake_minimum_required(VERSION 3.15)
project(MyVK)

set(CMAKE_CXX_STANDARD 17)

add_library(MyVK_Dep_Vulkan STATIC
        vma/vk_mem_alloc.cpp
        volk/volk.c
        )
target_include_directories(MyVK_Dep_Vulkan PUBLIC vma volk vulkan)
target_compile_definitions(MyVK_Dep_Vulkan PUBLIC VK_NO_PROTOTYPES)
target_link_libraries(MyVK_Dep_Vulkan PRIVATE ${CMAKE_DL_LIBS})

add_subdirectory(glfw)
add_library(MyVK_Dep_GLFW INTERFACE)
target_link_libraries(MyVK_Dep_GLFW INTERFACE glfw)

add_library(MyVK_Dep_ImGui STATIC
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_impl_glfw.cpp
        )
target_include_directories(MyVK_Dep_ImGui PUBLIC imgui)
target_link_libraries(MyVK_Dep_ImGui PRIVATE glfw)
