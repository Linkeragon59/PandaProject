#include "Render_Gui.h"

#include "GameCore_imgui.h"

namespace Render
{
	Gui::Gui(ImGuiContext* aGuiContext)
		: myGuiContext(aGuiContext)
	{
		PrepareFont();
	}

	void Gui::Update()
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData || imDrawData->CmdListsCount == 0)
			return;

		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if (vertexBufferSize == 0 || indexBufferSize == 0)
			return;

		// Vertex buffer
		if (!myVertexBuffer || myVertexCount != imDrawData->TotalVtxCount)
		{
			myVertexBuffer = new VulkanBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			myVertexCount = imDrawData->TotalVtxCount;
			myVertexBuffer->Map();
		}

		// Index buffer
		if (!myIndexBuffer || myIndexCount != imDrawData->TotalIdxCount)
		{
			myIndexBuffer = new VulkanBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			myIndexCount = imDrawData->TotalIdxCount;
			myIndexBuffer->Map();
		}

		// Update data
		ImDrawVert* vtxDst = (ImDrawVert*)myVertexBuffer->myMappedData;
		ImDrawIdx* idxDst = (ImDrawIdx*)myIndexBuffer->myMappedData;
		for (int i = 0; i < imDrawData->CmdListsCount; ++i)
		{
			const ImDrawList* cmdList = imDrawData->CmdLists[i];
			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		myVertexBuffer->Flush();
		myIndexBuffer->Flush();
	}

	void Gui::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex)
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData || imDrawData->CmdListsCount == 0)
			return;

		ShaderHelpers::GuiDescriptorInfo info;
		info.myFontSamplerInfo = &myFontTexture->myDescriptor;
		VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(ShaderHelpers::BindType::Gui, info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);

		ImGuiIO& io = ImGui::GetIO();
		myPushConstBlock.myScale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		myPushConstBlock.myTranslate = glm::vec2(-1.0f);
		vkCmdPushConstants(aCommandBuffer, aPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(myPushConstBlock), &myPushConstBlock);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer->myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer->myBuffer, 0, VK_INDEX_TYPE_UINT16); // See ImDrawIdx declaration

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

	void Gui::PrepareFont()
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->AddFontFromFileTTF("Frameworks/fonts/Roboto-Medium.ttf", 16.0f); // TODO : Choose which font to load
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		VkDeviceSize textureSize = texWidth * texHeight * 4;
		VulkanBuffer textureStaging;
		textureStaging.Create(textureSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		memcpy(textureStaging.myMappedData, fontData, static_cast<size_t>(textureSize));
		textureStaging.Unmap();

		myFontTexture = new VulkanImage(texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myFontTexture->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			RenderCore::GetInstance()->GetGraphicsQueue());

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { static_cast<uint>(texWidth), static_cast<uint>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myFontTexture->myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, RenderCore::GetInstance()->GetGraphicsQueue());

		textureStaging.Destroy();

		myFontTexture->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			RenderCore::GetInstance()->GetGraphicsQueue());

		myFontTexture->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myFontTexture->CreateImageSampler();
		myFontTexture->SetupDescriptor();
	}
}
