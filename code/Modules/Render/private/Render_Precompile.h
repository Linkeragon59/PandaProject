#pragma once

#include "GameCore_Defines.h"
#include "GameCore_Assert.h"
#include "GameCore_glm.h"
#include "GameCore_Utils.h"

#include "vulkan/vulkan.h"
#define VMA_ASSERT(expr) Verify((expr)) // workaround for Release builds
#include "vk_mem_alloc.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "Render_Module.h"
#include "Render_Helpers.h"
