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
		for (auto node : myNodes)
			delete node;
	}

	bool Model::LoadFromFile(std::string aFilename, VkQueue aTransferQueue, uint32_t /*someLoadingFlags*/, float aScale)
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

		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;

		const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			Node* rootNode = LoadNode(gltfModel, scene.nodes[i], aScale, vertexBuffer, indexBuffer);
			assert(rootNode);
			myNodes.push_back(rootNode);
		}



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

	Node* Model::LoadNode(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Node& gltfNode = aModel.nodes[aNodeIndex];

		// TODO: Do we need to allocate nodes?
		Node* node = new Node;
		node->myIndex = aNodeIndex;
		node->myName = gltfNode.name;

		node->myMatrix = glm::mat4(1.0f);
		if (gltfNode.translation.size() == 3)
			node->myTranslation = glm::make_vec3(gltfNode.translation.data());
		if (gltfNode.rotation.size() == 4)
			node->myRotation = glm::make_quat(gltfNode.rotation.data());
		if (gltfNode.scale.size() == 3)
			node->myScale = glm::make_vec3(gltfNode.scale.data());
		if (gltfNode.matrix.size() == 16)
			node->myMatrix = glm::make_mat4x4(gltfNode.matrix.data());

		// Load children
		for (size_t i = 0; i < gltfNode.children.size(); ++i)
		{
			Node* childNode = LoadNode(aModel, gltfNode.children[i], aScale, someOutVertices, someOutIndices);
			childNode->myParent = node;
			node->myChildren.push_back(childNode);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
		{
			const tinygltf::Mesh& gltfMesh = aModel.meshes[gltfNode.mesh];
			Mesh* mesh = new Mesh();
			mesh->myName = gltfMesh.name;
			for (size_t i = 0; i < gltfMesh.primitives.size(); ++i)
			{
				const tinygltf::Primitive& gltfPrimitive = gltfMesh.primitives[i];
				if (gltfPrimitive.indices < 0)
					continue;

				uint32_t vertexStart = static_cast<uint32_t>(someOutVertices.size());
				uint32_t vertexCount = 0;
				uint32_t indexStart = static_cast<uint32_t>(someOutIndices.size());
				uint32_t indexCount = 0;

				glm::vec3 posMin{};
				glm::vec3 posMax{};

				// Parse vertices
				{
					const float* bufferPositions = nullptr;
					const float* bufferNormals = nullptr;
					const float* bufferUVs = nullptr;
					const float* bufferColors = nullptr;
					uint32_t numColorComponents = 0;

					// Position attribute is required
					assert(gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end());
					const tinygltf::Accessor& posAccessor = aModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = aModel.bufferViews[posAccessor.bufferView];
					bufferPositions = reinterpret_cast<const float*>(&(aModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);

					if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& normAccessor = aModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = aModel.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float*>(&(aModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
					}

					if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& uvAccessor = aModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = aModel.bufferViews[uvAccessor.bufferView];
						bufferUVs = reinterpret_cast<const float*>(&(aModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					}

					if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end())
					{
						const tinygltf::Accessor& colorAccessor = aModel.accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& colorView = aModel.bufferViews[colorAccessor.bufferView];
						bufferColors = reinterpret_cast<const float*>(&(aModel.buffers[colorView.buffer].data[colorAccessor.byteOffset + colorView.byteOffset]));
						numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
					}

					for (size_t v = 0; v < vertexCount; ++v)
					{
						Vertex vert{};
						vert.myPosition = glm::make_vec3(&bufferPositions[v * 3]);
						vert.myNormal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
						vert.myUV = bufferUVs ? glm::make_vec2(&bufferUVs[v * 2]) : glm::vec2(0.0f);
						vert.myColor = glm::vec4(1.0f);
						if (bufferColors)
						{
							if (numColorComponents == 3)
								vert.myColor = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
							else if (numColorComponents == 4)
								vert.myColor = glm::make_vec4(&bufferColors[v * 4]);
						}
						someOutVertices.push_back(vert);
					}
				}

				// Parse indices
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.indices];
					const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
					indexCount = static_cast<uint32_t>(accessor.count);

					// TODO: Avoid allocs?
					switch (accessor.componentType) 
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						uint8_t* buf = new uint8_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
						for (size_t index = 0; index < accessor.count; ++index)
							someOutIndices.push_back(buf[index] + vertexStart);
						delete buf;
					}
					break;
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						uint16_t* buf = new uint16_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
						for (size_t index = 0; index < accessor.count; ++index)
							someOutIndices.push_back(buf[index] + vertexStart);
						delete buf;
					}
					break;
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						uint32_t* buf = new uint32_t[accessor.count];
						memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
						for (size_t index = 0; index < accessor.count; ++index)
							someOutIndices.push_back(buf[index] + vertexStart);
						delete buf;
					}
					break;
					default:
						throw std::runtime_error("Index component type not supported");
					}
				}
				
				mesh->myPrimitives.push_back(Primitive());
				Primitive& primitive = mesh->myPrimitives.back();
				primitive.myFirstVertex = vertexStart;
				primitive.myVertexCount = vertexCount;
				primitive.myFirstIndex = indexStart;
				primitive.myIndexCount = indexCount;
				primitive.myMinPos = posMin;
				primitive.myMaxPos = posMax;
			}
			node->myMesh = mesh;
		}

		// Call one update to init the matrix
		node->Update();
		return node;
	}
}
}
