#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VkScene
{
public:
	void Play();
protected:
	const int WIDTH = 800;
	const int HEIGHT = 600;
private:
#pragma region Functions
	void initWindow();

	void initVulkan();

	void mainLoop();

	void cleanup();

	void createInstance();
#pragma endregion

#pragma region Variables
	GLFWwindow* window;

	VkInstance instance;
#pragma endregion
};

