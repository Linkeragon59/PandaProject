#pragma once

#include "Render_RenderModule.h"

#include "Render_Renderer.h"
#include "Render_VulkanBuffer.h"
#include "Render_VulkanImage.h"
#include "Render_DescriptorContainer.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	struct VulkanDevice;
	class SwapChain;

	class RenderCore
	{
	public:
		static RenderCore* GetInstance() { return RenderModule::GetInstance()->GetRenderCore(); }
		RenderCore();
		~RenderCore();

		void Initialize();
		void Finalize();

		void StartFrame();
		void Update();
		void EndFrame();

		void RegisterWindow(GLFWwindow* aWindow, RendererType aType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Renderer* GetRenderer(GLFWwindow* aWindow) const;

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
	};
}
