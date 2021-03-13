#include "glTFNode.h"

#include "glTFMesh.h"

namespace Render
{

	glTFNode::glTFNode()
	{
	}

	glTFNode::~glTFNode()
	{
		for (glTFNode* child : myChildren)
			delete child;

		myUniformBuffer.Destroy();

		if (myMesh)
			delete myMesh;
	}

	void glTFNode::Load(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Node& gltfNode = aModel.nodes[aNodeIndex];
		myIndex = aNodeIndex;
		myName = gltfNode.name;

		if (gltfNode.translation.size() == 3)
			myTranslation = glm::make_vec3(gltfNode.translation.data());
		if (gltfNode.rotation.size() == 4)
			myRotation = glm::make_quat(gltfNode.rotation.data());
		if (gltfNode.scale.size() == 3)
			myScale = glm::make_vec3(gltfNode.scale.data());
		if (gltfNode.matrix.size() == 16)
			myMatrix = glm::make_mat4x4(gltfNode.matrix.data());
		myMatrix = glm::scale(myMatrix, glm::vec3(aScale));

		// Load children
		for (size_t i = 0; i < gltfNode.children.size(); ++i)
		{
			glTFNode* childNode = new glTFNode();
			childNode->myParent = this;
			childNode->Load(aModel, gltfNode.children[i], aScale, someOutVertices, someOutIndices);
			myChildren.push_back(childNode);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
		{
			const tinygltf::Mesh& gltfMesh = aModel.meshes[gltfNode.mesh];
			glTFMesh* mesh = new glTFMesh();
			mesh->Load(aModel, gltfMesh, someOutVertices, someOutIndices);
			myMesh = mesh;
		}

		myUniformBuffer.Create(
			sizeof(VulkanPSOContainer::PerObjectUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUniformBuffer.SetupDescriptor();
		myUniformBuffer.Map();
	}

	void glTFNode::Update()
	{
		glm::mat4 matrix = GetMatrix();
		memcpy(myUniformBuffer.myMappedData, &matrix, sizeof(glm::mat4));

		for (glTFNode* child : myChildren)
			child->Update();
	}

	glm::mat4 glTFNode::GetLocalMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(glm::mat4(1.0f), myScale) * myMatrix;
	}

	glm::mat4 glTFNode::GetMatrix() const
	{
		glm::mat4 matrix = GetLocalMatrix();
		glTFNode* node = myParent;
		while (node)
		{
			matrix = GetLocalMatrix() * matrix;
			node = node->myParent;
		}
		return matrix;
	}
}
