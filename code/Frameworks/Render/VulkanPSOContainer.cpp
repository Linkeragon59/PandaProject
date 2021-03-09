#include "VulkanPSOContainer.h"

#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanModel.h"

namespace Render
{
	VulkanPSOContainer::VulkanPSOContainer(VkRenderPass aRenderPass)
		: myRenderPass(aRenderPass)
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();

		SetupDescriptorSetLayout();

		CreatePipelineCache();
		PreparePipelines();
	}

	VulkanPSOContainer::~VulkanPSOContainer()
	{
		if (myDefaultPipeline)
		{
			vkDestroyPipeline(myDevice, myDefaultPipeline, nullptr);
			myDefaultPipeline = VK_NULL_HANDLE;
		}

		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;

		vkDestroyPipelineCache(myDevice, myPipelineCache, nullptr);
		myPipelineCache = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(myDevice, myDescriptorSetLayout, nullptr);
		myDescriptorSetLayout = VK_NULL_HANDLE;
	}

	void VulkanPSOContainer::SetupDescriptorSetLayout()
	{
		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding ubolayoutBinding{};
		ubolayoutBinding.binding = 0;
		ubolayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubolayoutBinding.descriptorCount = 1;
		ubolayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// Binding 1 : Fragment shader sampler
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings = { ubolayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = (uint32_t)layoutBindings.size();
		descriptorLayoutInfo.pBindings = layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myDescriptorSetLayout), "Failed to create the descriptor set layout");
	}

	void VulkanPSOContainer::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.initialDataSize = 0;

		VK_CHECK_RESULT(vkCreatePipelineCache(myDevice, &pipelineCacheCreateInfo, nullptr, &myPipelineCache), "Failed to create the pipeline cache!");
	}

	void VulkanPSOContainer::PreparePipelines()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &myDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myPipelineLayout), "Failed to create the pipeline layout");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";

		auto bindingDescription = VulkanModel::Vertex::GetBindingDescription();
		auto attributeDescriptions = VulkanModel::Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
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

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
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
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		// Create the graphics pipeline state objects

		// Default shading pipeline
		VkShaderModule defaultVert = CreateShaderModule("Frameworks/shaders/basicRenderer_vert.spv");
		shaderStages[0].module = defaultVert;
		VkShaderModule defaultFrag = CreateShaderModule("Frameworks/shaders/basicRenderer_frag.spv");
		shaderStages[1].module = defaultFrag;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myDefaultPipeline), "Failed to create the default pipeline");

		vkDestroyShaderModule(myDevice, defaultVert, nullptr);
		vkDestroyShaderModule(myDevice, defaultFrag, nullptr);
	}
}
