#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <optional>
#include <fstream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#define vkInt uint32_t
#define MAX_FRAMES_IN_FLIGHT 2
#define WINDOW_NAME "Vulkan Demo"

struct UniformBufferObj {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Vertex {
	glm::vec4 pos;
	glm::vec4 col;
	glm::vec2 texCoord;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindDescription() {
		VkVertexInputBindingDescription bindDescription{};
		bindDescription.binding = 0;
		bindDescription.stride = sizeof(Vertex);
		bindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttribDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attribDescriptions;

		attribDescriptions[0].binding = 0;
		attribDescriptions[0].location = 0;
		attribDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDescriptions[0].offset = offsetof(Vertex, pos);

		attribDescriptions[1].binding = 0;
		attribDescriptions[1].location = 1;
		attribDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDescriptions[1].offset = offsetof(Vertex, col);

		attribDescriptions[2].binding = 0;
		attribDescriptions[2].location = 2;
		attribDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attribDescriptions[2].offset = offsetof(Vertex, texCoord);

		attribDescriptions[3].binding = 0;
		attribDescriptions[3].location = 3;
		attribDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescriptions[3].offset = offsetof(Vertex, normal);

		return attribDescriptions;
	}
};

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
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	vkInt findMemType(vkInt filter, VkMemoryPropertyFlags properties);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, 
		VkBuffer& buffer,	VkDeviceMemory& bufferMemory);

	void cpyBuf(VkBuffer srcBuf, VkBuffer dstBuf, VkDeviceSize size);

	void createImage(vkInt width, vkInt height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);

	void transitionImgLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);

	void copyBufferToImage(VkBuffer, VkImage, vkInt width, vkInt height);

	VkImageView createImageView(VkImage img, VkFormat form, VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	bool hasStencilFormat(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkCommandBuffer beginSingleCommands();
	void endSingleCommand(VkCommandBuffer);
#pragma endregion
	//0x00007ffdf7908310
#pragma region ProtectedVar
	const int WIDTH = 800;
	const int HEIGHT = 600;

	int currentFrame = 0;
	bool frameBufferResize = false;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<Vertex> vertices = {
		{{-1.f, -1.f, 0.0f, 1.0f}, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f}, {0.0f, 0.0f, 0.0f}},
		{{1.f, -1.f, 0.0f, 1.0f}, {0.f, 1.f, 0.f, 1.f}, {1.f, 0.f}, {0.0f, 0.0f, 0.0f}},
		{{1.f, 1.f, 0.0f, 1.0f}, {0.f, 0.f, 1.f, 1.f}, {1.f, 1.f}, {0.0f, 0.0f, 0.0f}},
		{{-1.f, 1.f, 0.f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.f}, {0.0f, 0.0f, 0.0f}},

		{{-1.f, -1.f, -0.5f, 1.0f}, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f}, {0.0f, 0.0f, 0.0f}},
		{{1.f, -1.f, -0.5f, 1.0f}, {0.f, 1.f, 0.f, 1.f}, {1.f, 0.f}, {0.0f, 0.0f, 0.0f}},
		{{1.f, 1.f, -0.5f, 1.0f}, {0.f, 0.f, 1.f, 1.f}, {1.f, 1.f}, {0.0f, 0.0f, 0.0f}},
		{{-1.f, 1.f, -0.5f, 1.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.f}, {0.0f, 0.0f, 0.0f}},
	};
	const std::vector<vkInt> indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
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

	void createImgViews();

	void createRenderPasses(); //does this excite you? making a fool of me, tutorial?????

	void createDescriptorSetLayout();

	void createGraphicsPipeline(); //YES, FINALLY, THAT GOOD SHITTTTTTTTTT

	void createFramebuffers();

	void createCommandPool();

	void createDepthResources();

	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();

	void createVBO();

	void createIBO();

	void createUBO();

	void createDescriptorPool();
	void createDescriptorSets();

	void createCommandBuffers();

	void createSyncObjects();

	void drawFrame();

	void updateUBO(vkInt);

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();
#pragma endregion

#pragma region PipelineMethods
	VkShaderModule createShaderModule(const std::vector<char>& code);


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

	void recreateSwapchain();

	void cleanupSwapchain();
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
	std::vector<VkImage> swapChainImg;
	VkFormat swapChainImgForm;
	VkExtent2D swapChainEXT;
	std::vector<VkImageView> swapChainImgViews;

	VkRenderPass renderPass;
	VkViewport viewport;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipeLayout;
	VkPipeline graphicsPipeline;

	std::vector<VkFramebuffer> frameBuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imgAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> inFlightFences; //by Paramore
	std::vector<VkFence> imgInFlight;

	VkBuffer vertBuffer;
	VkBuffer indexBuffer;
	VkDeviceMemory vboMemory;
	VkDeviceMemory iboMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uboMemories;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSetList;

	VkImage textureImg;
	VkDeviceMemory texImgMemory;
	VkImageView texImgView;

	VkSampler texSampler;

	VkImage depthImg;
	VkDeviceMemory depthImgMemory;
	VkImageView depthImgView;

#pragma endregion
};