#include "glTFModel.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>

namespace Render
{
	glTFModel::glTFModel()
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();
	}

	glTFModel::~glTFModel()
	{
		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);

		for (glTFTexture* texture : myTextures)
			delete texture;

		for (glTFMaterial* material : myMaterials)
			delete material;

		for (glTFNode* node : myNodes)
			delete node;
	}

	bool glTFModel::LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale)
	{
		myTransferQueue = aTransferQueue;

		tinygltf::TinyGLTF gltfContext;
		// Can override the Image loading with
		//gltfContext.SetImageLoader

		tinygltf::Model gltfModel;
		std::string error, warning;
		if (!gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, aFilename))
		{
			// TODO: Display the error message
			return false;
		}

		LoadTextures(gltfModel);
		LoadMaterials(gltfModel);

		std::vector<VulkanPSOContainer::Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;

		LoadNodes(gltfModel, aScale, vertexBuffer, indexBuffer);

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(VulkanPSOContainer::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
		assert((vertexBufferSize > 0) && (indexBufferSize > 0));

		VulkanBuffer vertexStagingBuffer, indexStagingBuffer;

		vertexStagingBuffer.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vertexStagingBuffer.Map();
		memcpy(vertexStagingBuffer.myMappedData, vertexBuffer.data(), vertexBufferSize);
		vertexStagingBuffer.Unmap();

		indexStagingBuffer.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStagingBuffer.Map();
		memcpy(indexStagingBuffer.myMappedData, indexBuffer.data(), indexBufferSize);
		indexStagingBuffer.Unmap();

		myVertexBuffer.Create(vertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myIndexBuffer.Create(indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBuffer commandBuffer = BeginOneTimeCommand();
		{
			VkBufferCopy copyRegion = {};

			copyRegion.size = vertexBufferSize;
			vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.myBuffer, myVertexBuffer.myBuffer, 1, &copyRegion);

			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.myBuffer, myIndexBuffer.myBuffer, 1, &copyRegion);
		}
		EndOneTimeCommand(commandBuffer, myTransferQueue);

		vertexStagingBuffer.Destroy();
		indexStagingBuffer.Destroy();

		auto countNodes = [this](glTFNode* aNode) { (void)aNode; myNodeCount++; };
		IterateNodes(countNodes);

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = myNodeCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = myNodeCount;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = myNodeCount;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the model descriptor pool");

		auto prepareNodes = [this](glTFNode* aNode)
		{
			if (aNode->myMesh)
			{
				for (glTFPrimitive* primitive : aNode->myMesh->myPrimitives)
				{
					const VkDescriptorImageInfo* textureDescriptor = VulkanRenderer::GetInstance()->GetEmptyTextureDescriptor();
					if (primitive->myMaterial >= 0 && myMaterials[primitive->myMaterial]->myBaseColorTexture >= 0)
					{
						textureDescriptor = &myTextures[myMaterials[primitive->myMaterial]->myBaseColorTexture]->myImage.myDescriptor;
					}
					primitive->SetupDescriptor(myDescriptorPool, &aNode->myUniformBuffer.myDescriptor, textureDescriptor);
				}
			}
		};
		IterateNodes(prepareNodes);

		// Init the nodes matrices
		Update();

		LoadAnimations(gltfModel);

		return true;
	}

	void glTFModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout)
	{
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);

		std::array<VkBuffer, 1> modelVertexBuffers = { myVertexBuffer.myBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, (uint32_t)modelVertexBuffers.size(), modelVertexBuffers.data(), offsets);

		auto drawNodes = [this, aCommandBuffer, aPipelineLayout](glTFNode* aNode)
		{
			if (aNode->myMesh)
			{
				for (const glTFPrimitive* primitive : aNode->myMesh->myPrimitives)
				{
					vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, 1, 1, &primitive->myDescriptorSet, 0, NULL);
					vkCmdDrawIndexed(aCommandBuffer, primitive->myIndexCount, 1, primitive->myFirstIndex, 0, 0);
				}
			}
		};
		IterateNodes(drawNodes, false);
	}

	void glTFModel::Update()
	{
		if (myAnimations.size() > 0)
		{
			if (myAnimationTime < 0.f)
				myAnimationTime = myAnimations[0].myStartTime;
			myAnimationTime += 0.1f;
			if (myAnimationTime > myAnimations[0].myEndTime)
				myAnimationTime = myAnimations[0].myStartTime;

			for (auto& channel : myAnimations[0].myChannels)
			{
				glTFNode* node = GetNodeByIndex(channel.myNodeIndex);
				if (!node)
					continue;

				glTFAnimationSampler& sampler = myAnimations[0].mySamplers[channel.mySamplerIndex];
				if (sampler.myTimes.size() > sampler.myValues.size())
					continue;

				for (size_t i = 0; i < sampler.myTimes.size() - 1; i++)
				{
					if ((myAnimationTime >= sampler.myTimes[i]) && (myAnimationTime <= sampler.myTimes[i + 1]))
					{
						float u = std::max(0.0f, myAnimationTime - sampler.myTimes[i]) / (sampler.myTimes[i + 1] - sampler.myTimes[i]);
						if (u <= 1.0f)
						{
							switch (channel.myType)
							{
							case glTFAnimationChannel::Type::TRANSLATION:
								node->myTranslation = glm::vec3(glm::mix(sampler.myValues[i], sampler.myValues[i + 1], u));
								break;
							case glTFAnimationChannel::Type::SCALE:
								node->myScale = glm::vec3(glm::mix(sampler.myValues[i], sampler.myValues[i + 1], u));
								break;
							case glTFAnimationChannel::Type::ROTATION:
								glm::quat q1 = { sampler.myValues[i].x, sampler.myValues[i].y, sampler.myValues[i].z, sampler.myValues[i].w };
								glm::quat q2 = { sampler.myValues[i + 1].x, sampler.myValues[i + 1].y, sampler.myValues[i + 1].z, sampler.myValues[i + 1].w };
								node->myRotation = glm::normalize(glm::slerp(q1, q2, u));
								break;
							}
						}
					}
				}
			}
		}

		for (glTFNode* node : myNodes)
			node->Update();

		/*auto debugNodes = [](glTFNode* aNode)
		{
			std::cout << aNode->myName << std::endl;
			std::cout << glm::to_string(aNode->GetMatrix()) << std::endl;
		};
		IterateNodes(debugNodes);*/
	}

	void glTFModel::LoadTextures(const tinygltf::Model& aModel)
	{
		for (const tinygltf::Image& image : aModel.images)
		{
			glTFTexture* texture = new glTFTexture();
			texture->Load(image, myTransferQueue);
			myTextures.push_back(texture);
		}
	}

	void glTFModel::LoadMaterials(tinygltf::Model& aModel)
	{
		for (tinygltf::Material& gltfMaterial : aModel.materials)
		{
			glTFMaterial* material = new glTFMaterial();
			material->Load(aModel, gltfMaterial);
			myMaterials.push_back(material);
		}
	}

	void glTFModel::LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Scene& scene = aModel.scenes[aModel.defaultScene > -1 ? aModel.defaultScene : 0];
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			glTFNode* rootNode = new glTFNode;
			rootNode->Load(aModel, scene.nodes[i], aScale, someOutVertices, someOutIndices);
			myNodes.push_back(rootNode);
		}
	}

	glTFNode* glTFModel::GetNodeByIndex(uint32_t anIndex)
	{
		glTFNode* node = nullptr;
		auto findNode = [anIndex, &node](glTFNode* aNode) { if (aNode->myIndex == anIndex) node = aNode; };
		IterateNodes(findNode);
		return node;
	}

	void glTFModel::LoadAnimations(const tinygltf::Model& aModel)
	{
		for (uint32_t i = 0; i < (uint32_t)aModel.animations.size(); i++)
		{
			glTFAnimation animation{};
			animation.Load(aModel, i);
			myAnimations.push_back(animation);
		}
	}

}
