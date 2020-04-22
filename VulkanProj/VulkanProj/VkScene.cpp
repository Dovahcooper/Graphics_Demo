#include "VkScene.h"
#include <cstdlib>
#include <stdexcept>
#include <map>
#include <set>

#define max(A, B) ((A) < (B) ? (B) : (A))
#define min(A, B) ((A) < (B) ? (A) : (B))

void VkScene::Play() {
	initVulkan();
	mainLoop();
	cleanup();
}

void VkScene::initVulkan() {
	initWindow();
	createInstance();
	setupDebugMessenger();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
}

void VkScene::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Demo", nullptr, nullptr);
}

void VkScene::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VkScene::createInstance()
{
	if (enableValidation && !checkValidationLayerSupport()) {
		throw std::runtime_error("Failed to load Validation Layers");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	vkInt glfwExtCount = 0;
	const char** glfwExt;

	glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<vkInt>(extensions.size());;
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidation) {
		createInfo.enabledLayerCount = static_cast<vkInt>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else
		createInfo.enabledLayerCount = 0;

	VkResult err = vkCreateInstance(&createInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Instance");
	}

	vkInt extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

	std::vector<VkExtensionProperties> vkExtensions(extCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, vkExtensions.data());

	printf("Available Extensions: \n");
	for (auto ext : vkExtensions) {
		printf("\t%s\n", ext.extensionName);
	}
}

void VkScene::setupDebugMessenger()
{
	if (!enableValidation) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to Create Debug Messenger");
	}
}

void VkScene::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkResult VkScene::CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT * pCreateInfo,
	const VkAllocationCallbacks * pAllocator,
	VkDebugUtilsMessengerEXT * pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, 
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VkScene::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surf) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Window Surface");
	}


}

void VkScene::selectPhysicalDevice()
{
	vkInt deviceCount;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("No devices with Vulkan Support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	printf("GPUs Found:\n");
	
	std::multimap<int, VkPhysicalDevice> deviceCandidates;

	for (const auto& itr : devices) {
		int score = deviceSuitability(itr);
		deviceCandidates.insert(std::make_pair(score, itr));
	}

	if (deviceCandidates.rbegin()->first > 0) {
		physicalDevice = deviceCandidates.rbegin()->second;
	}
	else
		throw std::runtime_error("No Suitable GPU");
}

void VkScene::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<vkInt> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	float queuePriority = 1.f;

	for (vkInt itr : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = itr;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfoList.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<vkInt>(queueCreateInfoList.size());
	createInfo.pQueueCreateInfos = queueCreateInfoList.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;

	if (enableValidation) {
		createInfo.enabledLayerCount = static_cast<vkInt>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Problem creating Logical Device");
	}

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void VkScene::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surf;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (CreateSwapChain(instance, logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	//vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
	//swapChainImages.resize(imageCount);
	//vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
	//
	//swapChainImageFormat = surfaceFormat.format;
	//swapChainExtent = extent;
}

bool VkScene::checkValidationLayerSupport()
{
	vkInt layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& itr : availLayers) {
			if (strcmp(layerName, itr.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

std::vector<const char*> VkScene::getRequiredExtensions() {
	vkInt glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

SwapChainSupportDetails VkScene::querySwapChainSupport(VkPhysicalDevice _dev)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_dev, surf, &details.capabilities);

	vkInt formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_dev, surf, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_dev, surf, &formatCount, details.formats.data());
	}

	vkInt presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_dev, surf, &presentModeCount, nullptr);

	if (formatCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_dev, surf, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VkScene::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VkScene::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkScene::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

		actualExtent.width = max(capabilities.minImageExtent.width,
			min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = max(capabilities.minImageExtent.height,
			min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkResult VkScene::CreateSwapChain(VkInstance instance,
	VkDevice device,
	VkSwapchainCreateInfoKHR* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkSwapchainKHR* pSwapchain)
{
	auto func = (PFN_vkCreateSwapchainKHR)vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");

	if (func != nullptr) {
		return func(device, pCreateInfo, pAllocator, pSwapchain);
	}
	else return VK_ERROR_UNKNOWN;
}

int VkScene::deviceSuitability(VkPhysicalDevice _dev)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(_dev, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(_dev, &deviceFeatures);

	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader)
		return 0;

	QueueFamilyIndices family = findQueueFamilies(_dev);
	if (!family.isComplete())
		return 0;

	bool extensionSupport = checkDeviceExtensionSupport(_dev);
	if (!extensionSupport)
		return 0;

	bool swapChainAdq = false;
	if (extensionSupport) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_dev);
		swapChainAdq = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	if (!swapChainAdq)
		return 0;

	printf("\t%s scored: %d\n", deviceProperties.deviceName, score);

	return score;
}

QueueFamilyIndices VkScene::findQueueFamilies(VkPhysicalDevice _dev)
{
	QueueFamilyIndices fam;

	vkInt queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_dev, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> famProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_dev, &queueFamilyCount, famProperties.data());

	int i = 0;
	for (const auto& itr : famProperties) {
		if (itr.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			fam.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(_dev, i, surf, &presentSupport);

		if (presentSupport) {
			fam.presentFamily = i;
		}

		if (fam.isComplete()) {
			break;
		}

		i++;
	}

	return fam;
}

bool VkScene::checkDeviceExtensionSupport(VkPhysicalDevice _dev)
{
	vkInt extCount = 0;
	vkEnumerateDeviceExtensionProperties(_dev, nullptr, &extCount, nullptr);
	std::vector<VkExtensionProperties> availExt(extCount);
	vkEnumerateDeviceExtensionProperties(_dev, nullptr, &extCount, availExt.data());

	std::set<std::string> requireExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& itr : availExt) {
		requireExtensions.erase(itr.extensionName);
	}

	return requireExtensions.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkScene::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		printf("Validation Layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void VkScene::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VkScene::cleanup() {
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

	vkDestroyDevice(logicalDevice, nullptr);

	vkDestroySurfaceKHR(instance, surf, nullptr);

	if (enableValidation) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}