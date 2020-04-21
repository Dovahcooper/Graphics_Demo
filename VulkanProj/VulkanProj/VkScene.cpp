#include "VkScene.h"
#include <cstdlib>
#include <stdexcept>
#include <vector>

void VkScene::Play() {
	initVulkan();
	mainLoop();
	cleanup();
}

void VkScene::initVulkan() {
	initWindow();
	createInstance();
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

void VkScene::cleanup() {
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void VkScene::createInstance()
{
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

	createInfo.enabledExtensionCount = glfwExtCount;
	createInfo.ppEnabledExtensionNames = glfwExt;

	createInfo.enabledLayerCount = 0;

	VkResult err = vkCreateInstance(&createInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Instance");
	}

	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions.data());

	printf("Available Extensions: \n");
	for (auto ext : extensions) {
		printf("\t%s\n", ext.extensionName);
	}
}
