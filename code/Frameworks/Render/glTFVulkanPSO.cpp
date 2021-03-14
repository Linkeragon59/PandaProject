#include "glTFVulkanPSO.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

namespace Render
{
namespace glTF
{
	VkDescriptorSetLayout VulkanPSO::ourPerFrameDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout VulkanPSO::ourPerObjectDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout VulkanPSO::ourPerImageDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout VulkanPSO::ourPerSkinDescriptorSetLayout = VK_NULL_HANDLE;

	void VulkanPSO::SetupDescriptorSetLayouts()
	{
		// Per Frame

		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding uboFrameBinding{};
		uboFrameBinding.binding = 0;
		uboFrameBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboFrameBinding.descriptorCount = 1;
		uboFrameBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> frameBindings = { uboFrameBinding };

		VkDescriptorSetLayoutCreateInfo frameDescriptorLayoutInfo{};
		frameDescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		frameDescriptorLayoutInfo.bindingCount = (uint32_t)frameBindings.size();
		frameDescriptorLayoutInfo.pBindings = frameBindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), &frameDescriptorLayoutInfo, nullptr, &ourPerFrameDescriptorSetLayout),
			"Failed to create the per frame descriptor set layout");

		// Per Object

		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding uboObjectBinding{};
		uboObjectBinding.binding = 0;
		uboObjectBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboObjectBinding.descriptorCount = 1;
		uboObjectBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> objectBindings = { uboObjectBinding };

		VkDescriptorSetLayoutCreateInfo objectDescriptorLayoutInfo{};
		objectDescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		objectDescriptorLayoutInfo.bindingCount = (uint32_t)objectBindings.size();
		objectDescriptorLayoutInfo.pBindings = objectBindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), &objectDescriptorLayoutInfo, nullptr, &ourPerObjectDescriptorSetLayout),
			"Failed to create the per object descriptor set layout");

		// Per Skin

		// Binding 0 : Vertex shader storage buffer
		VkDescriptorSetLayoutBinding storageBufferBinding{};
		storageBufferBinding.binding = 0;
		storageBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storageBufferBinding.descriptorCount = 1;
		storageBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> skinBindings = { storageBufferBinding };

		VkDescriptorSetLayoutCreateInfo skinDescriptorLayoutInfo{};
		skinDescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		skinDescriptorLayoutInfo.bindingCount = (uint32_t)skinBindings.size();
		skinDescriptorLayoutInfo.pBindings = skinBindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), &skinDescriptorLayoutInfo, nullptr, &ourPerSkinDescriptorSetLayout),
			"Failed to create the per skin descriptor set layout");

		// Per Image

		// Binding 0 : Fragment shader sampler
		VkDescriptorSetLayoutBinding samplerBinding{};
		samplerBinding.binding = 0;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerBinding.descriptorCount = 1;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> imageBindings = { samplerBinding };

		VkDescriptorSetLayoutCreateInfo imageDescriptorLayoutInfo{};
		imageDescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		imageDescriptorLayoutInfo.bindingCount = (uint32_t)imageBindings.size();
		imageDescriptorLayoutInfo.pBindings = imageBindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), &imageDescriptorLayoutInfo, nullptr, &ourPerImageDescriptorSetLayout),
			"Failed to create the per image descriptor set layout");
	}

	void VulkanPSO::DestroyDescriptorSetLayouts()
	{
		vkDestroyDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), ourPerFrameDescriptorSetLayout, nullptr);
		ourPerFrameDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), ourPerObjectDescriptorSetLayout, nullptr);
		ourPerObjectDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), ourPerImageDescriptorSetLayout, nullptr);
		ourPerImageDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(VulkanRenderer::GetInstance()->GetDevice(), ourPerSkinDescriptorSetLayout, nullptr);
		ourPerSkinDescriptorSetLayout = VK_NULL_HANDLE;
	}

	VulkanPSO::VulkanPSO(VkRenderPass aRenderPass)
		: myRenderPass(aRenderPass)
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();
		PreparePipeline();
	}

	VulkanPSO::~VulkanPSO()
	{
		vkDestroyPipeline(myDevice, myPipeline, nullptr);
		myPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;
	}

	void VulkanPSO::PreparePipeline()
	{
		std::array<VkDescriptorSetLayout, 4> descriptorSetLayouts = {
			ourPerFrameDescriptorSetLayout,
			ourPerObjectDescriptorSetLayout,
			ourPerSkinDescriptorSetLayout,
			ourPerImageDescriptorSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myPipelineLayout), "Failed to create the pipeline layout");

		VkShaderModule vertModule = CreateShaderModule("Frameworks/shaders/glTFDefault.vert.spv");
		VkShaderModule fragModule = CreateShaderModule("Frameworks/shaders/glTFDefault.frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
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
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myPipeline), "Failed to create the glTF pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}
}
}
