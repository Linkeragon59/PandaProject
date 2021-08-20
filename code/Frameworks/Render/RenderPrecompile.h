#pragma once

#include "Assert.h"
#include "glmInclude.h"
#include "Types.h"
#include "Utils.h"

#include "vulkan/vulkan.h"
#define VMA_ASSERT(expr) Verify((expr)) // workaround for Release builds
#include "vk_mem_alloc.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
