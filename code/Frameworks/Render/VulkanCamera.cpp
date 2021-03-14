#include "VulkanCamera.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include "DummyVulkanPSO.h"
#include "glTFVulkanPSO.h"

#include "Input.h"

namespace Render
{
	void VulkanCamera::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();

		double mouseX, mouseY;
		inputManager->PollMousePosition(mouseX, mouseY);

		static double prevMouseX = mouseX;
		static double prevMouseY = mouseY;

		double deltaMouseX = prevMouseX - mouseX;
		double deltaMouseY = prevMouseY - mouseY;

		if (inputManager->PollRawInput(Input::RawInput::MouseLeft) == Input::RawInputState::Pressed)
		{
			Rotate(glm::vec3(deltaMouseY, -deltaMouseX, 0.0f));
		}
		if (inputManager->PollRawInput(Input::RawInput::MouseMiddle) == Input::RawInputState::Pressed)
		{
			Translate(glm::vec3(-deltaMouseX * 0.01f, -deltaMouseY * 0.01f, 0.0f));
		}

		prevMouseX = mouseX;
		prevMouseY = mouseY;
		/*if (inputManager->PollRawInput(Input::RawInput::KeyW) == Input::RawInputState::Pressed)
			myPosition += myDirection * 0.1f;
		else if (inputManager->PollRawInput(Input::RawInput::KeyS) == Input::RawInputState::Pressed)
			myPosition -= myDirection * 0.1f;

		glm::mat4 view = glm::lookAt(myPosition, myPosition + myDirection, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 10.0f);
		proj[1][1] *= -1; // adapt calculation for Vulkan*/

		DummyVulkanPSO::PerFrameUBO dummyUBO;
		dummyUBO.myView = myView;
		dummyUBO.myProj = myPerspective;
		memcpy(myDummyUBO.myMappedData, &dummyUBO, sizeof(dummyUBO));

		glTF::VulkanPSO::PerFrameUBO glTFUBO;
		glTFUBO.myView = myView;
		glTFUBO.myProj = myPerspective;
		memcpy(myglTFUBO.myMappedData, &glTFUBO, sizeof(glTFUBO));
	}

	VulkanCamera::VulkanCamera()
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();

		SetupDescriptorPool();
		PrepareUniformBuffers();
		SetupDummyDescriptorSet();
		SetupglTFDescriptorSet();

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->AddScrollInputCallback([this](double aX, double aY) {
			(void)aX;
			Translate(glm::vec3(0.0f, 0.0f, (float)aY * 0.3f));
		});
	}

	VulkanCamera::~VulkanCamera()
	{
		myDummyDescriptorSet = VK_NULL_HANDLE;
		myDummyUBO.Destroy();

		myglTFDescriptorSet = VK_NULL_HANDLE;
		myglTFUBO.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void VulkanCamera::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 2;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanCamera::PrepareUniformBuffers()
	{
		myDummyUBO.Create(sizeof(DummyVulkanPSO::PerFrameUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDummyUBO.SetupDescriptor();
		myDummyUBO.Map();

		myglTFUBO.Create(sizeof(glTF::VulkanPSO::PerFrameUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myglTFUBO.SetupDescriptor();
		myglTFUBO.Map();

		Update();
	}

	void VulkanCamera::SetupDummyDescriptorSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { DummyVulkanPSO::ourPerFrameDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDummyDescriptorSet), "Failed to create the descriptor set");

		// Binding 0 : Vertex shader uniform buffer
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myDummyDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myDummyUBO.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VulkanCamera::SetupglTFDescriptorSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { glTF::VulkanPSO::ourPerFrameDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myglTFDescriptorSet), "Failed to create the descriptor set");

		// Binding 0 : Vertex shader uniform buffer
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myglTFDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myglTFUBO.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VulkanCamera::SetPosition(const glm::vec3& aPosition)
	{
		myPosition = aPosition;
		UpdateViewMatrix();
	}

	void VulkanCamera::Translate(const glm::vec3& aPositionDelta)
	{
		myPosition += aPositionDelta;
		UpdateViewMatrix();
	}

	void VulkanCamera::SetRotation(const glm::vec3& aRotation)
	{
		myRotation = aRotation;
		UpdateViewMatrix();
	}

	void VulkanCamera::Rotate(const glm::vec3& aRotationDelta)
	{
		myRotation += aRotationDelta;
		UpdateViewMatrix();
	}

	void VulkanCamera::UpdateViewMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(-myRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = myPosition;
		translation.y *= -1.0f;
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

		myView = translationMatrix * rotationMatrix;
	}

	void VulkanCamera::SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar)
	{
		myAspectRatio = anAspectRatio;
		myFov = aFov;
		myZNear = aZNear;
		myZFar = aZFar;
		UpdatePerspectiveMatrix();
	}

	void VulkanCamera::UpdatePerspectiveMatrix()
	{
		myPerspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
		myPerspective[1][1] *= -1.0f;
	}
}
