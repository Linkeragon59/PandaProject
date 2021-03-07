#include "VulkanglTFNode.h"

#include "VulkanRenderCore.h"

namespace Render
{
namespace VulkanglTF
{
	void Node::Update()
	{
		if (myMesh)
		{
			glm::mat4 matrix = GetMatrix();
			memcpy(myMesh->myUniformBuffer.myMappedData, &matrix, sizeof(glm::mat4));
		}

		for (auto child : myChildren)
			child->Update();
	}

	glm::mat4 Node::GetLocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(glm::mat4(1.0f), myScale) * myMatrix;
	}

	glm::mat4 Node::GetMatrix()
	{
		glm::mat4 matrix = GetLocalMatrix();
		Node* node = myParent;
		while (node)
		{
			matrix = node->GetLocalMatrix() * matrix;
			node = node->myParent;
		}
		return matrix;
	}
}
}
