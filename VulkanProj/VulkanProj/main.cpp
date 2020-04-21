#include "VkScene.h"
#include <iostream>

int main() {
    VkScene* scene = new VkScene();

    try {
        scene->Play();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}