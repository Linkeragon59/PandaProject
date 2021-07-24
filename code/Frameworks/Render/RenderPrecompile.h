#pragma once

#include "Assert.h"
#include "Types.h"

#include "vulkan/vulkan.h"
#define VMA_ASSERT(expr) Verify((expr)) // workaround for Release builds
#include "vk_mem_alloc.h"

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
