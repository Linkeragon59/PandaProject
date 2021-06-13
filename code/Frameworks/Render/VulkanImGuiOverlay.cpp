#include "VulkanImGuiOverlay.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"

#include "imgui.h"

namespace Render
{
namespace Vulkan
{
	ImGuiOverlay::ImGuiOverlay()
	{
		myDevice = Renderer::GetInstance()->GetDevice();
		myContext = ImGui::CreateContext();
	}

	ImGuiOverlay::~ImGuiOverlay()
	{
		ImGui::DestroyContext(myContext);
	}

	void ImGuiOverlay::Prepare(VkRenderPass aRenderPass)
	{
		// Color scheme
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 1.0f;

		PrepareFont();
		SetupDescriptorPool();
		SetupDescriptorSetLayout();
		SetupDescriptorSet();
		SetupPipeline(aRenderPass);
	}

	void ImGuiOverlay::Update(uint aWidth, uint aHeight)
	{
		// TODO - This part of the update should be called by the GameWork?

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)aWidth, (float)aHeight);
		io.DeltaTime = 1.0f/60.0f;

		io.MousePos = ImVec2(0.0f, 0.0f);
		//io.MouseDown[0] = mouseButtons.left;
		//io.MouseDown[1] = mouseButtons.right;

		ImGui::NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::Begin("UI Test", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::Text("Test");

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::Render();

		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData)
			return;

		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if (vertexBufferSize == 0 || indexBufferSize == 0)
			return;

		// Vertex buffer
		if (myVertexBuffer.myBuffer == VK_NULL_HANDLE || myVertexCount != imDrawData->TotalVtxCount)
		{
			myVertexBuffer.Destroy();
			myVertexBuffer.Create(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			myVertexCount = imDrawData->TotalVtxCount;
			myVertexBuffer.Map();
		}

		// Index buffer
		if (myIndexBuffer.myBuffer == VK_NULL_HANDLE || myIndexCount != imDrawData->TotalIdxCount)
		{
			myIndexBuffer.Destroy();
			myIndexBuffer.Create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			myIndexCount = imDrawData->TotalIdxCount;
			myIndexBuffer.Map();
		}

		// Update data
		ImDrawVert* vtxDst = (ImDrawVert*)myVertexBuffer.myMappedData;
		ImDrawIdx* idxDst = (ImDrawIdx*)myIndexBuffer.myMappedData;
		for (int i = 0; i < imDrawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		myVertexBuffer.Flush();
		myIndexBuffer.Flush();
	}

	void ImGuiOverlay::Destroy()
	{
		vkDestroyPipeline(myDevice, myPipeline, nullptr);
		myPipeline = VK_NULL_HANDLE;

		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(myDevice, myDescriptorSetLayout, nullptr);
		myDescriptorSetLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		myFontTexture.Destroy();
	}

	void ImGuiOverlay::Resize(uint aWidth, uint aHeight)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)aWidth, (float)aHeight);
	}

	void ImGuiOverlay::Draw(const VkCommandBuffer aCommandBuffer)
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData || imDrawData->CmdListsCount == 0)
			return;

		ImGuiIO& io = ImGui::GetIO();

		vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipeline);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayout, 0, 1, &myDescriptorSet, 0, NULL);

		myPushConstBlock.myScale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		myPushConstBlock.myTranslate = glm::vec2(-1.0f);
		vkCmdPushConstants(aCommandBuffer, myPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &myPushConstBlock);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer.myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT16); // See ImDrawIdx declaration

		int vertexOffset = 0;
		int indexOffset = 0;
		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* cmd = &cmdList->CmdBuffer[j];
				VkRect2D scissorRect{};
				scissorRect.offset.x = std::max((int)cmd->ClipRect.x, 0);
				scissorRect.offset.y = std::max((int)cmd->ClipRect.y, 0);
				scissorRect.extent.width = (uint32_t)(cmd->ClipRect.z - cmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(cmd->ClipRect.w - cmd->ClipRect.y);
				vkCmdSetScissor(aCommandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(aCommandBuffer, cmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += cmd->ElemCount;
			}
			vertexOffset += cmdList->VtxBuffer.Size;
		}
	}

	void ImGuiOverlay::PrepareFont()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->AddFontFromFileTTF("Frameworks/fonts/Roboto-Medium.ttf", 16.0f);
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		VkDeviceSize textureSize = texWidth * texHeight * 4;
		Buffer textureStaging;
		textureStaging.Create(textureSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		memcpy(textureStaging.myMappedData, fontData, static_cast<size_t>(textureSize));
		textureStaging.Unmap();

		myFontTexture.Create(texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myFontTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			Renderer::GetInstance()->GetGraphicsQueue());

		VkCommandBuffer commandBuffer = BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { static_cast<uint>(texWidth), static_cast<uint>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myFontTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, Renderer::GetInstance()->GetGraphicsQueue());

		textureStaging.Destroy();

		myFontTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Renderer::GetInstance()->GetGraphicsQueue());

		myFontTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myFontTexture.CreateImageSampler();
		myFontTexture.SetupDescriptor();
	}

	void ImGuiOverlay::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void ImGuiOverlay::SetupDescriptorSetLayout()
	{
		std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
		// Binding 0 : Font Sampler
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = (uint)bindings.size();
		descriptorLayoutInfo.pBindings = bindings.data();
		VK_CHECK_RESULT(
			vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myDescriptorSetLayout),
			"Failed to create the UI Overlay DescriptorSetLayout");
	}

	void ImGuiOverlay::SetupDescriptorSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { myDescriptorSetLayout };
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the UI Overlay descriptor set");

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].pImageInfo = &myFontTexture.myDescriptor;

		vkUpdateDescriptorSets(myDevice, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void ImGuiOverlay::SetupPipeline(VkRenderPass aRenderPass)
	{
		std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts = { myDescriptorSetLayout };

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		VK_CHECK_RESULT(
			vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myPipelineLayout),
			"Failed to create the GBuffer pipeline layout");

		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/uiOverlay_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("Frameworks/shaders/uiOverlay_frag.spv");

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

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ImDrawVert);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(ImDrawVert, pos);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(ImDrawVert, uv);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[2].offset = offsetof(ImDrawVert, col);

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
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // TODO : Check
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

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
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.renderPass = aRenderPass;
		pipelineInfo.subpass = 2; // TODO : Shouldn't we have a separate subpass?
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK_RESULT(
			vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myPipeline),
			"Failed to create the GBuffer pipeline");

		vkDestroyShaderModule(myDevice, vertModule, nullptr);
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
	}
}
}
