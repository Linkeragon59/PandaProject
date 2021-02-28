#include "VulkanRenderCore.h"

#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <assert.h>
#include <set>

namespace Render
{
	namespace
	{
		uint32_t locVulkanApiVersion = VK_API_VERSION_1_0;

#if defined(_WINDOWS) && !defined(NDEBUG)
		constexpr bool locEnableValidationLayers = true;
#else
		constexpr bool locEnableValidationLayers = false;
#endif
	}
	
	VulkanRenderCore* VulkanRenderCore::ourInstance = nullptr;

	void VulkanRenderCore::CreateInstance()
	{
		assert(!ourInstance);
		
		glfwInit();
		
		ourInstance = new VulkanRenderCore();
	}

	void VulkanRenderCore::DestroyInstance()
	{
		assert(ourInstance);

		delete ourInstance;
		ourInstance = nullptr;

		glfwTerminate();
	}

	GLFWwindow* VulkanRenderCore::OpenWindow(int aWidth, int aHeight, const char* aTitle)
	{
		assert(ourInstance);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(aWidth, aHeight, aTitle, nullptr, nullptr);

		VulkanSwapChain* swapChain = new VulkanSwapChain(window);
		ourInstance->mySwapChains.push_back(swapChain);

		ourInstance->PrepareDummyScene();

		return window;
	}

	void VulkanRenderCore::CloseWindow(GLFWwindow* aWindow)
	{
		assert(ourInstance);

		for (uint32_t i = 0; i < (uint32_t)ourInstance->mySwapChains.size(); ++i)
		{
			if (ourInstance->mySwapChains[i]->GetWindow() == aWindow)
			{
				delete ourInstance->mySwapChains[i];
				ourInstance->mySwapChains.erase(ourInstance->mySwapChains.begin() + i);
			}
		}

		glfwDestroyWindow(aWindow);
	}

	VkPhysicalDevice VulkanRenderCore::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice VulkanRenderCore::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator VulkanRenderCore::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue VulkanRenderCore::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool VulkanRenderCore::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	VulkanRenderCore::VulkanRenderCore()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
			SetupDebugMessenger();

