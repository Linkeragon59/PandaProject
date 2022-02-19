cmake_minimum_required(VERSION 3.16)

# Interface Libraries

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(rapidjson)
#add_subdirectory(stb)
add_subdirectory(tinygltf)
#add_subdirectory(tinyobjloader)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(VulkanSDK)

# Dependency Libraries

add_subdirectory(imgui)
set_target_properties(ImGui PROPERTIES FOLDER "Extern")

add_subdirectory(SoLoud)
set_target_properties(SoLoud PROPERTIES FOLDER "Extern")
