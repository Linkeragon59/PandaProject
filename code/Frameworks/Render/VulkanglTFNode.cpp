#include "VulkanglTFNode.h"

#include "VulkanRenderCore.h"

namespace Render
{
namespace VulkanglTF
{

	Node::~Node()
	{
		if (myMesh)
			delete myMesh;
	}

	void Node::Load(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Node& gltfNode = aModel.nodes[aNodeIndex];
		myIndex = aNodeIndex;
		myName = gltfNode.name;

		myMatrix = glm::mat4(1.0f);
		if (gltfNode.translation.size() == 3)
			myTranslation = glm::make_vec3(gltfNode.translation.data());
		if (gltfNode.rotation.size() == 4)
			myRotation = glm::make_quat(gltfNode.rotation.data());
		if (gltfNode.scale.size() == 3)
			myScale = glm::make_vec3(gltfNode.scale.data());
		if (gltfNode.matrix.size() == 16)
			myMatrix = glm::make_mat4x4(gltfNode.matrix.data());

		// Load children
		for (size_t i = 0; i < gltfNode.children.size(); ++i)
		{
			Node* childNode = new Node();
			childNode->Load(aModel, gltfNode.children[i], aScale, someOutVertices, someOutIndices);
			childNode->myParent = this;
			myChildren.push_back(childNode);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
		{
			const tinygltf::Mesh& gltfMesh = aModel.meshes[gltfNode.mesh];
			Mesh* mesh = new Mesh();
			mesh->Load(aModel, gltfMesh, someOutVertices, someOutIndices);
			myMesh = mesh;
		}

		// Call one update to init the matrix
		Update();
	}

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
			matrix = GetLocalMatrix() * matrix;
			node = myParent;
		}
		return matrix;
	}
}
}
