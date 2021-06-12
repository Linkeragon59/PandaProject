#pragma once

#include "VulkanBuffer.h"

namespace Render
{
namespace Vulkan
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		const glm::mat4& GetView() const { return myView; }
		const glm::mat4& GetProjection() const { return myProjection; }

		void Update(const glm::mat4& aView, const glm::mat4& aProjection);
		void BindViewProj(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aSetIndex);
	private:
		glm::mat4 myView;
		glm::mat4 myProjection;
		Buffer myViewProjUBO;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
}