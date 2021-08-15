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

	void Node::Load(const tinygltf::Model& aModel, uint aNodeIndex, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
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

		// Load children
		myChildren.resize(gltfNode.children.size());
		for (uint i = 0; i < (uint)gltfNode.children.size(); ++i)
		{
			myChildren[i] = new Node();
			myChildren[i]->myParent = this;
			myChildren[i]->Load(aModel, gltfNode.children[i], someOutVertices, someOutIndices);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
			myMesh.Load(aModel, gltfNode.mesh, someOutVertices, someOutIndices);

		myUBO.Create(
			sizeof(ShaderHelpers::ModelMatrixData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO.SetupDescriptor();
		myUBO.Map();
	}

	void Node::UpdateUBO(const glm::mat4& aMatrix)
	{
		glm::mat4 matrix = aMatrix * GetMatrix();
		memcpy(myUBO.myMappedData, &matrix, sizeof(glm::mat4));

		for (Node* child : myChildren)
			child->UpdateUBO(aMatrix);
	}

	void Node::UpdateJoints(const Model* aContainer)
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

	void Node::Draw(const Model* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType)
	{
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

			ShaderHelpers::ObjectDescriptorInfo info;
			info.myModelMatrixInfo = &myUBO.myDescriptor;
			info.myImageSamplerInfo = &image->myImage.myDescriptor;
			info.myMaterialInfo = &material->mySSBO.myDescriptor;
			info.myJointMatricesInfo = &skin->mySSBO.myDescriptor;
			VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(aType, info);
			vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);

			vkCmdDrawIndexed(aCommandBuffer, primitive.myIndexCount, 1, primitive.myFirstIndex, 0, 0);
		}

		for (Node* child : myChildren)
			child->Draw(aContainer, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex, aType);
	}

	glm::mat4 Node::GetLocalMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(myScale) * myMatrix;
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
