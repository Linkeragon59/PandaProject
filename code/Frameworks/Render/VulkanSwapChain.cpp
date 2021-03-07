#include "VulkanSwapChain.h"

#include "VulkanRenderCore.h"

#include <GLFW/glfw3.h>
#include <array>
#include <assert.h>

namespace Render
{
	VulkanSwapChain::VulkanSwapChain(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		myDevice = VulkanRenderCore::GetInstance()->GetDevice();

		VK_CHECK_RESULT(glfwCreateWindowSurface(VulkanRenderCore::GetInstance()->GetVkInstance(), myWindow, nullptr, &mySurface), "Failed to create the surface!");

		Setup();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		Cleanup();

		vkDestroySurfaceKHR(VulkanRenderCore::GetInstance()->GetVkInstance(), mySurface, nullptr);
	}

	void VulkanSwapChain::Setup()
	{
		SetupVkSwapChain();
		SetupDepthStencil();
		CreatePipelineCache();
		SetupRenderPass();
		SetupCommandBuffers();
		SetupFramebuffers();
		PrepareUniformBuffers();
		SetupDescriptorSetLayout();
		PreparePipelines();
		SetupDescriptorPool();
		SetupDescriptorSet();
		BuildCommandBuffers();

		Draw();
	}

	void VulkanSwapChain::Cleanup()
	{
		vkDeviceWaitIdle(myDevice);
		
		myDescriptorSet = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		vkDestroyPipeline(myDevice, myPhongPipeline, nullptr);
		myPhongPipeline = VK_NULL_HANDLE;
		vkDestroyPipeline(myDevice, myToonPipeline, nullptr);
		myToonPipeline = VK_NULL_HANDLE;
		if (myWireFramePipeline)
		{
			vkDestroyPipeline(myDevice, myWireFramePipeline, nullptr);
			myWireFramePipeline = VK_NULL_HANDLE;
		}
		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;

		vkDestroyDescriptorSetLayout(myDevice, myDescriptorSetLayout, nullptr);
		myDescriptorSetLayout = VK_NULL_HANDLE;

		myUniform.Destroy();

		for (auto framebuffer : myFramebuffers)
			vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
		myFramebuffers.clear();

		myCommandBuffers.clear();

		vkDestroyRenderPass(myDevice, myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;

		vkDestroyPipelineCache(myDevice, myPipelineCache, nullptr);
		myPipelineCache = VK_NULL_HANDLE;

		myDepthImage.Destroy();

		for (auto imageView : myImageViews)
			vkDestroyImageView(myDevice, imageView, nullptr);
		myImageViews.clear();

		vkDestroySwapchainKHR(myDevice, myVkSwapChain, nullptr);
		myVkSwapChain = VK_NULL_HANDLE;
	}

	void VulkanSwapChain::SetupVkSwapChain()
	{
		VkPhysicalDevice physicalDevice = VulkanRenderCore::GetInstance()->GetPhysicalDevice();

		// TODO: Support separate queues for graphics and present.
		assert(VulkanRenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.has_value());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			VulkanRenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.value(),
			mySurface,
			&presentSupport);
		if (!presentSupport)
			throw std::runtime_error("The device doesn't support presenting on the graphics queue!");

		VkSurfaceCapabilitiesKHR capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mySurface, &capabilities);
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			myExtent = capabilities.currentExtent;
		}
		else
		{
			int width = 0, height = 0;
			glfwGetWindowSize(myWindow, &width, &height);
			myExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width));
			myExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height));
		}

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, nullptr);
		assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, availableFormats.data());
		VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				surfaceFormat = availableFormat;
		}
		myFormat = surfaceFormat.format;

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, nullptr);
		assert(presentModeCount > 0);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, availablePresentModes.data());
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0)
			imageCount = std::min(imageCount, capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = myExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &myVkSwapChain), "Failed to create a swap chain!");

		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, nullptr);
		myImages.resize(imageCount);
		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, myImages.data());

		myImageViews.resize(myImages.size());
		for (size_t i = 0, e = myImages.size(); i < e; ++i)
		{
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = myImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = myFormat;
			viewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewCreateInfo, nullptr, &myImageViews[i]), "Failed to create an image view for the swap chain!");
		}
	}

	void VulkanSwapChain::SetupDepthStencil()
	{
		VkFormat depthFormat = VulkanRenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		myDepthImage.Create(myExtent.width,
			myExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkImageAspectFlags aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (VulkanImage::DepthFormatHasStencilAspect(depthFormat))
			aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
		myDepthImage.CreateImageView(aspects);

		// optional
		/*TransitionImageLayout(
			VulkanRenderCore::GetInstance()->GetGraphicsQueue(),
			myDepthImage,
			depthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);*/
	}

	void VulkanSwapChain::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.initialDataSize = 0;

		VK_CHECK_RESULT(vkCreatePipelineCache(myDevice, &pipelineCacheCreateInfo, nullptr, &myPipelineCache), "Failed to create the pipeline cache!");
	}

	void VulkanSwapChain::SetupRenderPass()
	{
		std::array<VkAttachmentDescription, 2> attachments;
		// Color attachment
		attachments[0] = {};
		attachments[0].format = myFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1] = {};
		attachments[1].format = VulkanRenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass), "Failed to create the render pass!");
	}

	void VulkanSwapChain::SetupCommandBuffers()
	{
		myCommandBuffers.resize(myImages.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanRenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");
	}

	void VulkanSwapChain::SetupFramebuffers()
	{
		myFramebuffers.resize(myImageViews.size());

		for (size_t i = 0, e = myFramebuffers.size(); i < e; ++i)
		{
			std::array<VkImageView, 2> attachments = { myImageViews[i], myDepthImage.myImageView };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = myRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = myExtent.width;
			framebufferInfo.height = myExtent.height;
			framebufferInfo.layers = 1;

			VK_CHECK_RESULT(vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &myFramebuffers[i]), "Failed to create a framebuffer!");
		}
	}

	void VulkanSwapChain::PrepareUniformBuffers()
	{
		myUniform.Create(sizeof(UBOVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Persistent map
		myUniform.Map();
		UBOVS ubovs;
		float proj[16] = {
			2.92f, 0.f, 0.f, 0.f,
			0.f, 1.73f, 0.f, 0.f,
			0.f, 0.f, -1.f, -1.f,
			0.f, 0.f, -0.1f, 0.f
		};
		float view[16] = {
			0.97f, -0.1f, -0.23f, 0.f,
			0.f, 0.91f, -0.42f, 0.f,
			0.26f, 0.41f, 0.88f, 0.f,
			0.f, 0.f, -10.5f, 1.f
		};
		ubovs.myProjection = glm::make_mat4(proj);
		ubovs.myModelView = glm::make_mat4(view);
		ubovs.myLightPos = glm::vec4(0.0f, 2.0f, 1.0f, 0.0f);
		memcpy(myUniform.myMappedData, &ubovs, sizeof(ubovs));

		myUniform.SetupDescriptor();
	}

	void VulkanSwapChain::SetupDescriptorSetLayout()
	{
		// Binding 0 : Vertex shader uniform buffer
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.binding = 0;
		layoutBinding.descriptorCount = 1;

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { layoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = (uint32_t)layoutBindings.size();
		descriptorLayoutInfo.pBindings = layoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(myDevice, &descriptorLayoutInfo, nullptr, &myDescriptorSetLayout), "Failed to create a descriptor set layout");
	}

	void VulkanSwapChain::PreparePipelines()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &myDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		VK_CHECK_RESULT(vkCreatePipelineLayout(myDevice, &pipelineLayoutCreateInfo, nullptr, &myPipelineLayout), "Failed to create a pipeline layout");

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizerState{};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = VK_FALSE;
		rasterizerState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		shaderStages[0] = {};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1] = {};
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Color });
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		// Create the graphics pipeline state objects

		// Phong shading pipeline
		VkShaderModule phongVert = CreateShaderModule("Frameworks/shaders/phong.vert.spv");
		shaderStages[0].module = phongVert;
		VkShaderModule phongFrag = CreateShaderModule("Frameworks/shaders/phong.frag.spv");
		shaderStages[1].module = phongFrag;

		// We are using this pipeline as the base for the other pipelines (derivatives)
		pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myPhongPipeline), "Failed to create the Phong pipeline");

		vkDestroyShaderModule(myDevice, phongVert, nullptr);
		vkDestroyShaderModule(myDevice, phongFrag, nullptr);

		// Base pipeline will be our first created pipeline
		pipelineInfo.basePipelineIndex = -1; // It's only allowed to either use a handle or index for the base pipeline
		pipelineInfo.basePipelineHandle = myPhongPipeline;

		// All pipelines created after the base pipeline will be derivatives
		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;

		// Toon shading pipeline
		VkShaderModule toonVert = CreateShaderModule("Frameworks/shaders/toon.vert.spv");
		shaderStages[0].module = toonVert;
		VkShaderModule toonFrag = CreateShaderModule("Frameworks/shaders/toon.frag.spv");
		shaderStages[1].module = toonFrag;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myToonPipeline), "Failed to create the Toon pipeline");

		vkDestroyShaderModule(myDevice, toonVert, nullptr);
		vkDestroyShaderModule(myDevice, toonFrag, nullptr);

		// Pipeline for wire frame rendering
		// Non solid rendering is not a mandatory Vulkan feature
		if (VulkanRenderCore::GetInstance()->GetVulkanDevice()->myEnabledFeatures.fillModeNonSolid)
		{
			rasterizerState.polygonMode = VK_POLYGON_MODE_LINE;

			VkShaderModule wireframeVert = CreateShaderModule("Frameworks/shaders/wireframe.vert.spv");
			shaderStages[0].module = wireframeVert;
			VkShaderModule wireframeFrag = CreateShaderModule("Frameworks/shaders/wireframe.frag.spv");
			shaderStages[1].module = wireframeFrag;

			VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myWireFramePipeline), "Failed to create the Wireframe pipeline");

			vkDestroyShaderModule(myDevice, wireframeVert, nullptr);
			vkDestroyShaderModule(myDevice, wireframeFrag, nullptr);
		}
	}

	void VulkanSwapChain::SetupDescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = 1;
		std::vector<VkDescriptorPoolSize> poolSizes = { descriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanSwapChain::SetupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = &myDescriptorSetLayout;
		descriptorSetAllocateInfo.descriptorSetCount = 1;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor pool");

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myUniform.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VulkanSwapChain::BuildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = myRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = myExtent.width;
		renderPassBeginInfo.renderArea.extent.height = myExtent.height;
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkglTF::Model* dummyScene = VulkanRenderCore::GetInstance()->GetDummyScene();

		for (int32_t i = 0; i < myCommandBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = myFramebuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(myCommandBuffers[i], &cmdBufferBeginInfo), "Failed to begin a command buffer");

			vkCmdBeginRenderPass(myCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)myExtent.width;
			viewport.height = (float)myExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent = myExtent;
			scissor.offset = { 0, 0 };
			vkCmdSetScissor(myCommandBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayout, 0, 1, &myDescriptorSet, 0, NULL);
			dummyScene->bindBuffers(myCommandBuffers[i]);

			// Left : Phong
			viewport.width = (float)myExtent.width / 3.0f;
			vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);
			vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPhongPipeline);
			dummyScene->draw(myCommandBuffers[i]);

			// Center : Toon
			viewport.x += (float)myExtent.width / 3.0f;
			vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);
			vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myToonPipeline);
			// Line width > 1.0f only if wide lines feature is supported
			if (VulkanRenderCore::GetInstance()->GetVulkanDevice()->myEnabledFeatures.wideLines)
				vkCmdSetLineWidth(myCommandBuffers[i], 2.0f);
			dummyScene->draw(myCommandBuffers[i]);

			if (VulkanRenderCore::GetInstance()->GetVulkanDevice()->myEnabledFeatures.fillModeNonSolid)
			{
				// Right : Wireframe
				viewport.x += (float)myExtent.width / 3.0f;
				vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);
				vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myWireFramePipeline);
				dummyScene->draw(myCommandBuffers[i]);
			}

			vkCmdEndRenderPass(myCommandBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(myCommandBuffers[i]), "Failed to end a command buffer");
		}
	}

	void VulkanSwapChain::Draw()
	{
		uint32_t index = 0;
		VkResult result = vkAcquireNextImageKHR(myDevice, myVkSwapChain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &index);
		assert(result == VK_SUCCESS);

		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &submitPipelineStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[index];

		result = vkQueueSubmit(VulkanRenderCore::GetInstance()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &myVkSwapChain;
		presentInfo.pImageIndices = &index;

		result = vkQueuePresentKHR(VulkanRenderCore::GetInstance()->GetGraphicsQueue(), &presentInfo);
		assert(result == VK_SUCCESS);

		/*submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));*/
	}
}
