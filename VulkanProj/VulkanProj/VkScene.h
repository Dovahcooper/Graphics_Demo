#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

struct QueueFamily {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
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
#pragma endregion

#pragma region ProtectedVar
	const int WIDTH = 800;
	const int HEIGHT = 600;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
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

	void selectPhysicalDevice();

	void createLogicalDevice();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();
#pragma endregion

#pragma region PhysicalDeviceMethods
	int deviceSuitability(VkPhysicalDevice _dev);

	QueueFamily findQueueFamilies(VkPhysicalDevice);
#pragma endregion

#pragma region Variables
	GLFWwindow* window;

	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice;
#pragma endregion
};

