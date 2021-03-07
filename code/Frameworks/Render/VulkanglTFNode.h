#pragma once

#include "VulkanHelpers.h"
#include "VulkanglTFMesh.h"

#pragma warning(push)
#pragma warning(disable:4201)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(pop)

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string>

namespace Render
{
namespace VulkanglTF
{
	struct Node
	{
		void Update();

		glm::mat4 GetLocalMatrix();
		glm::mat4 GetMatrix();

		Node* myParent = nullptr;
		std::vector<Node*> myChildren;

		uint32_t myIndex = 0;
		std::string myName;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix;

		Mesh* myMesh = nullptr;
	};
}
}
