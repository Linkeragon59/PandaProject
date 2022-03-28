#include "Render_Gui.h"

#include "Render_ImGuiHelper.h"

#include "GameCore_InputModule.h"
#include "GameCore_WindowModule.h"
#include "GameCore_TimeModule.h"

#include <GLFW/glfw3.h>

namespace Render
{
	Gui::Gui()
	{
		myGuiContext = ImGui::CreateContext();

		// TODO : Choose the window
		myWindow = GameCore::Facade::GetInstance()->GetMainWindow();
		
		glfwGetWindowSize(myWindow, &myWindowWidth, &myWindowHeight);
		myWindowResizeCallbackId = GameCore::WindowModule::GetInstance()->AddWindowSizeCallback([this](int aWidth, int aHeight) {
			myWindowWidth = aWidth;
			myWindowHeight = aHeight;
			}, myWindow);

		myScrollCallbackId = GameCore::InputModule::GetInstance()->AddScrollCallback([this](double aXScroll, double aYScroll) {
			myXScroll = aXScroll;
			myYScroll = aYScroll;
			}, myWindow);

		myCharacterCallbackId = GameCore::InputModule::GetInstance()->AddCharacterCallback([this](uint aUnicodeCodePoint) {
			myTextInput.push(aUnicodeCodePoint);
			}, myWindow);
		
		PrepareFont();

		InitStyle();
		InitIO();
	}

	Gui::~Gui()
	{
		GameCore::WindowModule::GetInstance()->RemoveWindowSizeCallback(myWindowResizeCallbackId);
		GameCore::InputModule::GetInstance()->RemoveScrollCallback(myScrollCallbackId);
		GameCore::InputModule::GetInstance()->RemoveCharacterCallback(myCharacterCallbackId);

		ImGui::DestroyContext(myGuiContext);
	}

	void Gui::Update()
	{
		ImGui::SetCurrentContext(myGuiContext);

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)myWindowWidth, (float)myWindowHeight);
		io.DeltaTime = GameCore::TimeModule::GetInstance()->GetDeltaTime();

		double x, y;
		GameCore::InputModule::GetInstance()->PollMousePosition(x, y, myWindow);
		io.MousePos = ImVec2((float)x, (float)y);

		io.MouseDown[ImGuiMouseButton_Left] = GameCore::InputModule::GetInstance()->PollMouseInput(Input::MouseLeft, myWindow) == Input::Status::Pressed;
		io.MouseDown[ImGuiMouseButton_Right] = GameCore::InputModule::GetInstance()->PollMouseInput(Input::MouseRight, myWindow) == Input::Status::Pressed;
		io.MouseDown[ImGuiMouseButton_Middle] = GameCore::InputModule::GetInstance()->PollMouseInput(Input::MouseMiddle, myWindow) == Input::Status::Pressed;

		io.MouseWheelH = (float)myXScroll;
		io.MouseWheel = (float)myYScroll;
		myXScroll = myYScroll = 0.0;

		io.KeyCtrl = GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyLeftCtrl, myWindow) == Input::Status::Pressed
			|| GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyRightCtrl, myWindow) == Input::Status::Pressed;
		io.KeyShift = GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyLeftShift, myWindow) == Input::Status::Pressed
			|| GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyRightShift, myWindow) == Input::Status::Pressed;
		io.KeyAlt = GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyLeftAlt, myWindow) == Input::Status::Pressed
			|| GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyRightAlt, myWindow) == Input::Status::Pressed;
		io.KeySuper = GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyLeftSuper, myWindow) == Input::Status::Pressed
			|| GameCore::InputModule::GetInstance()->PollKeyInput(Input::KeyRightSuper, myWindow) == Input::Status::Pressed;

		for (uint key = 0; key < ImGuiKey_COUNT; ++key)
		{
			io.KeysDown[io.KeyMap[key]] = GameCore::InputModule::GetInstance()->PollKeyInput((Input::Key)io.KeyMap[key], myWindow) == Input::Status::Pressed;
		}

		while (!myTextInput.empty())
		{
			io.AddInputCharacter(myTextInput.front());
			myTextInput.pop();
		}

		ImGui::NewFrame();

		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("Graph Editor", nullptr,
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_MenuBar
			);

			ImVec2 availableRegionPos = ImGui::GetCursorScreenPos();
			ImVec2 availableRegionSize = ImGui::GetContentRegionAvail();

			ImVec2 canvasPos = availableRegionPos;
			ImVec2 canvasSize = ImVec2(2.0f * availableRegionSize.x / 3.0f, availableRegionSize.y);

			ImVec2 propertiesPos = ImVec2(availableRegionPos.x + 2.0f * availableRegionSize.x / 3.0f, availableRegionPos.y);
			ImVec2 propertiesSize = ImVec2(availableRegionSize.x / 3.0f, availableRegionSize.y);

			ImGui::SetNextWindowPos(canvasPos);
			ImGui::BeginChild("canvas", canvasSize, true, ImGuiWindowFlags_NoScrollbar);
			{
				//myCanvas->Draw(canvasPos, canvasSize);
			}
			ImGui::EndChild();

			ImGui::SetNextWindowPos(propertiesPos);
			ImGui::BeginChild("properties", propertiesSize);
			ImGui::EndChild();

			ImGui::PopStyleVar();

			ImGui::ShowDemoWindow();

			ImGui::End();
		}

		ImGui::Render();

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

	void Gui::InitStyle()
	{
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
	}

	void Gui::InitIO()
	{
		ImGuiIO& io = ImGui::GetIO();

		// For now, disable the .ini files, as it is not useful so far
		io.IniFilename = nullptr;

		io.DisplaySize = ImVec2((float)myWindowWidth, (float)myWindowHeight);

		io.KeyMap[ImGuiKey_Tab] = Input::KeyTab;
		io.KeyMap[ImGuiKey_LeftArrow] = Input::KeyLeft;
		io.KeyMap[ImGuiKey_RightArrow] = Input::KeyRight;
		io.KeyMap[ImGuiKey_UpArrow] = Input::KeyUp;
		io.KeyMap[ImGuiKey_DownArrow] = Input::KeyDown;
		io.KeyMap[ImGuiKey_PageUp] = Input::KeyPageUp;
		io.KeyMap[ImGuiKey_PageDown] = Input::KeyPageDown;
		io.KeyMap[ImGuiKey_Home] = Input::KeyHome;
		io.KeyMap[ImGuiKey_End] = Input::KeyEnd;
		io.KeyMap[ImGuiKey_Insert] = Input::KeyInsert;
		io.KeyMap[ImGuiKey_Delete] = Input::KeyDelete;
		io.KeyMap[ImGuiKey_Backspace] = Input::KeyBackspace;
		io.KeyMap[ImGuiKey_Space] = Input::KeySpace;
		io.KeyMap[ImGuiKey_Enter] = Input::KeyEnter;
		io.KeyMap[ImGuiKey_Escape] = Input::KeyEscape;
		io.KeyMap[ImGuiKey_KeyPadEnter] = Input::KeyNumPadEnter;
		io.KeyMap[ImGuiKey_A] = Input::KeyA;
		io.KeyMap[ImGuiKey_C] = Input::KeyC;
		io.KeyMap[ImGuiKey_V] = Input::KeyV;
		io.KeyMap[ImGuiKey_X] = Input::KeyX;
		io.KeyMap[ImGuiKey_Y] = Input::KeyY;
		io.KeyMap[ImGuiKey_Z] = Input::KeyZ;
	}
}
