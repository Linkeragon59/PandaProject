#pragma once

#include "Render_Renderer.h"
#include "Render_VulkanBuffer.h"
#include "Render_VulkanImage.h"
#include "Render_DescriptorContainer.h"
#include "Render_ShaderHelpers.h"

struct GLFWwindow;

namespace Render
{
	struct VulkanDevice;
	class SwapChain;
	class ModelContainer;
	class GuiContainer;

	class RenderCore
	{
	public:
		static void Create();
		static void Destroy();
		static RenderCore* GetInstance() { return ourInstance; }

		void Initialize();
		void Finalize();

		void StartFrame();
		void EndFrame();

		void RegisterWindow(GLFWwindow* aWindow, Renderer::Type aType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Renderer* GetRenderer(GLFWwindow* aWindow) const;

		ModelContainer* GetModelContainer() const { return myModelContainer; }
		GuiContainer* GetGuiContainer() const { return myGuiContainer; }

		VkInstance GetVkInstance() const { return myVkInstance; }

		VulkanDevice* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		const VkDescriptorImageInfo* GetWhiteTextureDescriptorInfo() const { return &myWhiteTexture.myDescriptor; }
		const VkDescriptorImageInfo* GetBlackTextureDescriptorInfo() const { return &myBlackTexture.myDescriptor; }
		const VkDescriptorImageInfo* GetMissingTextureDescriptorInfo() const { return &myMissingTexture.myDescriptor; }
		const VkDescriptorBufferInfo* GetDefaultMaterialDescriptorInfo() const { return &myDefaultMaterial.myDescriptor; }
		const VkDescriptorBufferInfo* GetDefaultJointsMatrixDescriptorInfo() const { return &myDefaultJointsMatrix.myDescriptor; }

		VkDescriptorSetLayout GetDescriptorSetLayout(ShaderHelpers::BindType aType);
		VkDescriptorSet GetDescriptorSet(ShaderHelpers::BindType aType, const ShaderHelpers::DescriptorInfo& someDescriptorInfo);

		uint GetMaxInFlightFramesCount() const { return myMaxInFlightFramesCount; }

	private:
		static RenderCore* ourInstance;

		RenderCore();
		~RenderCore();

		void CreateVkInstance();
		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;
		void CreateDevice();
		VulkanDevice* myDevice = nullptr;

		void SetupDefaultData();
		void DestroyDefaultData();
		void SetupTexture(VulkanImage& anImage, uint8* aColor);
		VulkanImage myWhiteTexture;
		VulkanImage myBlackTexture;
		VulkanImage myMissingTexture;
		VulkanBuffer myDefaultMaterial;
		VulkanBuffer myDefaultJointsMatrix;

		std::vector<SwapChain*> mySwapChains;
		void UpdateMaxInFlightFramesCount();
		uint myMaxInFlightFramesCount = 0;

		void RecycleDescriptorSets();
		std::array<DescriptorContainer, (size_t)ShaderHelpers::BindType::Count> myDescriptorContainers;

		ModelContainer* myModelContainer = nullptr;
		GuiContainer* myGuiContainer = nullptr;
	};
}
