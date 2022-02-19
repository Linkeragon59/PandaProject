#pragma once

#include "Render_VulkanBuffer.h"

namespace Render
{
	class Camera
	{
	public:
		Camera();

		const glm::mat4& GetView() const { return myView; }
		const glm::mat4& GetProjection() const { return myProjection; }

		void Update(const glm::mat4& aView, const glm::mat4& aProjection);
		void BindViewProj(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aSetIndex);

	private:
		glm::mat4 myView = glm::mat4(1.0f);
		glm::mat4 myProjection = glm::mat4(1.0f);
		VulkanBufferPtr myViewProjUBO;
	};
}
