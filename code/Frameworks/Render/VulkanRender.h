#pragma once

#include "RenderModel.h"
#include "RenderRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanModel.h"
#include "VulkanDescriptorContainer.h"
#include "VulkanShaderHelpers.h"

struct GLFWwindow;

namespace Render::Vulkan
{
	struct Device;
	class SwapChain;

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

		void RegisterWindow(GLFWwindow* aWindow, Render::Renderer::Type aType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Render::Renderer* GetRenderer(GLFWwindow* aWindow);

		Render::Model* SpawnModel(const BaseModelData& someData);
		void DespawnModel(Render::Model* aModel);

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
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

		VkDescriptorSetLayout GetDescriptorSetLayout(ShaderHelpers::DescriptorLayout aLayout);
		void AllocateDescriptorSet(ShaderHelpers::DescriptorLayout aLayout, VkDescriptorSet& anOutDescriptorSet);
		void UpdateDescriptorSet(ShaderHelpers::DescriptorLayout aLayout, const ShaderHelpers::DescriptorInfo& someDescriptorInfo, VkDescriptorSet aDescriptorSet);

	private:
		static RenderCore* ourInstance;

		RenderCore();
		~RenderCore();

		void CreateVkInstance();
		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;
		void CreateDevice();
		Device* myDevice = nullptr;

		void SetupDefaultData();
		void DestroyDefaultData();
		void SetupTexture(Image& anImage, uint8* aColor);
		Image myWhiteTexture;
		Image myBlackTexture;
		Image myMissingTexture;
		Buffer myDefaultMaterial;
		Buffer myDefaultJointsMatrix;

		std::vector<SwapChain*> mySwapChains;

		uint GetModelsLifetime() const;
		void DeleteUnusedModels(bool aDeleteNow = false);
		typedef std::pair<Model*, uint> ModelLifeTime;
		std::vector<ModelLifeTime> myModelsToDelete;

		std::array<DescriptorContainer, (size_t)ShaderHelpers::DescriptorLayout::Count> myDescriptorContainers;
	};
}
