#pragma once

namespace Render::Vulkan::ShaderHelpers
{
	void SetupDescriptorSetLayouts();
	void DestroyDescriptorSetLayouts();
	VkDescriptorSetLayout GetCameraDescriptorSetLayout();
	VkDescriptorSetLayout GetObjectDescriptorSetLayout();
	VkDescriptorSetLayout GetLightsDescriptorSetLayout(); // Temp

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

	struct NearFarData
	{
		glm::vec2 myPlanes;
	};

	struct ModelData
	{
		glm::mat4 myModel;
	};

	VkShaderModule CreateShaderModule(const std::string& aFilename);
}
