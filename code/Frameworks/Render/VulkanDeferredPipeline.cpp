#include "VulkanDeferredPipeline.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanCamera.h"

#include "VulkanglTFModel.h"
#include "DummyModel.h"

#include <random>

namespace Render
{
namespace Vulkan
{
	VkDescriptorSetLayout DeferredPipeline::ourLightingDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout DeferredPipeline::ourTransparentDescriptorSetLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout DeferredPipeline::ourCameraDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout DeferredPipeline::ourObjectDescriptorSetLayout = VK_NULL_HANDLE;

	void DeferredPipeline::SetupDescriptorSetLayouts()
	{
		VkDevice device = Renderer::GetInstance()->GetDevice();

		// Lighting
		{
			std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
			// Binding 0 : Position attachment
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 1 : Normal attachment
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[1].descriptorCount = 1;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 2 : Albedo attachment
			bindings[2].binding = 2;
			bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[2].descriptorCount = 1;
			bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// Binding 3 : Lights
			bindings[3].binding = 3;
			bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[3].descriptorCount = 1;
			bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
			descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayoutInfo.bindingCount = (uint)bindings.size();
			descriptorLayoutInfo.pBindings = bindings.data();
			VK_CHECK_RESULT(
				vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &ourLightingDescriptorSetLayout),
				"Failed to create the Lighting descriptor set layout");
		}

