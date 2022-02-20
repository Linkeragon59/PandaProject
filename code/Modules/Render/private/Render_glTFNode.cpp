#include "Render_glTFNode.h"

#include "Render_glTFModel.h"
#include "Render_glTFMesh.h"

namespace Render
{
	glTFNode::~glTFNode()
	{
		for (glTFNode* child : myChildren)
			delete child;
	}

	void glTFNode::Load(const tinygltf::Model& aModel, uint aNodeIndex, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
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
			myChildren[i] = new glTFNode();
			myChildren[i]->myParent = this;
			myChildren[i]->Load(aModel, gltfNode.children[i], someOutVertices, someOutIndices);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
			myMesh.Load(aModel, gltfNode.mesh, someOutVertices, someOutIndices);

		myUBO = new VulkanBuffer(
			sizeof(ShaderHelpers::ModelMatrixData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO->SetupDescriptor();
		myUBO->Map();
	}

	void glTFNode::UpdateUBO(const glm::mat4& aMatrix)
	{
		glm::mat4 matrix = aMatrix * GetMatrix();
		memcpy(myUBO->myMappedData, &matrix, sizeof(glm::mat4));

		for (glTFNode* child : myChildren)
			child->UpdateUBO(aMatrix);
	}

	void glTFNode::UpdateJoints(const glTFModel* aContainer)
	{
		if (mySkinIndex > -1)
		{
			const glTFSkin* skin = aContainer->GetSkin(mySkinIndex);

			glm::mat4 inverseTransform = glm::inverse(GetMatrix());

			size_t numJoints = (uint)skin->myJoints.size();
			std::vector<glm::mat4> jointMatrices(numJoints);
			for (size_t i = 0; i < numJoints; i++)
			{
				jointMatrices[i] = skin->myJoints[i]->GetMatrix() * skin->myInverseBindMatrices[i];
				jointMatrices[i] = inverseTransform * jointMatrices[i];
			}

			memcpy(skin->mySSBO->myMappedData, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
		}

		for (glTFNode* child : myChildren)
			child->UpdateJoints(aContainer);
	}

	void glTFNode::Draw(const glTFModel* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType)
	{
		const glTFSkin* skin = nullptr;
		const glTFImage* image = nullptr;
		const glTFMaterial* material = nullptr;

		if (mySkinIndex > -1)
		{
			skin = aContainer->GetSkin(mySkinIndex);
		}
		if (!skin)
		{
			skin = aContainer->GetEmptySkin();
		}

		for (glTFPrimitive& primitive : myMesh.myPrimitives)
		{
			if (primitive.myMaterial > -1)
			{
				material = aContainer->GetMaterial(primitive.myMaterial);
				Assert(material);

				if (material->myBaseColorTexture > -1)
				{
					const glTFTexture* texture = aContainer->GetTexture(material->myBaseColorTexture);
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
			info.myModelMatrixInfo = &myUBO->myDescriptor;
			info.myImageSamplerInfo = &image->myImage->myDescriptor;
			info.myMaterialInfo = &material->mySSBO->myDescriptor;
			info.myJointMatricesInfo = &skin->mySSBO->myDescriptor;
			VkDescriptorSet descriptorSet = RenderModule::GetInstance()->GetDescriptorSet(aType, info);
			vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);

			vkCmdDrawIndexed(aCommandBuffer, primitive.myIndexCount, 1, primitive.myFirstIndex, 0, 0);
		}

		for (glTFNode* child : myChildren)
			child->Draw(aContainer, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex, aType);
	}

	glm::mat4 glTFNode::GetLocalMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(myScale) * myMatrix;
	}

	glm::mat4 glTFNode::GetMatrix() const
	{
		glm::mat4 matrix = GetLocalMatrix();
		glTFNode* parent = myParent;
		while (parent)
		{
			matrix = parent->GetLocalMatrix() * matrix;
			parent = parent->myParent;
		}
		return matrix;
	}
}
