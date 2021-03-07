#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string>

namespace Render
{
namespace VulkanglTF
{
	struct Primitive
	{
		uint32_t myFirstVertex;
		uint32_t myVertexCount;
		uint32_t myFirstIndex;
		uint32_t myIndexCount;

		glm::vec3 myMinPos = glm::vec3(FLT_MAX);
		glm::vec3 myMaxPos = glm::vec3(-FLT_MAX);
	};

	struct Mesh
	{
		Mesh();
		~Mesh();

		std::vector<Primitive> myPrimitives;

		std::string myName;

		VulkanBuffer myUniformBuffer;
		VkDescriptorSet myDescriptorSet;
		struct UniformBlock {
			glm::mat4 myMatrix;
		} myUniformData;
	};
}
}
