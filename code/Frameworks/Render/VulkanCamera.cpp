#include "VulkanCamera.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanRender.h"

namespace Render::Vulkan
{
	Camera::Camera()
	{
		myViewProjUBO.Create(sizeof(ShaderHelpers::ViewProjMatricesData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myViewProjUBO.SetupDescriptor();
		myViewProjUBO.Map();

		Update(glm::mat4(1.0f), glm::mat4(1.0f));
	}

	Camera::~Camera()
	{
		myViewProjUBO.Destroy();
	}

	void Camera::Update(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myView = aView;
		myProjection = aProjection;

		ShaderHelpers::ViewProjMatricesData viewProjData;
		viewProjData.myView = myView;
		viewProjData.myProjection = myProjection;
		memcpy(myViewProjUBO.myMappedData, &viewProjData, sizeof(ShaderHelpers::ViewProjMatricesData));
	}

	void Camera::BindViewProj(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aSetIndex)
	{
		ShaderHelpers::CameraDescriptorInfo info;
		info.myViewProjMatricesInfo = &myViewProjUBO.myDescriptor;
		VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(ShaderHelpers::BindType::Camera, info);

		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aSetIndex, 1, &descriptorSet, 0, NULL);
	}
}
