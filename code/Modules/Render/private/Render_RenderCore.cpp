#include "Render_RenderCore.h"

#include "Render_Debug.h"
#include "Render_VulkanDevice.h"
#include "Render_SwapChain.h"
#include "Render_Resource.h"

#include "GameCore_EntityCameraComponent.h"
#include "GameCore_EntityTransformComponent.h"

#include <GLFW/glfw3.h>

namespace Render
{
	namespace
	{
		uint locVulkanApiVersion = VK_API_VERSION_1_0;

#if WINDOWS_BUILD && DEBUG_BUILD
		constexpr bool locEnableValidationLayers = true;
#else
		constexpr bool locEnableValidationLayers = false;
#endif
	}

	RenderCore::RenderCore()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			Debug::FillDebugMessengerCreateInfo(createInfo);
			VK_CHECK_RESULT(Debug::CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger), "Couldn't create a debug messenger!");
		}

		CreateDevice();

		if (locEnableValidationLayers)
			Debug::SetupDebugMarkers(myDevice->myLogicalDevice);
	}

	RenderCore::~RenderCore()
	{
		delete myDevice;

		if (locEnableValidationLayers)
			Debug::DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void RenderCore::Initialize()
	{
		SetupDefaultData();
		RenderResource::EnableDeleteQueue(true);

		RegisterWindow(GameCore::Facade::GetInstance()->GetMainWindow(), RendererType::Deferred);
	}

	void RenderCore::Finalize()
	{
		UnregisterWindow(GameCore::Facade::GetInstance()->GetMainWindow());

		Assert(mySwapChains.empty(), "Renderering is being finalized while swapchains are still alive!");

		RenderResource::EnableDeleteQueue(false);

		for (DescriptorContainer& container : myDescriptorContainers)
			container.Destroy();

		DestroyDefaultData();
	}

	void RenderCore::StartFrame()
	{
		RecycleDescriptorSets();

		for (SwapChain* swapChain : mySwapChains)
			swapChain->AcquireNext();
	}

	void RenderCore::Update()
	{
		{
			GameCore::ComponentContainer<EntitySimpleGeometryModelComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntitySimpleGeometryModelComponent>();
			GameCore::ComponentContainer<GameCore::Entity3DTransformComponent>* transformContainer = GameCore::EntityModule::GetInstance()->GetComponentContainer<GameCore::Entity3DTransformComponent>();
			for (auto iter = container->begin(), end = container->end(); iter != end; ++iter)
			{
				if (GameCore::Entity3DTransformComponent* transform = transformContainer->GetComponent(iter.GetEntityId()))
					iter.GetComponent()->Update(transform->GetMatrix());
			}
		}

		{
			GameCore::ComponentContainer<EntityglTFModelComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntityglTFModelComponent>();
			GameCore::ComponentContainer<GameCore::Entity3DTransformComponent>* transformContainer = GameCore::EntityModule::GetInstance()->GetComponentContainer<GameCore::Entity3DTransformComponent>();
			for (auto iter = container->begin(), end = container->end(); iter != end; ++iter)
			{
				if (GameCore::Entity3DTransformComponent* transform = transformContainer->GetComponent(iter.GetEntityId()))
					iter.GetComponent()->Update(transform->GetMatrix());
			}
		}

		{
			GameCore::ComponentContainer<EntityGuiComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntityGuiComponent>();
			for (EntityGuiComponent* component : *container)
			{
				component->Update();
			}
		}

		Renderer* renderer = mySwapChains[0]->GetRenderer();

		{
			GameCore::ComponentContainer<GameCore::EntityCameraComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<GameCore::EntityCameraComponent>();
			for (GameCore::EntityCameraComponent* component : *container)
			{
				renderer->SetViewProj(component->GetViewMatrix(), component->GetPerspectiveMatrix());
			}
		}

		{
			GameCore::ComponentContainer<EntitySimpleGeometryModelComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntitySimpleGeometryModelComponent>();
			for (EntitySimpleGeometryModelComponent* component : *container)
			{
				renderer->DrawModel(component->GetModel());
			}
		}

		{
			GameCore::ComponentContainer<EntityglTFModelComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntityglTFModelComponent>();
			for (EntityglTFModelComponent* component : *container)
			{
				renderer->DrawModel(component->GetModel());
			}
		}

		{
			GameCore::ComponentContainer<EntityGuiComponent>* container = GameCore::EntityModule::GetInstance()->GetComponentContainer<EntityGuiComponent>();
			for (EntityGuiComponent* component : *container)
			{
				renderer->DrawGui(component->GetGui());
			}
		}
	}

	void RenderCore::EndFrame()
	{
		for (SwapChain* swapChain : mySwapChains)
			swapChain->Present();
	}

	void RenderCore::RegisterWindow(GLFWwindow* aWindow, RendererType aType)
	{
		SwapChain* swapChain = new SwapChain(aWindow, aType);
		mySwapChains.push_back(swapChain);

		UpdateMaxInFlightFramesCount();
	}

	void RenderCore::UnregisterWindow(GLFWwindow* aWindow)
	{
		auto it = std::find_if(mySwapChains.begin(), mySwapChains.end(), [aWindow](SwapChain* aSwapChain) { return aSwapChain->GetWindowHandle() == aWindow; });
		if (it != mySwapChains.end())
		{
			delete (*it);
			mySwapChains.erase(it);
		}

		UpdateMaxInFlightFramesCount();
	}

	Renderer* RenderCore::GetRenderer(GLFWwindow* aWindow) const
	{
		auto it = std::find_if(mySwapChains.begin(), mySwapChains.end(), [aWindow](SwapChain* aSwapChain) { return aSwapChain->GetWindowHandle() == aWindow; });
		if (it != mySwapChains.end())
		{
			return (*it)->GetRenderer();
		}
		return nullptr;
	}

	VkPhysicalDevice RenderCore::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice RenderCore::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator RenderCore::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue RenderCore::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool RenderCore::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}


	VkDescriptorSetLayout RenderCore::GetDescriptorSetLayout(ShaderHelpers::BindType aType)
	{
		if (myDescriptorContainers[(size_t)aType].myLayout == VK_NULL_HANDLE)
			myDescriptorContainers[(size_t)aType].Create(aType);

		return myDescriptorContainers[(size_t)aType].myLayout;
	}

	VkDescriptorSet RenderCore::GetDescriptorSet(ShaderHelpers::BindType aType, const ShaderHelpers::DescriptorInfo& someDescriptorInfo)
	{
		Assert(myDescriptorContainers[(size_t)aType].myLayout != VK_NULL_HANDLE);
		return myDescriptorContainers[(size_t)aType].GetDescriptorSet(someDescriptorInfo, myMaxInFlightFramesCount);
	}

	void RenderCore::CreateVkInstance()
	{
		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			Debug::PopulateValidationLayers(layers);

		Verify(Helpers::CheckInstanceLayersSupport(layers), "Validation layers are enabled but not supported!");

		uint glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (locEnableValidationLayers)
			Debug::PopulateDebugExtensions(extensions);

		Verify(Helpers::CheckInstanceExtensionsSupport(extensions), "Required extensions are not available!");

		// TODO: Have more parameters when creating the RenderModule
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = locVulkanApiVersion;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		if (locEnableValidationLayers)
		{
			Debug::FillDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		createInfo.enabledLayerCount = static_cast<uint>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &myVkInstance), "Failed to create Vulkan instance!");
	}

	void RenderCore::CreateDevice()
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		Assert(deviceCount > 0, "No physical device supporting Vulkan was found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, devices.data());

		// TODO: Select the physical device based on requirements.
		// For now, just take the first one.
		myDevice = new VulkanDevice(devices[0]);

		// Choose Device features to enable
		VkPhysicalDeviceFeatures enabledFeatures{};
		if (myDevice->myFeatures.samplerAnisotropy == VK_TRUE)
		{
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		}
		if (myDevice->myFeatures.fillModeNonSolid)
		{
			enabledFeatures.fillModeNonSolid = VK_TRUE;
			if (myDevice->myFeatures.wideLines)
			{
				enabledFeatures.wideLines = VK_TRUE;
			}
		};

		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			Debug::PopulateValidationLayers(layers);

		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		if (myDevice->SupportsExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

		myDevice->SetupLogicalDevice(
			enabledFeatures,
			layers,
			extensions,
			VK_QUEUE_GRAPHICS_BIT);
		myDevice->SetupVmaAllocator(myVkInstance, locVulkanApiVersion);
	}

	void RenderCore::SetupDefaultData()
	{
		uint8 white[4] = { 0xff, 0xff, 0xff, 0xff };
		uint8 black[4] = { 0x00, 0x00, 0x00, 0xff };
		uint8 missing[4] = { 0xff, 0x14, 0x93, 0xff };
		SetupTexture(myWhiteTexture, white);
		SetupTexture(myBlackTexture, black);
		SetupTexture(myMissingTexture, missing);

		ShaderHelpers::MaterialData materialData;
		myDefaultMaterial.Create(sizeof(ShaderHelpers::MaterialData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultMaterial.SetupDescriptor();
		myDefaultMaterial.Map();
		memcpy(myDefaultMaterial.myMappedData, &materialData, sizeof(ShaderHelpers::MaterialData));
		myDefaultMaterial.Unmap();

		ShaderHelpers::JointMatrixData jointsData;
		myDefaultJointsMatrix.Create(sizeof(ShaderHelpers::JointMatrixData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultJointsMatrix.SetupDescriptor();
		myDefaultJointsMatrix.Map();
		memcpy(myDefaultJointsMatrix.myMappedData, &jointsData, sizeof(ShaderHelpers::JointMatrixData));
		myDefaultJointsMatrix.Unmap();
	}

	void RenderCore::DestroyDefaultData()
	{
		myWhiteTexture.Destroy();
		myBlackTexture.Destroy();
		myMissingTexture.Destroy();

		myDefaultMaterial.Destroy();
		myDefaultJointsMatrix.Destroy();
	}

	void RenderCore::SetupTexture(VulkanImage& anImage, uint8* aColor)
	{
		anImage.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			GetGraphicsQueue());

		VulkanBuffer textureStaging;
		textureStaging.Create(4,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		{
			memcpy(textureStaging.myMappedData, aColor, 4);
		}
		textureStaging.Unmap();

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { 1, 1, 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, anImage.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, GetGraphicsQueue());

		textureStaging.Destroy();

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			GetGraphicsQueue());
		anImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		anImage.CreateImageSampler();
		anImage.SetupDescriptor();
	}

	void RenderCore::UpdateMaxInFlightFramesCount()
	{
		myMaxInFlightFramesCount = 0;
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			myMaxInFlightFramesCount = std::max(myMaxInFlightFramesCount, mySwapChains[i]->GetImagesCount());
		}
	}

	void RenderCore::RecycleDescriptorSets()
	{
		for (DescriptorContainer& container : myDescriptorContainers)
			container.RecycleDescriptors();
	}
}
