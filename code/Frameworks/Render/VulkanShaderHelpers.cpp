#include "VulkanShaderHelpers.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include "File.h"

namespace Render
{
namespace Vulkan
{
namespace ShaderHelpers
{
	VkDescriptorSetLayout locCameraDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout locObjectDescriptorSetLayout = VK_NULL_HANDLE;

	void SetupDescriptorSetLayouts()
	{
		// Camera
		{
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
				vkCreateDescriptorSetLayout(Renderer::GetInstance()->GetDevice(), &descriptorLayoutInfo, nullptr, &locCameraDescriptorSetLayout),
				"Failed to create the Camera DescriptorSetLayout");
		}

		// Object
		{
			std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
			// Binding 0 : Model
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			// Binding 1 : Joint Matrices
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			// Binding 2 : Texture Sampler
			bindings[2].binding = 2;
			bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[2].descriptorCount = 1;
			bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 3 : Material
			bindings[3].binding = 3;
			bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[3].descriptorCount = 1;
			bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
			descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayoutInfo.bindingCount = (uint)bindings.size();
			descriptorLayoutInfo.pBindings = bindings.data();
			VK_CHECK_RESULT(
				vkCreateDescriptorSetLayout(Renderer::GetInstance()->GetDevice(), &descriptorLayoutInfo, nullptr, &locObjectDescriptorSetLayout),
				"Failed to create the Object DescriptorSetLayout");
		}
	}

	void DestroyDescriptorSetLayouts()
	{
		vkDestroyDescriptorSetLayout(Renderer::GetInstance()->GetDevice(), locCameraDescriptorSetLayout, nullptr);
		locCameraDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(Renderer::GetInstance()->GetDevice(), locObjectDescriptorSetLayout, nullptr);
		locObjectDescriptorSetLayout = VK_NULL_HANDLE;
	}

	VkDescriptorSetLayout GetCameraDescriptorSetLayout()
	{
		return locCameraDescriptorSetLayout;
	}

	VkDescriptorSetLayout GetObjectDescriptorSetLayout()
	{
		return locObjectDescriptorSetLayout;
	}

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
		Verify(File::ReadAsBuffer(aFilename, shaderCode), "Couldn't read shader file: %s", aFilename.c_str());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint*>(shaderCode.data());

		VK_CHECK_RESULT(vkCreateShaderModule(Renderer::GetInstance()->GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create a module shader!");

		return shaderModule;
	}
}
}
}
