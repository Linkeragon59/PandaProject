#include "VulkanglTFMesh.h"

#include "VulkanRenderCore.h"

#include <iostream>

namespace Render
{
namespace VulkanglTF
{
	Mesh::Mesh()
	{
		myUniformBuffer.Create(
			sizeof(myUniformData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUniformBuffer.Map();
		myUniformBuffer.SetupDescriptor();
	}

	Mesh::~Mesh()
	{
		myUniformBuffer.Destroy();
	}

	void Mesh::Load(const tinygltf::Model& aModel, const tinygltf::Mesh& aMesh, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		myName = aMesh.name;
		for (size_t i = 0; i < aMesh.primitives.size(); ++i)
		{
			const tinygltf::Primitive& gltfPrimitive = aMesh.primitives[i];
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
				std::cout << "Mesh " << myName.c_str() << " count " << vertexCount << std::endl;
				std::cout << "So far " << someOutVertices.size() << std::endl;
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

			myPrimitives.push_back(Primitive());
			Primitive& primitive = myPrimitives.back();
			primitive.myFirstVertex = vertexStart;
			primitive.myVertexCount = vertexCount;
			primitive.myFirstIndex = indexStart;
			primitive.myIndexCount = indexCount;
			primitive.myMinPos = posMin;
			primitive.myMaxPos = posMax;
			primitive.myMaterial = gltfPrimitive.material;
		}
	}
}
}
