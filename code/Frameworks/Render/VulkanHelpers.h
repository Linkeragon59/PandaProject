#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <array>
#include <string>
#include <stdexcept>
#include <assert.h>

#pragma warning(push)
#pragma warning(disable:4201)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(pop)

#include <GLFW/glfw3.h>

#define VK_CHECK_RESULT(X, Msg) if (X != VK_SUCCESS) { throw std::runtime_error(Msg); }

namespace Render
{
	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers);
	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);

	VkShaderModule CreateShaderModule(const std::string& aFilename);

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool = VK_NULL_HANDLE);
	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);
}
