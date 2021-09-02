#include "Render_Gui.h"

#include "imgui.h"

namespace Render
{
	Gui::Gui()
	{
		ImGui::CreateContext();

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
	}

	Gui::~Gui()
	{
		myFontTexture.Destroy();

		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		ImGui::DestroyContext();
	}

	void Gui::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex)
	{
		{
			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2((float)1920, (float)1080);
			io.DeltaTime = 1.0f / 6.0f;

			io.MousePos = ImVec2(0.0f, 0.0f);
			io.MouseDown[0] = false;
			io.MouseDown[1] = false;

			ImGui::NewFrame();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			ImGui::SetNextWindowPos(ImVec2(10, 10));
			ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Always);
			ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::PushItemWidth(110.0f);

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			if (ImGui::CollapsingHeader("Subpasses", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Test Gui");
			}
			if (ImGui::CollapsingHeader("Subpasses", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Button("Test Button");
			}

			drawList->AddCircle(ImVec2(200.0f, 200.0f), 50.0f, ImGui::GetColorU32(IM_COL32(255, 255, 255, 255)), 0, 5.0f);

			ImGui::PopItemWidth();

			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::Render();
		}

		ImDrawData* imDrawData = ImGui::GetDrawData();
		if (!imDrawData || imDrawData->CmdListsCount == 0)
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

		ShaderHelpers::GuiDescriptorInfo info;
		info.myFontSamplerInfo = &myFontTexture.myDescriptor;
		VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(ShaderHelpers::BindType::Gui, info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);

		ImGuiIO& io = ImGui::GetIO();
		myPushConstBlock.myScale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		myPushConstBlock.myTranslate = glm::vec2(-1.0f);
		vkCmdPushConstants(aCommandBuffer, aPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(myPushConstBlock), &myPushConstBlock);

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

	void Gui::PrepareFont()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->AddFontFromFileTTF("Frameworks/fonts/Roboto-Medium.ttf", 16.0f);
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		VkDeviceSize textureSize = texWidth * texHeight * 4;
		VulkanBuffer textureStaging;
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
			RenderCore::GetInstance()->GetGraphicsQueue());

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { static_cast<uint>(texWidth), static_cast<uint>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myFontTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, RenderCore::GetInstance()->GetGraphicsQueue());

		textureStaging.Destroy();

		myFontTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			RenderCore::GetInstance()->GetGraphicsQueue());

		myFontTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myFontTexture.CreateImageSampler();
		myFontTexture.SetupDescriptor();
	}
}
