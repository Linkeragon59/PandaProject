#pragma once

#include "VulkanHelpers.h"

namespace Render
{
	void PopulateValidationLayers(std::vector<const char*>& anOutLayerList);
	void PopulateDebugExtensions(std::vector<const char*>& anOutExtensionList);

	VkResult CreateDebugMessenger(
		VkInstance anInstance,
		const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
		const VkAllocationCallbacks* anAllocator,
		VkDebugUtilsMessengerEXT* anOutMessenger);

	void DestroyDebugMessenger(
		VkInstance anInstance,
		VkDebugUtilsMessengerEXT aMessenger,
		const VkAllocationCallbacks* anAllocator);

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT aMessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* aUserData);

	void FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& someOutInfo);
}
