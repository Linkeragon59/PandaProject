#include "Render_DescriptorContainer.h"

namespace Render
{
	void DescriptorContainer::Create(ShaderHelpers::BindType aType)
	{
		myDevice = RenderModule::GetInstance()->GetDevice();
		myType = aType;

		myAllocatedSets.resize(1);

		switch (myType)
		{
		case ShaderHelpers::BindType::Camera:
			{
				const uint maxCamerasCount = 64;

				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : ViewProj
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLayout),
					"Failed to create the Camera DescriptorSetLayout");

				std::array<VkDescriptorPoolSize, 1> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = maxCamerasCount;

				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
				descriptorPoolInfo.pPoolSizes = poolSizes.data();
				descriptorPoolInfo.maxSets = maxCamerasCount;

				VK_CHECK_RESULT(
					vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myPool),
					"Failed to create the Camera DescriptorPool");
			}
			break;
		case ShaderHelpers::BindType::SimpleObject:
			{
				const uint maxObjectsCount = 64;

				std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
				// Binding 0 : Model
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				// Binding 1 : Texture Sampler
				bindings[1].binding = 1;
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[1].descriptorCount = 1;
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLayout),
					"Failed to create the SimpleObject DescriptorSetLayout");

				std::array<VkDescriptorPoolSize, 2> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = maxObjectsCount;
				poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				poolSizes[1].descriptorCount = maxObjectsCount;

				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
				descriptorPoolInfo.pPoolSizes = poolSizes.data();
				descriptorPoolInfo.maxSets = maxObjectsCount;

				VK_CHECK_RESULT(
					vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myPool),
					"Failed to create the SimpleObject DescriptorPool");
			}
			break;
		case ShaderHelpers::BindType::Object:
			{
				const uint maxObjectsCount = 64;

				std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
				// Binding 0 : Model
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				// Binding 1 : Texture Sampler
				bindings[1].binding = 1;
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[1].descriptorCount = 1;
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				// Binding 2 : Material
				bindings[2].binding = 2;
				bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				bindings[2].descriptorCount = 1;
				bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				// Binding 3 : Joint Matrices
				bindings[3].binding = 3;
				bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				bindings[3].descriptorCount = 1;
				bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLayout),
					"Failed to create the Object DescriptorSetLayout");

				std::array<VkDescriptorPoolSize, 3> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = maxObjectsCount;
				poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				poolSizes[1].descriptorCount = maxObjectsCount;
				poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				poolSizes[2].descriptorCount = 2 * maxObjectsCount;

				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
				descriptorPoolInfo.pPoolSizes = poolSizes.data();
				descriptorPoolInfo.maxSets = maxObjectsCount;

				VK_CHECK_RESULT(
					vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myPool),
					"Failed to create the Object DescriptorPool");
			}
			break;
		case ShaderHelpers::BindType::LightsSet:
			{
				const uint maxLightsSetsCount = 64;

				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : Lights
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLayout),
					"Failed to create the LightsSet DescriptorSetLayout");

				std::array<VkDescriptorPoolSize, 1> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = maxLightsSetsCount;

				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
				descriptorPoolInfo.pPoolSizes = poolSizes.data();
				descriptorPoolInfo.maxSets = maxLightsSetsCount;

				VK_CHECK_RESULT(
					vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myPool),
					"Failed to create the LightsSet DescriptorPool");
			}
			break;
		case ShaderHelpers::BindType::Gui:
			{
				const uint maxGuiCount = 64;

				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : Font Sampler
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLayout),
					"Failed to create the Gui DescriptorSetLayout");

				std::array<VkDescriptorPoolSize, 1> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				poolSizes[0].descriptorCount = maxGuiCount;

				VkDescriptorPoolCreateInfo descriptorPoolInfo{};
				descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
				descriptorPoolInfo.pPoolSizes = poolSizes.data();
				descriptorPoolInfo.maxSets = maxGuiCount;

				VK_CHECK_RESULT(
					vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myPool),
					"Failed to create the Gui DescriptorPool");
			}
			break;
		default:
			Assert(false, "Unsupported Descriptor Layout");
			break;
		}
	}

	void DescriptorContainer::Destroy()
	{
		if (myType == ShaderHelpers::BindType::Count)
			return;

		myAllocatedSets.clear();
		myCurrentFrame = 0;

		vkDestroyDescriptorSetLayout(myDevice, myLayout, nullptr);
		myLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(myDevice, myPool, nullptr);
		myPool = VK_NULL_HANDLE;

		myType = ShaderHelpers::BindType::Count;

		myDevice = VK_NULL_HANDLE;
	}

	VkDescriptorSet DescriptorContainer::GetDescriptorSet(const ShaderHelpers::DescriptorInfo& someDescriptorInfo, uint aFramesToKeep)
	{
		if (aFramesToKeep > (uint)myAllocatedSets.size())
		{
			myAllocatedSets.resize(aFramesToKeep);
		}

		FrameAllocatedDescriptors& descriptorSets = myAllocatedSets[myCurrentFrame];
		if (descriptorSets.myFirstAvailableSet >= (uint)descriptorSets.mySets.size())
		{
			descriptorSets.myFirstAvailableSet = (uint)descriptorSets.mySets.size();

			VkDescriptorSet newSet;
			AllocateDescriptorSet(newSet);
			descriptorSets.mySets.push_back(newSet);
		}

		UpdateDescriptorSet(someDescriptorInfo, descriptorSets.mySets[descriptorSets.myFirstAvailableSet]);
		return descriptorSets.mySets[descriptorSets.myFirstAvailableSet++];
	}

	void DescriptorContainer::RecycleDescriptors()
	{
		if (myType == ShaderHelpers::BindType::Count)
			return;

		myCurrentFrame = (myCurrentFrame + 1) % (uint)myAllocatedSets.size();
		FrameAllocatedDescriptors& descriptorSets = myAllocatedSets[myCurrentFrame];
		descriptorSets.myFirstAvailableSet = 0;
	}

	void DescriptorContainer::AllocateDescriptorSet(VkDescriptorSet& anOutDescriptorSet)
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { myLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();

		// TODO : Handle VK_ERROR_FRAGMENTED_POOL and VK_ERROR_OUT_OF_POOL_MEMORY
		VK_CHECK_RESULT(
			vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &anOutDescriptorSet),
			"Failed to allocate a DescriptorSet");
	}

	void DescriptorContainer::UpdateDescriptorSet(const ShaderHelpers::DescriptorInfo& someDescriptorInfo, VkDescriptorSet aDescriptorSet)
	{
		switch (myType)
		{
		case ShaderHelpers::BindType::Camera:
			{
				const ShaderHelpers::CameraDescriptorInfo& info = static_cast<const ShaderHelpers::CameraDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : ViewProj
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pBufferInfo = info.myViewProjMatricesInfo;

				vkUpdateDescriptorSets(myDevice, (uint)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
			}
			break;
		case ShaderHelpers::BindType::SimpleObject:
			{
				const ShaderHelpers::ObjectDescriptorInfo& info = static_cast<const ShaderHelpers::ObjectDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
				// Binding 0 : Model
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].dstSet = aDescriptorSet;
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].pBufferInfo = info.myModelMatrixInfo;
				// Binding 1 : Texture Sampler
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].dstSet = aDescriptorSet;
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].pImageInfo = info.myImageSamplerInfo ? info.myImageSamplerInfo : RenderModule::GetInstance()->GetWhiteTextureDescriptorInfo();

				vkUpdateDescriptorSets(myDevice, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
			break;
		case ShaderHelpers::BindType::Object:
			{
				const ShaderHelpers::ObjectDescriptorInfo& info = static_cast<const ShaderHelpers::ObjectDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
				// Binding 0 : Model
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].dstSet = aDescriptorSet;
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].pBufferInfo = info.myModelMatrixInfo;
				// Binding 1 : Texture Sampler
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].dstSet = aDescriptorSet;
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].pImageInfo = info.myImageSamplerInfo ? info.myImageSamplerInfo : RenderModule::GetInstance()->GetWhiteTextureDescriptorInfo();
				// Binding 2 : Material
				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].dstSet = aDescriptorSet;
				descriptorWrites[2].dstBinding = 2;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrites[2].pBufferInfo = info.myMaterialInfo ? info.myMaterialInfo : RenderModule::GetInstance()->GetDefaultMaterialDescriptorInfo();
				// Binding 3 : Joint Matrices
				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].descriptorCount = 1;
				descriptorWrites[3].dstSet = aDescriptorSet;
				descriptorWrites[3].dstBinding = 3;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrites[3].pBufferInfo = info.myJointMatricesInfo ? info.myJointMatricesInfo : RenderModule::GetInstance()->GetDefaultJointsMatrixDescriptorInfo();

				vkUpdateDescriptorSets(myDevice, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
			break;
		case ShaderHelpers::BindType::LightsSet:
			{
				const ShaderHelpers::LightsSetDescriptorInfo& info = static_cast<const ShaderHelpers::LightsSetDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : Lights
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pBufferInfo = info.myLightsInfo;

				vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}
			break;
		case ShaderHelpers::BindType::Gui:
			{
				const ShaderHelpers::GuiDescriptorInfo& info = static_cast<const ShaderHelpers::GuiDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : Font Sampler
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[0].pImageInfo = info.myFontSamplerInfo;

				vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}
			break;
		default:
			Assert(false, "Unsupported Descriptor Type");
			break;
		}
	}
}
