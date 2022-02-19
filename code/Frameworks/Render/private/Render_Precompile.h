#pragma once

#include "Base_Assert.h"
#include "Base_glm.h"
#include "Base_Types.h"
#include "Base_Utils.h"

#include "vulkan/vulkan.h"
#define VMA_ASSERT(expr) Verify((expr)) // workaround for Release builds
#include "vk_mem_alloc.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "Render_Core.h"
#include "Render_Helpers.h"
