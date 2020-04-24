#pragma once
#include <glm/glm.hpp>
#include <vector>

class Mesh
{
public:

private:
	std::vector<glm::vec4> vertData;
	std::vector<glm::vec4> colData;
	std::vector<glm::vec2> uvData;
	std::vector<glm::vec3> normalData;
};

