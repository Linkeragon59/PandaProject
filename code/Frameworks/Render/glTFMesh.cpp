#include "glTFMesh.h"

namespace Render
{
namespace glTF
{
	void Mesh::Load(const tinygltf::Model& aModel, uint32_t aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
	{
		const tinygltf::Mesh& gltfMesh = aModel.meshes[aMeshIndex];

		myName = gltfMesh.name;

		for (size_t i = 0; i < gltfMesh.primitives.size(); ++i)
		{
			const tinygltf::Primitive& gltfPrimitive = gltfMesh.primitives[i];
			if (gltfPrimitive.indices < 0)
				continue;

			uint32_t vertexStart = static_cast<uint32_t>(someOutVertices.size());
			uint32_t vertexCount = 0;
			uint32_t indexStart = static_cast<uint32_t>(someOutIndices.size());
			uint32_t indexCount = 0;

			// Parse vertices
			{
				const float* bufferPositions = nullptr;
				const float* bufferColors = nullptr;
				const float* bufferTexCoord = nullptr;
				const float* bufferNormals = nullptr;
				const uint16_t* bufferJointIndices = nullptr;
				const float* bufferJointWeights = nullptr;
				
				uint32_t numColorComponents = 0;

				// Position attribute is required
				assert(gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end());
				if (gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferPositions = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = static_cast<uint32_t>(accessor.count);
				}

				if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferColors = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					numColorComponents = accessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferTexCoord = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferNormals = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("JOINTS_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferJointIndices = reinterpret_cast<const uint16_t*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("WEIGHTS_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferJointWeights = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				const bool hasSkin = (bufferJointIndices && bufferJointWeights);

				for (size_t v = 0; v < vertexCount; ++v)
				{
					Vertex vert{};
					vert.myPosition = glm::make_vec3(&bufferPositions[v * 3]);
					vert.myNormal = bufferNormals ? glm::normalize(glm::make_vec3(&bufferNormals[v * 3])) : glm::vec3(0.0f);
					vert.myUV = bufferTexCoord ? glm::make_vec2(&bufferTexCoord[v * 2]) : glm::vec2(0.0f);
					vert.myColor = glm::vec4(1.0f);
					if (bufferColors)
					{
						if (numColorComponents == 3)
							vert.myColor = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
						else if (numColorComponents == 4)
							vert.myColor = glm::make_vec4(&bufferColors[v * 4]);
					}
					vert.myJoint = hasSkin ? glm::vec4(glm::make_vec4(&bufferJointIndices[v * 4])) : glm::vec4(0.0f);
					vert.myWeight = hasSkin ? glm::make_vec4(&bufferJointWeights[v * 4]) : glm::vec4(1.0f);
					//vert.myTangent
					someOutVertices.push_back(vert);
				}
			}

			// Parse indices
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.indices];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				indexCount = static_cast<uint32_t>(accessor.count);

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
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
			primitive.myMaterial = gltfPrimitive.material;
		}
	}
}
}
