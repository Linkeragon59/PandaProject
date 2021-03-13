#include "glTFMesh.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Render
{
	void glTFPrimitive::SetupDescriptor(VkDescriptorPool aDescriptorPool, const VkDescriptorBufferInfo* aUBODescriptor, const VkDescriptorImageInfo* aTextureDescriptor)
	{
		VkDevice device = VulkanRenderer::GetInstance()->GetDevice();

		std::array<VkDescriptorSetLayout, 1> layouts = { VulkanPSOContainer::ourPerObjectDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = aDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor set");

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// Binding 0 : Vertex shader uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = aUBODescriptor;

		// Binding 1 : Fragment shader sampler
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = myDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].pImageInfo = aTextureDescriptor;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	glTFMesh::glTFMesh()
	{
	}

	glTFMesh::~glTFMesh()
	{
	}

	void glTFMesh::Load(const tinygltf::Model& aModel, const tinygltf::Mesh& aMesh, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices)
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

			// Parse vertices
			{
				const float* bufferPositions = nullptr;
				const float* bufferColors = nullptr;
				const float* bufferTexCoord = nullptr;
				
				uint32_t numColorComponents = 0;

				// Position attribute is required
				assert(gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end());
				const tinygltf::Accessor& posAccessor = aModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = aModel.bufferViews[posAccessor.bufferView];
				bufferPositions = reinterpret_cast<const float*>(&(aModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				vertexCount = static_cast<uint32_t>(posAccessor.count);

				if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& colorAccessor = aModel.accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& colorView = aModel.bufferViews[colorAccessor.bufferView];
					bufferColors = reinterpret_cast<const float*>(&(aModel.buffers[colorView.buffer].data[colorAccessor.byteOffset + colorView.byteOffset]));
					numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& texCoordAccessor = aModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& texCoordView = aModel.bufferViews[texCoordAccessor.bufferView];
					bufferTexCoord = reinterpret_cast<const float*>(&(aModel.buffers[texCoordView.buffer].data[texCoordAccessor.byteOffset + texCoordView.byteOffset]));
				}

				for (size_t v = 0; v < vertexCount; ++v)
				{
					VulkanPSOContainer::Vertex vert{};
					vert.myPos = glm::make_vec3(&bufferPositions[v * 3]);
					vert.myColor = glm::vec4(1.0f);					
					if (bufferColors)
					{
						if (numColorComponents == 3)
							vert.myColor = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
						else if (numColorComponents == 4)
							vert.myColor = glm::make_vec4(&bufferColors[v * 4]);
					}
					vert.myTexCoord = bufferTexCoord ? glm::make_vec2(&bufferTexCoord[v * 2]) : glm::vec2(0.0f);
					someOutVertices.push_back(vert);
				}
			}

			// Parse indices
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.indices];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				indexCount = static_cast<uint32_t>(accessor.count);

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					uint8_t* buf = new uint8_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
					delete[] buf;
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					uint16_t* buf = new uint16_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
					delete[] buf;
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					uint32_t* buf = new uint32_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
					delete[] buf;
				}
				break;
				default:
					throw std::runtime_error("Index component type not supported");
				}
			}
			
			glTFPrimitive* primitive = new glTFPrimitive();
			primitive->myFirstVertex = vertexStart;
			primitive->myVertexCount = vertexCount;
			primitive->myFirstIndex = indexStart;
			primitive->myIndexCount = indexCount;
			primitive->myMaterial = gltfPrimitive.material;
			myPrimitives.push_back(primitive);
		}
	}
}
