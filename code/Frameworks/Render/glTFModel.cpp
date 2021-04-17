#include "glTFModel.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Render
{
namespace glTF
{
	Model::Model()
	{
		myDevice = Render::Vulkan::Renderer::GetInstance()->GetDevice();
	}

	Model::~Model()
	{
		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);

		for (Node* node : myNodes)
			delete node;
	}

	bool Model::LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale)
	{
		myTransferQueue = aTransferQueue;

		tinygltf::TinyGLTF gltfContext;
		tinygltf::Model gltfModel;
		std::string error, warning;
		if (!gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, aFilename))
		{
			// TODO: Display the error message
			return false;
		}

		LoadImages(gltfModel);
		LoadTextures(gltfModel);
		LoadMaterials(gltfModel);

		std::vector<Mesh::Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		LoadNodes(gltfModel, aScale, vertexBuffer, indexBuffer);

		auto countNodes = [this](Node* aNode) { (void)aNode; myNodeCount++; };
		IterateNodes(countNodes);

		LoadSkins(gltfModel);
		LoadAnimations(gltfModel);

		// Calculate initial pose
		for (Node* node : myNodes)
			node->UpdateJoints(this);

		// Fill initial matrices
		for (Node* node : myNodes)
			node->UpdateUBO();

		SetupDescriptorPool();
		SetupDescriptorSets();

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(Mesh::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
		assert((vertexBufferSize > 0) && (indexBufferSize > 0));

		Render::Vulkan::Buffer vertexStagingBuffer, indexStagingBuffer;

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

		VkCommandBuffer commandBuffer = Render::Vulkan::BeginOneTimeCommand();
		{
			VkBufferCopy copyRegion = {};

			copyRegion.size = vertexBufferSize;
			vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.myBuffer, myVertexBuffer.myBuffer, 1, &copyRegion);

			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.myBuffer, myIndexBuffer.myBuffer, 1, &copyRegion);
		}
		Render::Vulkan::EndOneTimeCommand(commandBuffer, myTransferQueue);

		vertexStagingBuffer.Destroy();
		indexStagingBuffer.Destroy();

		return true;
	}

	void Model::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout)
	{
		// All vertices and indices are stored in a single buffer, so we only need to bind once
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer.myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (Node* node : myNodes)
			node->Draw(this, aCommandBuffer, aPipelineLayout);
	}

	void Model::Update()
	{
		if (myAnimations.size() > 0)
			myAnimations[0].Update(1.0f/60.0f);

		for (Node* node : myNodes)
			node->UpdateJoints(this);

		for (Node* node : myNodes)
			node->UpdateUBO();
	}

	void Model::LoadImages(const tinygltf::Model& aModel)
	{
		myImages.resize(aModel.images.size() + 1);
		for (uint32_t i = 0; i < (uint32_t)aModel.images.size(); i++)
			myImages[i].Load(aModel, i, myTransferQueue);
		myImages.back().LoadEmpty(myTransferQueue); // Add an empty image at the end
	}

	void Model::LoadTextures(const tinygltf::Model& aModel)
	{
		myTextures.resize(aModel.textures.size());
		for (uint32_t i = 0; i < (uint32_t)aModel.textures.size(); i++)
			myTextures[i].myImageIndex = aModel.textures[i].source;
	}

	void Model::LoadMaterials(tinygltf::Model& aModel)
	{
		myMaterials.resize(aModel.materials.size() + 1);
		for (uint32_t i = 0; i < (uint32_t)aModel.materials.size(); i++)
			myMaterials[i].Load(aModel, i);
		myMaterials.back().LoadEmpty(); // Add an empty material at the end
	}

	void Model::LoadSkins(const tinygltf::Model& aModel)
	{
		mySkins.resize(aModel.skins.size() + 1);
		for (uint32_t i = 0; i < (uint32_t)aModel.skins.size(); i++)
			mySkins[i].Load(this, aModel, i);
		mySkins.back().LoadEmpty(); // Add an empty skin at the end
	}

	void Model::LoadAnimations(const tinygltf::Model& aModel)
	{
		myAnimations.resize(aModel.animations.size());
		for (uint32_t i = 0; i < (uint32_t)aModel.animations.size(); i++)
			myAnimations[i].Load(this, aModel, i);
	}

	void Model::LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Scene& scene = aModel.scenes[aModel.defaultScene > -1 ? aModel.defaultScene : 0];
		myNodes.resize(scene.nodes.size());
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			myNodes[i] = new Node;
			myNodes[i]->Load(aModel, scene.nodes[i], aScale, someOutVertices, someOutIndices);
		}
	}

	void Model::SetupDescriptorPool()
	{
		assert(myNodeCount > 0);

		std::vector<VkDescriptorPoolSize> poolSizes{};
		VkDescriptorPoolSize uboSize;
		uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboSize.descriptorCount = myNodeCount;
		poolSizes.push_back(uboSize);
		if (myImages.size() > 0)
		{
			VkDescriptorPoolSize samplerSize;
			samplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerSize.descriptorCount = (uint32_t)myImages.size();
			poolSizes.push_back(samplerSize);
		}
		if (myMaterials.size() > 0)
		{
			VkDescriptorPoolSize storageSize;
			storageSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageSize.descriptorCount = (uint32_t)myMaterials.size();
			poolSizes.push_back(storageSize);
		}
		if (mySkins.size() > 0)
		{
			VkDescriptorPoolSize storageSize;
			storageSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageSize.descriptorCount = (uint32_t)mySkins.size();
			poolSizes.push_back(storageSize);
		}

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = myNodeCount + (uint32_t)myImages.size() + (uint32_t)myMaterials.size() + (uint32_t)mySkins.size();

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void Model::SetupDescriptorSets()
	{
		for (Node* node : myNodes)
			node->SetupDescriptorSet(myDescriptorPool);

		for (Image& image : myImages)
			image.SetupDescriptorSet(myDescriptorPool);

		for (Material& material : myMaterials)
			material.SetupDescriptorSet(myDescriptorPool);

		for (Skin& skin : mySkins)
			skin.SetupDescriptorSet(myDescriptorPool);
	}

	Node* Model::GetNodeByIndex(uint32_t anIndex)
	{
		Node* node = nullptr;
		auto findNode = [anIndex, &node](Node* aNode) { if (aNode->myIndex == anIndex) node = aNode; };
		IterateNodes(findNode);
		return node;
	}
}
}
