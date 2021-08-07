#include "VulkanglTFModel.h"

#include "VulkanHelpers.h"
#include "VulkanRender.h"

namespace Render::Vulkan::glTF
{
	Model::Model(const glTFModelData& someData)
	{
		myDevice = RenderCore::GetInstance()->GetDevice();
		LoadFromFile(someData.myFilename, RenderCore::GetInstance()->GetGraphicsQueue(), someData.myMatrix[3][3]);
	}

	Model::~Model()
	{
		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);

		for (Node* node : myNodes)
			delete node;
	}

	void Model::Update()
	{
		if (myAnimations.size() > 0)
			myAnimations[0].Update(1.0f / 60.0f);

		for (Node* node : myNodes)
			node->UpdateJoints(this);

		for (Node* node : myNodes)
			node->UpdateUBO();
	}

	void Model::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) const
	{
		// All vertices and indices are stored in a single buffer, so we only need to bind once
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer.myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (Node* node : myNodes)
			node->Draw(this, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex);
	}

	bool Model::LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue, float aScale)
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
		std::vector<uint> indexBuffer;
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
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint);
		Assert((vertexBufferSize > 0) && (indexBufferSize > 0));

		Buffer vertexStagingBuffer, indexStagingBuffer;

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

		return true;
	}

	void Model::LoadImages(const tinygltf::Model& aModel)
	{
		myImages.resize(aModel.images.size() + 1);
		for (uint i = 0; i < (uint)aModel.images.size(); i++)
			myImages[i].Load(aModel, i, myTransferQueue);
		myImages.back().LoadEmpty(myTransferQueue); // Add an empty image at the end
	}

	void Model::LoadTextures(const tinygltf::Model& aModel)
	{
		myTextures.resize(aModel.textures.size());
		for (uint i = 0; i < (uint)aModel.textures.size(); i++)
			myTextures[i].myImageIndex = aModel.textures[i].source;
	}

	void Model::LoadMaterials(tinygltf::Model& aModel)
	{
		myMaterials.resize(aModel.materials.size() + 1);
		for (uint i = 0; i < (uint)aModel.materials.size(); i++)
			myMaterials[i].Load(aModel, i);
		myMaterials.back().LoadEmpty(); // Add an empty material at the end
	}

	void Model::LoadSkins(const tinygltf::Model& aModel)
	{
		mySkins.resize(aModel.skins.size() + 1);
		for (uint i = 0; i < (uint)aModel.skins.size(); i++)
			mySkins[i].Load(this, aModel, i);
		mySkins.back().LoadEmpty(); // Add an empty skin at the end
	}

	void Model::LoadAnimations(const tinygltf::Model& aModel)
	{
		myAnimations.resize(aModel.animations.size());
		for (uint i = 0; i < (uint)aModel.animations.size(); i++)
			myAnimations[i].Load(this, aModel, i);
	}

	void Model::LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
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
		uint nbPrimitives = 0;
		auto countPrimitives = [&nbPrimitives](Node* aNode) { nbPrimitives += (uint)aNode->myMesh.myPrimitives.size(); };
		IterateNodes(countPrimitives);

		if (nbPrimitives == 0)
			return;

		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = nbPrimitives;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = nbPrimitives;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 2 * nbPrimitives;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = nbPrimitives;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void Model::SetupDescriptorSets()
	{
		for (Node* node : myNodes)
			node->SetupDescriptorSet(this, myDescriptorPool);
	}

	Node* Model::GetNodeByIndex(uint anIndex)
	{
		Node* node = nullptr;
		auto findNode = [anIndex, &node](Node* aNode) { if (aNode->myIndex == anIndex) node = aNode; };
		IterateNodes(findNode);
		return node;
	}
}
