#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <array>
#include <stdexcept>

#pragma warning(push)
#pragma warning(disable:4201)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(pop)

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
