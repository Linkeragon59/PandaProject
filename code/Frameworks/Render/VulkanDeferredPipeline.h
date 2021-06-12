#pragma once

#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render
{
namespace glTF
{
	class Model;
}
namespace Vulkan
{
	class DummyModel;

	struct DeferredPipeline
	{
		DeferredPipeline();

		void Prepare(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat);
		void Update();
		void Destroy();

		static void SetupDescriptorSetLayouts();
		static void DestroyDescriptorSetLayouts();

		static VkDescriptorSetLayout ourLightingDescriptorSetLayout;
		static VkDescriptorSetLayout ourTransparentDescriptorSetLayout;

		static VkDescriptorSetLayout ourCameraDescriptorSetLayout;
		static VkDescriptorSetLayout ourObjectDescriptorSetLayout;

		VkDevice myDevice = VK_NULL_HANDLE;

		enum class VertexComponent {
			Position,
			Normal,
			UV,
			Color,
			Joint,
			Weight,
			Tangent
		};

		struct Vertex
		{
			glm::vec3 myPosition;
			glm::vec3 myNormal;
			glm::vec2 myUV;
			glm::vec4 myColor;
			glm::vec4 myJoint;
			glm::vec4 myWeight;
			glm::vec4 myTangent;

			static VkVertexInputBindingDescription GetBindingDescription(uint aBinding = 0);
			static VkVertexInputAttributeDescription GetAttributeDescription(VertexComponent aComponent, uint aLocation = 0, uint aBinding = 0);
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<VertexComponent> someComponents, uint aBinding = 0);
		};

		struct ViewProjData
		{
			glm::mat4 myProjection;
			glm::mat4 myView;
		};
		struct ModelData
		{
			glm::mat4 myModel;
		};

		static const uint ourNumLights = 64;
		struct Light
		{
			glm::vec4 myPosition;
			glm::vec3 myColor;
			float myRadius;
		};
		struct LightData
		{
			glm::vec4 myViewPos;
			Light myLights[ourNumLights];
		};

		LightData myLightsData;

		// Per Scene UBOs
		Buffer myLightsUBO;

		Image myPositionAttachement;
		Image myNormalAttachement;
		Image myAlbedoAttachement;

		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipeline myTransparentPipeline = VK_NULL_HANDLE;

		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myTransparentPipelineLayout = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightingDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet myTransparentDescriptorSet = VK_NULL_HANDLE;

	private:
		void PrepareUBOs();

		//void UpdateViewProjUBO();
		void UpdateLightsUBO();
		void SetupRandomLights();

		void SetupRenderPass(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat);
		void DestroyRenderPass();

		void SetupDescriptorPool();
		void SetupDescriptorSets();

		void SetupGBufferPipeline();
		void DestroyGBufferPipeline();

		void SetupLightingPipeline();
		void DestroyLightingPipeline();

		void SetupTransparentPipeline();
		void DestroyTransparentPipeline();
	};
}
}
