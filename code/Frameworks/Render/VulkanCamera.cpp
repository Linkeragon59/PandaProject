#include "VulkanCamera.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include "Input.h"

namespace Render
{
	VkDescriptorSetLayout VulkanCamera::ourDescriptorSetLayout = VK_NULL_HANDLE;

	void VulkanCamera::SetupDescriptorSetLayout()
	{
		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding uboProjBinding{};
		uboProjBinding.binding = 0;
		uboProjBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboProjBinding.descriptorCount = 1;
		uboProjBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboProjBinding };

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
		descriptorLayoutInfo.pBindings = bindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), &descriptorLayoutInfo, nullptr, &ourDescriptorSetLayout),
			"Failed to create the per object descriptor set layout");
	}

	void VulkanCamera::DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), ourDescriptorSetLayout, nullptr);
		ourDescriptorSetLayout = VK_NULL_HANDLE;
	}

	void VulkanCamera::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		if (inputManager->PollRawInput(Input::RawInput::KeyW) == Input::RawInputState::Pressed)
			myPosition += myDirection * 0.1f;
		else if (inputManager->PollRawInput(Input::RawInput::KeyS) == Input::RawInputState::Pressed)
			myPosition -= myDirection * 0.1f;

		UBO ubo{};
		ubo.myView = glm::lookAt(myPosition, myPosition + myDirection, glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.myProj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 10.0f);
		ubo.myProj[1][1] *= -1; // adapt calculation for Vulkan

		memcpy(myUBO.myMappedData, &ubo, sizeof(ubo));
	}

	VulkanCamera::VulkanCamera()
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();

		SetupDescriptorPool();
		PrepareUniformBuffers();
		SetupDescriptorSet();
	}

	VulkanCamera::~VulkanCamera()
	{
		myDescriptorSet = VK_NULL_HANDLE;
		myUBO.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void VulkanCamera::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanCamera::PrepareUniformBuffers()
	{
		myUBO.Create(sizeof(UBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO.SetupDescriptor();

		// Persistent map
		myUBO.Map();

		myPosition = glm::vec3(-5.0f, 0.0f, 4.0f);
		myDirection = glm::normalize(glm::vec3(1.0f, 0.0f, -0.7f));

		Update();
	}

	void VulkanCamera::SetupDescriptorSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { ourDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor set");

		// Binding 0 : Vertex shader uniform buffer
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myUBO.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
}