		// Transparent
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
			// Binding 0 : Position attachment
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			bindings[0].descriptorCount = 1;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
			descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayoutInfo.bindingCount = (uint)bindings.size();
			descriptorLayoutInfo.pBindings = bindings.data();
			VK_CHECK_RESULT(
				vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &ourTransparentDescriptorSetLayout),
				"Failed to create the Transparent DescriptorSetLayout");
		}

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
				vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &ourCameraDescriptorSetLayout),
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
				vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &ourObjectDescriptorSetLayout),
				"Failed to create the Object DescriptorSetLayout");
		}
	}

	void DeferredPipeline::DestroyDescriptorSetLayouts()
	{
		VkDevice device = Renderer::GetInstance()->GetDevice();

		vkDestroyDescriptorSetLayout(device, ourLightingDescriptorSetLayout, nullptr);
		ourLightingDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(device, ourTransparentDescriptorSetLayout, nullptr);
		ourTransparentDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(device, ourCameraDescriptorSetLayout, nullptr);
		ourCameraDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(device, ourObjectDescriptorSetLayout, nullptr);
		ourObjectDescriptorSetLayout = VK_NULL_HANDLE;
	}

	VkVertexInputBindingDescription DeferredPipeline::Vertex::GetBindingDescription(uint aBinding /*= 0*/)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = aBinding;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkVertexInputAttributeDescription DeferredPipeline::Vertex::GetAttributeDescription(DeferredPipeline::VertexComponent aComponent, uint aLocation /*= 0*/, uint aBinding /*= 0*/)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = aLocation;
		attributeDescription.binding = aBinding;
		switch (aComponent)
		{
		case DeferredPipeline::VertexComponent::Position:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myPosition);
			break;
		case DeferredPipeline::VertexComponent::Normal:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myNormal);
			break;
		case DeferredPipeline::VertexComponent::UV:
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myUV);
			break;
		case DeferredPipeline::VertexComponent::Color:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myColor);
			break;
		case DeferredPipeline::VertexComponent::Joint:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myJoint);
			break;
		case DeferredPipeline::VertexComponent::Weight:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myWeight);
			break;
		case DeferredPipeline::VertexComponent::Tangent:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myTangent);
			break;
		}
		return attributeDescription;
	}

	std::vector<VkVertexInputAttributeDescription> DeferredPipeline::Vertex::GetAttributeDescriptions(const std::vector<DeferredPipeline::VertexComponent> someComponents, uint aBinding /*= 0*/)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.reserve(someComponents.size());
		for (uint i = 0; i < (uint)someComponents.size(); ++i)
			attributeDescriptions.push_back(GetAttributeDescription(someComponents[i], i, aBinding));
		return attributeDescriptions;
	}

	DeferredPipeline::DeferredPipeline()
	{
		myDevice = Renderer::GetInstance()->GetDevice();
	}

	void DeferredPipeline::Prepare(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat)
	{
		PrepareUBOs();

		SetupRenderPass(anExtent, aColorFormat, aDepthFormat);

		SetupDescriptorPool();
		SetupDescriptorSets();

		SetupGBufferPipeline();
		SetupLightingPipeline();
		SetupTransparentPipeline();
	}

	void DeferredPipeline::Update()
	{
		UpdateLightsUBO();
	}

	void DeferredPipeline::Destroy()
	{
		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		DestroyGBufferPipeline();
		DestroyLightingPipeline();
		DestroyTransparentPipeline();

		DestroyRenderPass();

		myLightsUBO.Destroy();
	}

	void DeferredPipeline::PrepareUBOs()
	{
		myLightsUBO.Create(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO.SetupDescriptor();
		myLightsUBO.Map();

		SetupRandomLights();
		UpdateLightsUBO();
	}

	void DeferredPipeline::UpdateLightsUBO()
	{
		glm::vec3 cameraPosition = Renderer::GetInstance()->GetCamera()->GetView()[3];
		myLightsData.myViewPos = glm::vec4(cameraPosition, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
		memcpy(myLightsUBO.myMappedData, &myLightsData, sizeof(LightData));
	}

	void DeferredPipeline::SetupRandomLights()
	{
		std::vector<glm::vec3> colors =
		{
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 0.0f),
		};

		std::default_random_engine rndGen((unsigned int)time(nullptr));
		std::uniform_real_distribution<float> rndDist(-10.0f, 10.0f);
		std::uniform_int_distribution<uint> rndCol(0, static_cast<uint>(colors.size() - 1));

		for (Light& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(rndDist(rndGen) * 6.0f, 0.25f + std::abs(rndDist(rndGen)) * 4.0f, rndDist(rndGen) * 6.0f, 1.0f);
			light.myColor = colors[rndCol(rndGen)];
			light.myRadius = 1.0f + std::abs(rndDist(rndGen));
		}
	}

	void DeferredPipeline::SetupRenderPass(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat)
	{
		myPositionAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myPositionAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myPositionAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myNormalAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myNormalAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myNormalAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myAlbedoAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myAlbedoAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myAlbedoAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		std::array<VkAttachmentDescription, 5> attachments{};
		{
			// Color attachment
			attachments[0].format = aColorFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Deferred attachments
			// Position
			attachments[1].format = myPositionAttachement.myFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Normals
			attachments[2].format = myNormalAttachement.myFormat;
			attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Albedo
			attachments[3].format = myAlbedoAttachement.myFormat;
			attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// Depth attachment
			attachments[4].format = aDepthFormat;
			attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		// Three subpasses
		VkAttachmentReference colorReferences[4];
		colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		
		VkAttachmentReference depthReference  = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkAttachmentReference inputReferences[3];
		inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		
		std::array<VkSubpassDescription, 3> subpassDescriptions{};
		{
			// First subpass: Fill G-Buffer components
			subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[0].colorAttachmentCount = 4;
			subpassDescriptions[0].pColorAttachments = colorReferences;
			subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

			// Second subpass: Final Lighting (using G-Buffer components)
			subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[1].colorAttachmentCount = 1;
			subpassDescriptions[1].pColorAttachments = colorReferences;
			subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
			subpassDescriptions[1].inputAttachmentCount = 3;
			subpassDescriptions[1].pInputAttachments = inputReferences;

			// Third subpass: Forward transparency
			subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[2].colorAttachmentCount = 1;
			subpassDescriptions[2].pColorAttachments = colorReferences;
			subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
			subpassDescriptions[2].inputAttachmentCount = 1;
			subpassDescriptions[2].pInputAttachments = inputReferences;
		}

		// TODO: Understand subpass dependencies better!
		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 3> dependencies{};
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = 0;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// This dependency transitions the input attachment from color attachment to shader read
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = 1;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[2].srcSubpass = 1;
			dependencies[2].dstSubpass = 2;
			dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass), "Failed to create the render pass!");
	}

	void DeferredPipeline::DestroyRenderPass()
	{
		vkDestroyRenderPass(myDevice, myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;

		myPositionAttachement.Destroy();
		myNormalAttachement.Destroy();
		myAlbedoAttachement.Destroy();
	}

	void DeferredPipeline::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1; //  1 - Lighting
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSizes[1].descriptorCount = 4; //  3 - Lighting, 1 - Transparent

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2; // Lighting, Transparent

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void DeferredPipeline::SetupDescriptorSets()
	{
		// Lighting
		{
			std::array<VkDescriptorSetLayout, 1> layouts = { ourLightingDescriptorSetLayout };
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = layouts.data();
			descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
			VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myLightingDescriptorSet), "Failed to create the node descriptor set");

			std::array<VkWriteDescriptorSet, 4> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pImageInfo = &myPositionAttachement.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].pImageInfo = &myNormalAttachement.myDescriptor;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[2].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[2].dstBinding = 2;
			writeDescriptorSets[2].pImageInfo = &myAlbedoAttachement.myDescriptor;
			writeDescriptorSets[2].descriptorCount = 1;
			writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[3].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[3].dstBinding = 3;
			writeDescriptorSets[3].pBufferInfo = &myLightsUBO.myDescriptor;
			writeDescriptorSets[3].descriptorCount = 1;
			vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}

		// Transparent
		{
			std::array<VkDescriptorSetLayout, 1> layouts = { ourTransparentDescriptorSetLayout };
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = layouts.data();
			descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
			VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myTransparentDescriptorSet), "Failed to create the node descriptor set");

			std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = myTransparentDescriptorSet;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pImageInfo = &myPositionAttachement.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}
	}

	void DeferredPipeline::SetupGBufferPipeline()
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			ourCameraDescriptorSetLayout,
			ourObjectDescriptorSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myGBufferPipelineLayout),
			"Failed to create the GBuffer pipeline layout");

		VkShaderModule vertModule = CreateShaderModule("Frameworks/shaders/gbuffer_vert.spv");
		VkShaderModule fragModule = CreateShaderModule("Frameworks/shaders/gbuffer_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions({
			VertexComponent::Position,
			VertexComponent::Normal,
			VertexComponent::UV,
			VertexComponent::Color,
			VertexComponent::Joint,
			VertexComponent::Weight
		});

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates{};
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_FALSE;
		blendAttachmentStates[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[1].blendEnable = VK_FALSE;
		blendAttachmentStates[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[2].blendEnable = VK_FALSE;
		blendAttachmentStates[3].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[3].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myGBufferPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGBufferPipeline),
			"Failed to create the GBuffer pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}

	void DeferredPipeline::DestroyGBufferPipeline()
	{
		vkDestroyPipeline(myDevice, myGBufferPipeline, nullptr);
		myGBufferPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(myDevice, myGBufferPipelineLayout, nullptr);
		myGBufferPipelineLayout = VK_NULL_HANDLE;
	}

	void DeferredPipeline::SetupLightingPipeline()
	{
		std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts = {
			ourLightingDescriptorSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myLightingPipelineLayout),
			"Failed to create the Lighting pipeline layout");

		VkShaderModule vertModule = CreateShaderModule("Frameworks/shaders/composition_vert.spv");
		VkShaderModule fragModule = CreateShaderModule("Frameworks/shaders/composition_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";
		// TODO: Use specialization constants to pass number of lights to the shader
		//VkSpecializationInfo
		//shaderStages[1].pSpecializationInfo

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_NONE;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates{};
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myLightingPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myLightingPipeline),
			"Failed to create the Lighting pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}

	void DeferredPipeline::DestroyLightingPipeline()
	{
		vkDestroyPipeline(myDevice, myLightingPipeline, nullptr);
		myLightingPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(myDevice, myLightingPipelineLayout, nullptr);
		myLightingPipelineLayout = VK_NULL_HANDLE;
	}

	void DeferredPipeline::SetupTransparentPipeline()
	{
		std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = {
			ourTransparentDescriptorSetLayout,
			ourCameraDescriptorSetLayout,
			ourObjectDescriptorSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myTransparentPipelineLayout),
			"Failed to create the Transparent pipeline layout");

		VkShaderModule vertModule = CreateShaderModule("Frameworks/shaders/transparent_vert.spv");
		VkShaderModule fragModule = CreateShaderModule("Frameworks/shaders/transparent_frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertModule;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragModule;
		shaderStages[1].pName = "main";

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions({
			VertexComponent::Position,
			VertexComponent::UV,
			VertexComponent::Color
		});

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_NONE;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates{};
		blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentStates[0].blendEnable = VK_TRUE;
		blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = (uint)blendAttachmentStates.size();
		colorBlendState.pAttachments = blendAttachmentStates.data();

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myTransparentPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 2;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myTransparentPipeline),
			"Failed to create the transparent pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}

	void DeferredPipeline::DestroyTransparentPipeline()
	{
		vkDestroyPipeline(myDevice, myTransparentPipeline, nullptr);
		myTransparentPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(myDevice, myTransparentPipelineLayout, nullptr);
		myTransparentPipelineLayout = VK_NULL_HANDLE;
	}
}
}