		CreateDevice();
		SetupRenderPass();
		CreatePipelineCache();
	}

	VulkanRenderCore::~VulkanRenderCore()
	{
		vkDestroyPipelineCache(myDevice->myLogicalDevice, myPipelineCache, nullptr);

		vkDestroyRenderPass(myDevice->myLogicalDevice, myRenderPass, nullptr);
		
		delete myDevice;

		if (locEnableValidationLayers)
			DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void VulkanRenderCore::CreateVkInstance()
	{
		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			PopulateValidationLayers(layers);

		if (!CheckInstanceLayersSupport(layers))
			throw std::runtime_error("Validation layers are enabled but not supported!");

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (locEnableValidationLayers)
			PopulateDebugExtensions(extensions);

		if (!CheckInstanceExtensionsSupport(extensions))
			throw std::runtime_error("Required extensions are not available!");

		// TODO: Have more parameters when creating the RenderCore
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
			FillDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &myVkInstance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan instance!");
	}

	void VulkanRenderCore::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		FillDebugMessengerCreateInfo(createInfo);

		if (CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger) != VK_SUCCESS)
			throw std::runtime_error("Couldn't create a debug messenger!");
	}

	void VulkanRenderCore::CreateDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("No physical device supporting Vulkan was found!");

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
			PopulateValidationLayers(layers);

		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		myDevice->SetupLogicalDevice(
			enabledFeatures,
			layers,
			extensions,
			VK_QUEUE_GRAPHICS_BIT);
		myDevice->SetupVmaAllocator(myVkInstance, locVulkanApiVersion);
	}

	void VulkanRenderCore::SetupRenderPass()
	{
		// TODO: We should likely have a version of this for each swapchain, so we can use the SwapChain format

		std::array<VkAttachmentDescription, 2> attachments;
		// Color attachment
		attachments[0] = {};
		attachments[0].format = VK_FORMAT_B8G8R8A8_SRGB; // This should match the swapchain
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1] = {};
		attachments[1].format = myDevice->FindBestDepthFormat();
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

		if (vkCreateRenderPass(myDevice->myLogicalDevice, &renderPassInfo, nullptr, &myRenderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the render pass!");
	}

	void VulkanRenderCore::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.initialDataSize = 0;

		if (vkCreatePipelineCache(myDevice->myLogicalDevice, &pipelineCacheCreateInfo, nullptr, &myPipelineCache) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the pipeline cache!");
	}

	void VulkanRenderCore::PrepareDummyScene()
	{
		LoadDummyScene();
		PrepareUniformBuffers();
		SetupDescriptorSetLayout();
		PreparePipelines();
		SetupDescriptorPool();
		SetupDescriptorSet();
		BuildCommandBuffers();

		// First draw
		Draw();
	}

	void VulkanRenderCore::LoadDummyScene()
	{
		const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
		myScene.loadFromFile("Frameworks/models/treasure_smooth.gltf", myDevice, myDevice->myGraphicsQueue, glTFLoadingFlags);
	}

	void VulkanRenderCore::PrepareUniformBuffers()
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
	}

	void VulkanRenderCore::SetupDescriptorSetLayout()
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

		if (vkCreateDescriptorSetLayout(myDevice->myLogicalDevice, &descriptorLayoutInfo, nullptr, &myDescriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a descriptor set layout");
	}

	void VulkanRenderCore::PreparePipelines()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &myDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(myDevice->myLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &myPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a pipeline layout");

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

		if (vkCreateGraphicsPipelines(myDevice->myLogicalDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myPhongPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the Phong pipeline");

		vkDestroyShaderModule(myDevice->myLogicalDevice, phongVert, nullptr);
		vkDestroyShaderModule(myDevice->myLogicalDevice, phongFrag, nullptr);

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

		if (vkCreateGraphicsPipelines(myDevice->myLogicalDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myToonPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the Toon pipeline");

		vkDestroyShaderModule(myDevice->myLogicalDevice, toonVert, nullptr);
		vkDestroyShaderModule(myDevice->myLogicalDevice, toonFrag, nullptr);

		// Pipeline for wire frame rendering
		// Non solid rendering is not a mandatory Vulkan feature
		if (myDevice->myEnabledFeatures.fillModeNonSolid)
		{
			rasterizerState.polygonMode = VK_POLYGON_MODE_LINE;

			VkShaderModule wireframeVert = CreateShaderModule("Frameworks/shaders/wireframe.vert.spv");
			shaderStages[0].module = wireframeVert;
			VkShaderModule wireframeFrag = CreateShaderModule("Frameworks/shaders/wireframe.frag.spv");
			shaderStages[1].module = wireframeFrag;

			if (vkCreateGraphicsPipelines(myDevice->myLogicalDevice, myPipelineCache, 1, &pipelineInfo, nullptr, &myWireFramePipeline) != VK_SUCCESS)
				throw std::runtime_error("Failed to create the Wireframe pipeline");

			vkDestroyShaderModule(myDevice->myLogicalDevice, wireframeVert, nullptr);
			vkDestroyShaderModule(myDevice->myLogicalDevice, wireframeFrag, nullptr);
		}
	}

	void VulkanRenderCore::SetupDescriptorPool()
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

		if (vkCreateDescriptorPool(myDevice->myLogicalDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the descriptor pool");
	}

	void VulkanRenderCore::SetupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = &myDescriptorSetLayout;
		descriptorSetAllocateInfo.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(myDevice->myLogicalDevice, &descriptorSetAllocateInfo, &myDescriptorSet) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the descriptor pool");
		
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myUniform.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice->myLogicalDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void VulkanRenderCore::BuildCommandBuffers()
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
		renderPassBeginInfo.renderArea.extent.width = mySwapChains[0]->myExtent.width;
		renderPassBeginInfo.renderArea.extent.height = mySwapChains[0]->myExtent.height;
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		for (int32_t i = 0; i < mySwapChains[0]->myCommandBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = mySwapChains[0]->myFramebuffers[i];

			if (vkBeginCommandBuffer(mySwapChains[0]->myCommandBuffers[i], &cmdBufferBeginInfo) != VK_SUCCESS)
				throw std::runtime_error("failed to begin a command buffer");

			vkCmdBeginRenderPass(mySwapChains[0]->myCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mySwapChains[0]->myExtent.width;
			viewport.height = (float)mySwapChains[0]->myExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(mySwapChains[0]->myCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent = mySwapChains[0]->myExtent;
			scissor.offset = { 0, 0 };
			vkCmdSetScissor(mySwapChains[0]->myCommandBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(mySwapChains[0]->myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayout, 0, 1, &myDescriptorSet, 0, NULL);
			myScene.bindBuffers(mySwapChains[0]->myCommandBuffers[i]);

			// Left : Phong
			viewport.width = (float)mySwapChains[0]->myExtent.width / 3.0f;
			vkCmdSetViewport(mySwapChains[0]->myCommandBuffers[i], 0, 1, &viewport);
			vkCmdBindPipeline(mySwapChains[0]->myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPhongPipeline);
			myScene.draw(mySwapChains[0]->myCommandBuffers[i]);

			// Center : Toon
			viewport.x += (float)mySwapChains[0]->myExtent.width / 3.0f;
			vkCmdSetViewport(mySwapChains[0]->myCommandBuffers[i], 0, 1, &viewport);
			vkCmdBindPipeline(mySwapChains[0]->myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myToonPipeline);
			// Line width > 1.0f only if wide lines feature is supported
			if (myDevice->myEnabledFeatures.wideLines)
				vkCmdSetLineWidth(mySwapChains[0]->myCommandBuffers[i], 2.0f);
			myScene.draw(mySwapChains[0]->myCommandBuffers[i]);

			if (myDevice->myEnabledFeatures.fillModeNonSolid)
			{
				// Right : Wireframe
				viewport.x += (float)mySwapChains[0]->myExtent.width / 3.0f;
				vkCmdSetViewport(mySwapChains[0]->myCommandBuffers[i], 0, 1, &viewport);
				vkCmdBindPipeline(mySwapChains[0]->myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myWireFramePipeline);
				myScene.draw(mySwapChains[0]->myCommandBuffers[i]);
			}

			vkCmdEndRenderPass(mySwapChains[0]->myCommandBuffers[i]);

			if (vkEndCommandBuffer(mySwapChains[0]->myCommandBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to end a command buffer");
		}
	}

	void VulkanRenderCore::Draw()
	{
		uint32_t index = 0;
		VkResult result = vkAcquireNextImageKHR(myDevice->myLogicalDevice, mySwapChains[0]->myVkSwapChain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &index);
		assert(result == VK_SUCCESS);

		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &submitPipelineStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mySwapChains[0]->myCommandBuffers[index];

		result = vkQueueSubmit(myDevice->myGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mySwapChains[0]->myVkSwapChain;
		presentInfo.pImageIndices = &index;

		result = vkQueuePresentKHR(myDevice->myGraphicsQueue, &presentInfo);
		assert(result == VK_SUCCESS);

		/*submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));*/
	}

}

