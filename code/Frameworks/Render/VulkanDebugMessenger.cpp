#include "VulkanDebugMessenger.h"

#include <iostream>

namespace Render
{
namespace Vulkan
{
	void PopulateValidationLayers(std::vector<const char*>& anOutLayerList)
	{
		anOutLayerList.push_back("VK_LAYER_KHRONOS_validation");
		anOutLayerList.push_back("VK_LAYER_LUNARG_monitor");
	}

	void PopulateDebugExtensions(std::vector<const char*>& anOutExtensionList)
	{
		anOutExtensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	VkResult CreateDebugMessenger(
		VkInstance anInstance,
		const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
		const VkAllocationCallbacks* anAllocator,
		VkDebugUtilsMessengerEXT* anOutMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(anInstance, "vkCreateDebugUtilsMessengerEXT");
		return (func != nullptr) ? func(anInstance, aCreateInfo, anAllocator, anOutMessenger) : VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugMessenger(
		VkInstance anInstance,
		VkDebugUtilsMessengerEXT aMessenger,
		const VkAllocationCallbacks* anAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(anInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(anInstance, aMessenger, anAllocator);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT aMessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* aUserData)
	{
		(void)aMessageSeverity;
		(void)aMessageType;
		(void)aUserData;

		std::cerr << "validation layer: " << aCallbackData->pMessage << std::endl;
		return VK_FALSE; // Don't interrupt the execution
	}

	void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& someOutInfo)
	{
		someOutInfo = {};
		someOutInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		someOutInfo.messageSeverity =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		someOutInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		someOutInfo.pfnUserCallback = DebugMessengerCallback;
	}
}
}
