#include "VkScene.h"
#include <cstdlib>
#include <stdexcept>
#include <map>

void VkScene::Play() {
	initVulkan();
	mainLoop();
	cleanup();
}

void VkScene::initVulkan() {
	initWindow();
	createInstance();
	selectPhysicalDevice();
	createLogicalDevice();
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

	uint32_t glfwExtCount = 0;
	const char** glfwExt;

	glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	VkResult err = vkCreateInstance(&createInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Instance");
	}

	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

	std::vector<VkExtensionProperties> vkExtensions(extCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, vkExtensions.data());

	printf("Available Extensions: \n");
	for (auto ext : vkExtensions) {
		printf("\t%s\n", ext.extensionName);
	}
}

void VkScene::selectPhysicalDevice()
{
	uint32_t deviceCount;

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
	QueueFamily indices = findQueueFamilies(physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;

	if (enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Problem creating Logical Device");
	}
}

bool VkScene::checkValidationLayerSupport()
{
	uint32_t layerCount;
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
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
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
		score = 0;

	QueueFamily family = findQueueFamilies(_dev);
	if (family.graphicsFamily.has_value())
		score += 1000;

	printf("\t%s %d\n", deviceProperties.deviceName, score);

	return score;
}

QueueFamily VkScene::findQueueFamilies(VkPhysicalDevice _dev)
{
	QueueFamily fam;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_dev, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> famProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_dev, &queueFamilyCount, famProperties.data());

	int i = 0;
	for (const auto& itr : famProperties) {
		if (itr.queueFlags == VK_QUEUE_GRAPHICS_BIT)
			fam.graphicsFamily = i;

		if (fam.isComplete())
			break;

		i++;
	}

	return fam;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkScene::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	printf("Validation Layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void VkScene::cleanup() {
	vkDestroyDevice(logicalDevice, nullptr);

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}