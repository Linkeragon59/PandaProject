#include "Render_ShaderHelpers.h"

#include "Render_DescriptorContainer.h"

#include "GameCore_File.h"

namespace Render::ShaderHelpers
{
	VkVertexInputBindingDescription Vertex::GetBindingDescription(uint aBinding /*= 0*/)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = aBinding;
		bindingDescription.stride = sizeof(ShaderHelpers::Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkVertexInputAttributeDescription Vertex::GetAttributeDescription(VertexComponent aComponent, uint aLocation /*= 0*/, uint aBinding /*= 0*/)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = aLocation;
		attributeDescription.binding = aBinding;
		switch (aComponent)
		{
		case VertexComponent::Position:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myPosition);
			break;
		case VertexComponent::Normal:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myNormal);
			break;
		case VertexComponent::UV:
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myUV);
			break;
		case VertexComponent::Color:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myColor);
			break;
		case VertexComponent::Joint:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myJoint);
			break;
		case VertexComponent::Weight:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myWeight);
			break;
		case VertexComponent::Tangent:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myTangent);
			break;
		}
		return attributeDescription;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions(const std::vector<VertexComponent> someComponents, uint aBinding /*= 0*/)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.reserve(someComponents.size());
		for (uint i = 0; i < (uint)someComponents.size(); ++i)
			attributeDescriptions.push_back(GetAttributeDescription(someComponents[i], i, aBinding));
		return attributeDescriptions;
	}

	VkShaderModule CreateShaderModule(const std::string& aFilename)
	{
		VkShaderModule shaderModule;

		std::vector<char> shaderCode;
		Verify(FileHelpers::ReadAsBuffer(aFilename, shaderCode), "Couldn't read shader file: %s", aFilename.c_str());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint*>(shaderCode.data());

		VK_CHECK_RESULT(vkCreateShaderModule(RenderModule::GetInstance()->GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create a module shader!");

		return shaderModule;
	}
}
