#include "VulkanDeferredPipeline.h"

#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanCamera.h"

#include "VulkanglTFModel.h"
#include "DummyModel.h"

#include <random>

namespace Render
{
namespace Vulkan
{
	DeferredPipeline::DeferredPipeline()
	{
		myDevice = Renderer::GetInstance()->GetDevice();
	}

	void DeferredPipeline::Prepare(const DeferredRenderPass& aRenderPass)
	{
		PrepareUBOs();

		SetupDescriptorPool();
		SetupDescriptorSetLayouts();
		SetupDescriptorSets(aRenderPass);

		SetupGBufferPipeline(aRenderPass);
		SetupLightingPipeline(aRenderPass);
		SetupTransparentPipeline(aRenderPass);
	}

	void DeferredPipeline::Update()
	{
		UpdateLightsUBO();
	}

	void DeferredPipeline::Destroy()
	{
		vkDestroyDescriptorSetLayout(myDevice, myLightingDescriptorSetLayout, nullptr);
		myLightingDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(myDevice, myTransparentDescriptorSetLayout, nullptr);
		myTransparentDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		DestroyGBufferPipeline();
		DestroyLightingPipeline();
		DestroyTransparentPipeline();

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

		std::default_random_engine rndGen((uint)time(nullptr));
		std::uniform_real_distribution<float> rndDist(-10.0f, 10.0f);
		std::uniform_int_distribution<uint> rndCol(0, static_cast<uint>(colors.size() - 1));

		for (Light& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(rndDist(rndGen) * 6.0f, 0.25f + std::abs(rndDist(rndGen)) * 4.0f, rndDist(rndGen) * 6.0f, 1.0f);
			light.myColor = colors[rndCol(rndGen)];
			light.myRadius = 1.0f + std::abs(rndDist(rndGen));
		}
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

	void DeferredPipeline::SetupDescriptorSetLayouts()
	{
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
				vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myLightingDescriptorSetLayout),
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
				vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myTransparentDescriptorSetLayout),
				"Failed to create the Transparent DescriptorSetLayout");
		}
	}

	void DeferredPipeline::SetupDescriptorSets(const DeferredRenderPass& aRenderPass)
	{
		// Lighting
		{
			std::array<VkDescriptorSetLayout, 1> layouts = { myLightingDescriptorSetLayout };
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
			writeDescriptorSets[0].pImageInfo = &aRenderPass.myPositionAttachement.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].pImageInfo = &aRenderPass.myNormalAttachement.myDescriptor;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[2].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[2].dstBinding = 2;
			writeDescriptorSets[2].pImageInfo = &aRenderPass.myAlbedoAttachement.myDescriptor;
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
			std::array<VkDescriptorSetLayout, 1> layouts = { myTransparentDescriptorSetLayout };
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
			writeDescriptorSets[0].pImageInfo = &aRenderPass.myPositionAttachement.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}
	}

	void DeferredPipeline::SetupGBufferPipeline(const DeferredRenderPass& aRenderPass)
	{
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			ShaderHelpers::GetCameraDescriptorSetLayout(),
			ShaderHelpers::GetObjectDescriptorSetLayout()
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myGBufferPipelineLayout),
			"Failed to create the GBuffer pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/gbuffer_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/gbuffer_frag.spv");

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

		auto bindingDescription = ShaderHelpers::Vertex::GetBindingDescription();
		auto attributeDescriptions = ShaderHelpers::Vertex::GetAttributeDescriptions({
			ShaderHelpers::VertexComponent::Position,
			ShaderHelpers::VertexComponent::Normal,
			ShaderHelpers::VertexComponent::UV,
			ShaderHelpers::VertexComponent::Color,
			ShaderHelpers::VertexComponent::Joint,
			ShaderHelpers::VertexComponent::Weight
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
		pipelineInfo.renderPass = aRenderPass.myRenderPass;
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

	void DeferredPipeline::SetupLightingPipeline(const DeferredRenderPass& aRenderPass)
	{
		std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts = {
			myLightingDescriptorSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myLightingPipelineLayout),
			"Failed to create the Lighting pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/composition_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/composition_frag.spv");

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
		pipelineInfo.renderPass = aRenderPass.myRenderPass;
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

	void DeferredPipeline::SetupTransparentPipeline(const DeferredRenderPass& aRenderPass)
	{
		std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = {
			myTransparentDescriptorSetLayout,
			ShaderHelpers::GetCameraDescriptorSetLayout(),
			ShaderHelpers::GetObjectDescriptorSetLayout()
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myTransparentPipelineLayout),
			"Failed to create the Transparent pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/transparent_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/transparent_frag.spv");

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

		auto bindingDescription = ShaderHelpers::Vertex::GetBindingDescription();
		auto attributeDescriptions = ShaderHelpers::Vertex::GetAttributeDescriptions({
			ShaderHelpers::VertexComponent::Position,
			ShaderHelpers::VertexComponent::UV,
			ShaderHelpers::VertexComponent::Color
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
		pipelineInfo.renderPass = aRenderPass.myRenderPass;
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
