#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

#define vkInt uint32_t

struct QueueFamilyIndices {
	std::optional<vkInt> graphicsFamily;
	std::optional<vkInt> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VkScene
{
public:
	void Play();
protected:
#pragma region Methods
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
#pragma endregion
	//0x00007ffdf7908310
#pragma region ProtectedVar
	const int WIDTH = 800;
	const int HEIGHT = 600;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidation = false;
#else
	const bool enableValidation = true;
#endif
#pragma endregion
private:
#pragma region Functions
	void initWindow();

	void initVulkan();

	void mainLoop();

	void cleanup();

	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void selectPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();
#pragma endregion

#pragma region SwapChainMethods
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice _dev);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkResult CreateSwapChain(VkInstance instance,
		VkDevice device,
		VkSwapchainCreateInfoKHR* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkSwapchainKHR* pSwapchain);
#pragma endregion

#pragma region PhysicalDeviceMethods
	int deviceSuitability(VkPhysicalDevice _dev);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

	bool checkDeviceExtensionSupport(VkPhysicalDevice _dev);
#pragma endregion

#pragma region Variables
	GLFWwindow* window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSurfaceKHR surf;
	VkSwapchainKHR swapChain;
#pragma endregion
};

