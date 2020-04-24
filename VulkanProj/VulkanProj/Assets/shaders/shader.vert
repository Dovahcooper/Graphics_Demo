#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormals;

layout(location = 0) out struct VertexData{
    vec4 pos;
    vec4 colour;
    vec3 normal;
    vec2 texCoord;
} vOut;

void main(){
    vOut.pos = ubo.proj * ubo.view * ubo.model * inPos;
    gl_Position = vOut.pos;

    vOut.colour = inCol;

    vOut.texCoord = inUV;
    vOut.normal = inNormals;
}