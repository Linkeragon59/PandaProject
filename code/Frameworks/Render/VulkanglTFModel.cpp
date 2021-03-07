#include "VulkanglTFModel.h"

#include "VulkanRenderCore.h"

namespace Render
{
namespace VulkanglTF
{

	Model::Model()
	{
		myDevice = VulkanRenderCore::GetInstance()->GetDevice();
	}

	Model::~Model()
	{
		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);

		for (auto node : myNodes)
			delete node;
	}

	bool Model::LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale,
		uint32_t someLoadingFlags,
		uint32_t someBindingFlags)
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

		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;

		LoadNodes(gltfModel, aScale, vertexBuffer, indexBuffer);

		// Pre-Calculations for requested features
		const bool preTransformVertices = someLoadingFlags & (uint32_t)FileLoadingFlags::PreTransformVertices;
		const bool preMultiplyColor = someLoadingFlags & (uint32_t)FileLoadingFlags::PreMultiplyVertexColors;
		const bool flipY = someLoadingFlags & (uint32_t)FileLoadingFlags::FlipY;
		if (preTransformVertices || preMultiplyColor || flipY)
		{
			auto preTransformNode = [this, preTransformVertices, preMultiplyColor, flipY, &vertexBuffer](Node* aNode)
			{
				if (!aNode->myMesh)
					return;

				const glm::mat4 matrix = aNode->GetMatrix();
				for (const Primitive& primitive : aNode->myMesh->myPrimitives)
				{
					for (uint32_t i = 0; i < primitive.myVertexCount; i++)
					{
						Vertex& vertex = vertexBuffer[primitive.myFirstVertex + i];
						// Pre-transform vertex positions by node-hierarchy
						if (preTransformVertices)
						{
							vertex.myPosition = glm::vec3(matrix * glm::vec4(vertex.myPosition, 1.0f));
							vertex.myNormal = glm::normalize(glm::mat3(matrix) * vertex.myNormal);
						}

						// Pre-Multiply vertex colors with material base color
						if (preMultiplyColor)
						{
							vertex.myColor = GetMaterial(primitive.myMaterial)->myBaseColorFactor * vertex.myColor;
						}

						// Flip Y-Axis of vertex positions
						if (flipY)
						{
							vertex.myPosition.y *= -1.0f;
							vertex.myNormal.y *= -1.0f;
						}
					}
				}
			};
			IterateNodes(preTransformNode, false);
		}

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
		assert((vertexBufferSize > 0) && (indexBufferSize > 0));

		VulkanBuffer vertexStagingBuffer, indexStagingBuffer;

		vertexStagingBuffer.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		indexStagingBuffer.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
		EndOneTimeCommand(commandBuffer, aTransferQueue);

		vertexStagingBuffer.Destroy();
		indexStagingBuffer.Destroy();

		uint32_t uboCount = 0;
		auto countUbos = [&uboCount](Node* aNode) { if (aNode->myMesh) uboCount++; };
		IterateNodes(countUbos, false);

		uint32_t imageCount = 0;
		for (auto material : myMaterials)
			if (material.myBaseColorTexture != -1)
				imageCount++;

		std::vector<VkDescriptorPoolSize> poolSizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uboCount } };
		if (imageCount > 0)
		{
			if (someBindingFlags & (uint32_t)DescriptorBindingFlags::ImageBaseColor)
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount });
			if (someBindingFlags & (uint32_t)DescriptorBindingFlags::ImageNormalMap)
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount });
		}

		VkDescriptorPoolCreateInfo descriptorPoolCI{};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolCI.pPoolSizes = poolSizes.data();
		descriptorPoolCI.maxSets = uboCount + imageCount; // TODO: Understand this
		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolCI, nullptr, &myDescriptorPool), "Failed to create the model descriptor pool");

		return true;
	}

	void Model::LoadTextures(const tinygltf::Model& aModel)
	{
		for (const tinygltf::Image& image : aModel.images)
		{
			Texture texture;
			texture.Load(image, myTransferQueue);
			myTextures.push_back(texture);
		}
	}

	void Model::LoadMaterials(tinygltf::Model& aModel)
	{
		for (tinygltf::Material& gltfMaterial : aModel.materials)
		{
			Material material;
			material.Load(aModel, gltfMaterial);
			myMaterials.push_back(material);
		}
		// Default material at the end of the list
		myMaterials.push_back(Material());
	}

	void Model::LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Scene& scene = aModel.scenes[aModel.defaultScene > -1 ? aModel.defaultScene : 0];
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			// TODO: Do we need to allocate nodes?
			Node* rootNode = new Node;
			rootNode->Load(aModel, scene.nodes[i], aScale, someOutVertices, someOutIndices);
			myNodes.push_back(rootNode);
		}
	}

	Texture* Model::GetTexture(int anIndex)
	{
		if (anIndex >= 0 && anIndex < (int)myTextures.size())
			return &myTextures[anIndex];
		return nullptr;
	}

	Material* Model::GetMaterial(int anIndex)
	{
		// For materials, index -1 represents the default material that has been added at the end of the list
		if (anIndex >= 0 && anIndex < (int)myMaterials.size() - 1)
			return &myMaterials[anIndex];
		else if (anIndex == -1)
			return &myMaterials.back();
		return nullptr;
	}
}
}
