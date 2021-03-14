#include "glTFNode.h"

#include "glTFModel.h"
#include "glTFMesh.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

namespace Render
{
namespace glTF
{
	Node::~Node()
	{
		for (Node* child : myChildren)
			delete child;

		myUBO.Destroy();
		myDescriptorSet = VK_NULL_HANDLE;
	}

	void Node::Load(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<VulkanPSO::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Node& gltfNode = aModel.nodes[aNodeIndex];
		myIndex = aNodeIndex;
		myName = gltfNode.name;

		mySkinIndex = gltfNode.skin;

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
		myChildren.resize(gltfNode.children.size());
		for (uint32_t i = 0; i < (uint32_t)gltfNode.children.size(); ++i)
		{
			myChildren[i] = new Node();
			myChildren[i]->myParent = this;
			myChildren[i]->Load(aModel, gltfNode.children[i], aScale, someOutVertices, someOutIndices);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
			myMesh.Load(aModel, gltfNode.mesh, someOutVertices, someOutIndices);

		myUBO.Create(
			sizeof(glTF::VulkanPSO::PerObjectUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO.SetupDescriptor();
		myUBO.Map();
	}

	void Node::SetupDescriptorSet(VkDescriptorPool aDescriptorPool)
	{
		VkDevice device = VulkanRenderer::GetInstance()->GetDevice();

		std::array<VkDescriptorSetLayout, 1> layouts = { glTF::VulkanPSO::ourPerObjectDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = aDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the node descriptor set");

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		// Binding 0 : Vertex shader uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &myUBO.myDescriptor;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		for (Node* child : myChildren)
			child->SetupDescriptorSet(aDescriptorPool);
	}

	void Node::UpdateUBO()
	{
		glm::mat4 matrix = GetMatrix();
		memcpy(myUBO.myMappedData, &matrix, sizeof(glm::mat4));

		for (Node* child : myChildren)
			child->UpdateUBO();
	}

	void Node::UpdateJoints(Model* aContainer)
	{
		if (mySkinIndex > -1)
		{
			const Skin* skin = aContainer->GetSkin(mySkinIndex);

			glm::mat4 inverseTransform = glm::inverse(GetMatrix());

			size_t numJoints = (uint32_t)skin->myJoints.size();
			std::vector<glm::mat4> jointMatrices(numJoints);
			for (size_t i = 0; i < numJoints; i++)
			{
				jointMatrices[i] = skin->myJoints[i]->GetMatrix() * skin->myInverseBindMatrices[i];
				jointMatrices[i] = inverseTransform * jointMatrices[i];
			}

			memcpy(skin->mySSBO.myMappedData, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
		}

		for (Node* child : myChildren)
			child->UpdateJoints(aContainer);
	}

	void Node::Draw(Model* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout)
	{
		if (myMesh.myPrimitives.size() > 0)
		{
			// Bind the object matrix
			vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, 1, 1, &myDescriptorSet, 0, NULL);

			// Bind the skin SSBO
			if (mySkinIndex > -1)
			{
				vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, 2, 1, &aContainer->GetSkin(mySkinIndex)->myDescriptorSet, 0, NULL);
			}

			for (const Primitive& primitive : myMesh.myPrimitives)
			{
				// Bind the material's image sampler
				if (primitive.myMaterial > -1)
				{
					const Material* material = aContainer->GetMaterial(primitive.myMaterial);
					assert(material);

					if (material->myBaseColorTexture > -1)
					{
						const Texture* texture = aContainer->GetTexture(material->myBaseColorTexture);
						assert(texture);
						vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, 3, 1, &aContainer->GetImage(texture->myImageIndex)->myDescriptorSet, 0, NULL);
					}
				}
				
				vkCmdDrawIndexed(aCommandBuffer, primitive.myIndexCount, 1, primitive.myFirstIndex, 0, 0);
			}
		}

		for (Node* child : myChildren)
			child->Draw(aContainer, aCommandBuffer, aPipelineLayout);
	}

	glm::mat4 Node::GetLocalMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(glm::mat4(1.0f), myScale) * myMatrix;
	}

	glm::mat4 Node::GetMatrix() const
	{
		glm::mat4 matrix = GetLocalMatrix();
		Node* parent = myParent;
		while (parent)
		{
			matrix = parent->GetLocalMatrix() * matrix;
			parent = parent->myParent;
		}
		return matrix;
	}
}
}
