#include "Render_glTFModel.h"

namespace Render
{
	glTFModel::glTFModel(const glTFModelData& someData)
	{
		myDevice = RenderModule::GetInstance()->GetDevice();

		Assert(someData.GetType() == ModelData::Type::glTF);
		LoadFromFile(someData.myFilename, RenderModule::GetInstance()->GetGraphicsQueue(), someData);
	}

	glTFModel::~glTFModel()
	{
		for (glTFNode* node : myNodes)
			delete node;
	}

	void glTFModel::Update(const ModelData& someData)
	{
		if (myAnimations.size() > 0)
			myAnimations[0].Update(1.0f / 60.0f);

		for (glTFNode* node : myNodes)
			node->UpdateJoints(this);

		for (glTFNode* node : myNodes)
			node->UpdateUBO(someData.myMatrix);
	}

	void glTFModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType)
	{
		// All vertices and indices are stored in a single buffer, so we only need to bind once
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer->myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer->myBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (glTFNode* node : myNodes)
			node->Draw(this, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex, aType);
	}

	bool glTFModel::LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue, const ModelData& someData)
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

		std::vector<glTFMesh::Vertex> vertexBuffer;
		std::vector<uint> indexBuffer;
		LoadNodes(gltfModel, vertexBuffer, indexBuffer);

		auto countNodes = [this](glTFNode* aNode) { (void)aNode; myNodeCount++; };
		IterateNodes(countNodes);

		LoadSkins(gltfModel);
		LoadAnimations(gltfModel);

		// Calculate initial pose
		for (glTFNode* node : myNodes)
			node->UpdateJoints(this);

		// Fill initial matrices
		for (glTFNode* node : myNodes)
			node->UpdateUBO(someData.myMatrix);

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(glTFMesh::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint);
		Assert((vertexBufferSize > 0) && (indexBufferSize > 0));

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

		myVertexBuffer = new VulkanBuffer(vertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myIndexBuffer = new VulkanBuffer(indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferCopy copyRegion = {};

			copyRegion.size = vertexBufferSize;
			vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.myBuffer, myVertexBuffer->myBuffer, 1, &copyRegion);

			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.myBuffer, myIndexBuffer->myBuffer, 1, &copyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, myTransferQueue);

		vertexStagingBuffer.Destroy();
		indexStagingBuffer.Destroy();

		return true;
	}

	void glTFModel::LoadImages(const tinygltf::Model& aModel)
	{
		myImages.resize(aModel.images.size() + 1);
		for (uint i = 0; i < (uint)aModel.images.size(); i++)
			myImages[i].Load(aModel, i, myTransferQueue);
		myImages.back().LoadEmpty(myTransferQueue); // Add an empty image at the end
	}

	void glTFModel::LoadTextures(const tinygltf::Model& aModel)
	{
		myTextures.resize(aModel.textures.size());
		for (uint i = 0; i < (uint)aModel.textures.size(); i++)
			myTextures[i].myImageIndex = aModel.textures[i].source;
	}

	void glTFModel::LoadMaterials(tinygltf::Model& aModel)
	{
		myMaterials.resize(aModel.materials.size() + 1);
		for (uint i = 0; i < (uint)aModel.materials.size(); i++)
			myMaterials[i].Load(aModel, i);
		myMaterials.back().LoadEmpty(); // Add an empty material at the end
	}

	void glTFModel::LoadSkins(const tinygltf::Model& aModel)
	{
		mySkins.resize(aModel.skins.size() + 1);
		for (uint i = 0; i < (uint)aModel.skins.size(); i++)
			mySkins[i].Load(this, aModel, i);
		mySkins.back().LoadEmpty(); // Add an empty skin at the end
	}

	void glTFModel::LoadAnimations(const tinygltf::Model& aModel)
	{
		myAnimations.resize(aModel.animations.size());
		for (uint i = 0; i < (uint)aModel.animations.size(); i++)
			myAnimations[i].Load(this, aModel, i);
	}

	void glTFModel::LoadNodes(const tinygltf::Model& aModel, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
	{
		const tinygltf::Scene& scene = aModel.scenes[aModel.defaultScene > -1 ? aModel.defaultScene : 0];
		myNodes.resize(scene.nodes.size());
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			myNodes[i] = new glTFNode;
			myNodes[i]->Load(aModel, scene.nodes[i], someOutVertices, someOutIndices);
		}
	}

	glTFNode* glTFModel::GetNodeByIndex(uint anIndex)
	{
		glTFNode* node = nullptr;
		auto findNode = [anIndex, &node](glTFNode* aNode) { if (aNode->myIndex == anIndex) node = aNode; };
		IterateNodes(findNode);
		return node;
	}
}
