#pragma once

#include "VulkanBuffer.h"

namespace Render
{
	class VulkanCamera
	{
	public:
		VulkanCamera();
		~VulkanCamera();

		void Update();

		VkDescriptorSet GetDummyDescriptorSet() const { return myDummyDescriptorSet; }
		VkDescriptorSet GetglTFDescriptorSet() const { return myglTFDescriptorSet; }

	private:		
		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDummyDescriptorSet();
		void SetupglTFDescriptorSet();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		VulkanBuffer myDummyUBO;
		VkDescriptorSet myDummyDescriptorSet = VK_NULL_HANDLE;
		
		VulkanBuffer myglTFUBO;
		VkDescriptorSet myglTFDescriptorSet = VK_NULL_HANDLE;

	public:
		void SetPosition(const glm::vec3& aPosition);
		void Translate(const glm::vec3& aPositionDelta);
		void SetRotation(const glm::vec3& aRotation);
		void Rotate(const glm::vec3& aRotationDelta);
		void UpdateViewMatrix();

		void SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar);
		void UpdatePerspectiveMatrix();

	private:
		glm::vec3 myPosition = glm::vec3();
		glm::vec3 myRotation = glm::vec3();

		float myAspectRatio = 1.0f;
		float myFov = 60.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		glm::mat4 myView;
		glm::mat4 myPerspective;
	};
}
