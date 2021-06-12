cmake_minimum_required(VERSION 3.16)

# Interfaces
add_subdirectory(glfw)
add_subdirectory(glm)
#add_subdirectory(stb)
add_subdirectory(tinygltf)
#add_subdirectory(tinyobjloader)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(VulkanSDK)

# Compiled libs
add_subdirectory(imgui)
set_target_properties(ImGui PROPERTIES FOLDER "Extern")
