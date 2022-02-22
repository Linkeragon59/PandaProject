#pragma once

#include "GameCore_Module.h"

#include "Render_Renderer.h"
#include "Render_VulkanBuffer.h"
#include "Render_VulkanImage.h"
#include "Render_DescriptorContainer.h"
#include "Render_ShaderHelpers.h"

#include "GameCore_Entity.h"
#include "Render_EntityRenderComponent.h"

struct GLFWwindow;

namespace Render
{
	struct VulkanDevice;
	class SwapChain;

	enum class RendererType
	{
		Deferred,
		Count
	};

	class RenderModule : public GameCore::Module
	{
	DECLARE_GAMECORE_MODULE(RenderModule, "Render")

	protected:
		void OnRegister() override;
		void OnUnregister() override;
		
		void OnInitialize() override;
		void OnFinalize() override;

		void OnUpdate(GameCore::Module::UpdateType aType) override;

	public:
		void RegisterWindow(GLFWwindow* aWindow, RendererType aType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Renderer* GetRenderer(GLFWwindow* aWindow) const;

		template<typename T>
		T* GetEntityRenderComponent(GameCore::EntityHandle aHandle)
		{
			auto entityComponent = myRenderComponents.find(aHandle);
			if (entityComponent != myRenderComponents.end())
				return entityComponent->second;
			return nullptr;
		}
		template<typename T>
		T* AddEntityRenderComponent(GameCore::EntityHandle aHandle)
		{
			if (myRenderComponents* component = Get3DModelComponent(aHandle))
				return component;
			myRenderComponents[aHandle] = new T();
			return myRenderComponents[aHandle];
		}
		template<typename T>
		void RemoveEntityRenderComponent(GameCore::EntityHandle aHandle)
		{
			delete myRenderComponents[aHandle];
			myRenderComponents.erase(aHandle);
		}

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

		std::map<GameCore::EntityHandle, EntityRenderComponent*> myRenderComponents;
	};
}
