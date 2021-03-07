#include "VulkanPSOContainer.h"

#include "VulkanRenderCore.h"

namespace Render
{
	VulkanPSOContainer::VulkanPSOContainer(VkRenderPass aRenderPass)
		: myRenderPass(aRenderPass)
	{
		myDevice = VulkanRenderCore::GetInstance()->GetDevice();

		SetupDescriptorPool();
		SetupDescriptorSetLayout();
		CreatePipelineCache();
		PreparePipelines();
	}

	VulkanPSOContainer::~VulkanPSOContainer()
	{
		if (myPhongPipeline)
		{
			vkDestroyPipeline(myDevice, myPhongPipeline, nullptr);
			myPhongPipeline = VK_NULL_HANDLE;
		}
		if (myToonPipeline)
		{
			vkDestroyPipeline(myDevice, myToonPipeline, nullptr);
			myToonPipeline = VK_NULL_HANDLE;
		}
		if (myWireFramePipeline)
		{
			vkDestroyPipeline(myDevice, myWireFramePipeline, nullptr);
			myWireFramePipeline = VK_NULL_HANDLE;
		}

		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;

		vkDestroyPipelineCache(myDevice, myPipelineCache, nullptr);
		myPipelineCache = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(myDevice, myDescriptorSetLayout, nullptr);
		myDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void VulkanPSOContainer::SetupDescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = 1;
		std::vector<VkDescriptorPoolSize> poolSizes = { descriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2; // TODO: Understand this

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanPSOContainer::SetupDescriptorSetLayout()
	{
		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.binding = 0;
		layoutBinding.descriptorCount = 1;

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { layoutBinding };

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

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Color });
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

		// Phong shading pipeline
		VkShaderModule phongVert = CreateShaderModule("Frameworks/shaders/phong.vert.spv");
		shaderStages[0].module = phongVert;
		VkShaderModule phongFrag = CreateShaderModule("Frameworks/shaders/phong.frag.spv");
		shaderStages[1].module = phongFrag;

		// We are using this pipeline as the base for the other pipelines (derivatives)
		pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myPhongPipeline), "Failed to create the Phong pipeline");

		vkDestroyShaderModule(myDevice, phongVert, nullptr);
		vkDestroyShaderModule(myDevice, phongFrag, nullptr);

		// Base pipeline will be our first created pipeline
		pipelineInfo.basePipelineIndex = -1; // It's only allowed to either use a handle or index for the base pipeline
		pipelineInfo.basePipelineHandle = myPhongPipeline;

		// All pipelines created after the base pipeline will be derivatives
		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;

		// Toon shading pipeline
		VkShaderModule toonVert = CreateShaderModule("Frameworks/shaders/toon.vert.spv");
		shaderStages[0].module = toonVert;
		VkShaderModule toonFrag = CreateShaderModule("Frameworks/shaders/toon.frag.spv");
		shaderStages[1].module = toonFrag;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myToonPipeline), "Failed to create the Toon pipeline");

		vkDestroyShaderModule(myDevice, toonVert, nullptr);
		vkDestroyShaderModule(myDevice, toonFrag, nullptr);

		// Pipeline for wire frame rendering
		// Non solid rendering is not a mandatory Vulkan feature
		if (VulkanRenderCore::GetInstance()->GetVulkanDevice()->myEnabledFeatures.fillModeNonSolid)
		{
			rasterizerState.polygonMode = VK_POLYGON_MODE_LINE;

			VkShaderModule wireframeVert = CreateShaderModule("Frameworks/shaders/wireframe.vert.spv");
			shaderStages[0].module = wireframeVert;
			VkShaderModule wireframeFrag = CreateShaderModule("Frameworks/shaders/wireframe.frag.spv");
			shaderStages[1].module = wireframeFrag;

			VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myWireFramePipeline), "Failed to create the Wireframe pipeline");

			vkDestroyShaderModule(myDevice, wireframeVert, nullptr);
			vkDestroyShaderModule(myDevice, wireframeFrag, nullptr);
		}
	}
}
