#include "VulkanglTFNode.h"

#include "VulkanglTFModel.h"
#include "VulkanglTFMesh.h"

#include "VulkanRender.h"
#include "VulkanHelpers.h"

namespace Render::Vulkan::glTF
{
	Node::~Node()
	{
		for (Node* child : myChildren)
			delete child;

		myUBO.Destroy();
	}

	void Node::Load(const tinygltf::Model& aModel, uint aNodeIndex, float aScale, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
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
		for (uint i = 0; i < (uint)gltfNode.children.size(); ++i)
		{
			myChildren[i] = new Node();
			myChildren[i]->myParent = this;
			myChildren[i]->Load(aModel, gltfNode.children[i], aScale, someOutVertices, someOutIndices);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
			myMesh.Load(aModel, gltfNode.mesh, someOutVertices, someOutIndices);

		myUBO.Create(
			sizeof(ShaderHelpers::ModelData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO.SetupDescriptor();
		myUBO.Map();
	}

	void Node::SetupDescriptorSet(Model* aContainer, VkDescriptorPool aDescriptorPool)
	{
		if (myMesh.myPrimitives.size() > 0)
		{
			VkDevice device = RenderCore::GetInstance()->GetDevice();

			std::array<VkDescriptorSetLayout, 1> layouts = { ShaderHelpers::GetObjectDescriptorSetLayout() };

			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = aDescriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = layouts.data();
			descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();

			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

			// Binding 0 : Model
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			// Binding 1 : Joint Matrices
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			// Binding 2 : Texture Sampler
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			// Binding 3 : Material
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

			const Skin* skin = nullptr;
			const Image* image = nullptr;
			const Material* material = nullptr;

			if (mySkinIndex > -1)
			{
				skin = aContainer->GetSkin(mySkinIndex);
			}
			if (!skin)
			{
				skin = aContainer->GetEmptySkin();
			}

			for (Primitive& primitive : myMesh.myPrimitives)
			{
				if (primitive.myMaterial > -1)
				{
					material = aContainer->GetMaterial(primitive.myMaterial);
					Assert(material);

					if (material->myBaseColorTexture > -1)
					{
						const Texture* texture = aContainer->GetTexture(material->myBaseColorTexture);
						Assert(texture);
						image = aContainer->GetImage(texture->myImageIndex);
						Assert(image);
					}
				}
				if (!image)
				{
					image = aContainer->GetEmptyImage();
				}
				if (!material)
				{
					material = aContainer->GetEmptyMaterial();
				}

				VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &primitive.myDescriptorSet), "Failed to create the descriptor set");

				// Binding 0 : Model
				descriptorWrites[0].dstSet = primitive.myDescriptorSet;
				descriptorWrites[0].pBufferInfo = &myUBO.myDescriptor;
				// Binding 1 : Joint Matrices
				descriptorWrites[1].dstSet = primitive.myDescriptorSet;
				descriptorWrites[1].pBufferInfo = &skin->mySSBO.myDescriptor;
				// Binding 2 : Texture Sampler
				descriptorWrites[2].dstSet = primitive.myDescriptorSet;
				descriptorWrites[2].pImageInfo = &image->myImage.myDescriptor;
				// Binding 3 : Material
				descriptorWrites[3].dstSet = primitive.myDescriptorSet;
				descriptorWrites[3].pBufferInfo = &material->mySSBO.myDescriptor;

				vkUpdateDescriptorSets(device, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}

		for (Node* child : myChildren)
			child->SetupDescriptorSet(aContainer, aDescriptorPool);
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

			size_t numJoints = (uint)skin->myJoints.size();
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

	void Node::Draw(Model* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex)
	{
		for (const Primitive& primitive : myMesh.myPrimitives)
		{
			vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &primitive.myDescriptorSet, 0, NULL);

			vkCmdDrawIndexed(aCommandBuffer, primitive.myIndexCount, 1, primitive.myFirstIndex, 0, 0);
		}

		for (Node* child : myChildren)
			child->Draw(aContainer, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex);
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
