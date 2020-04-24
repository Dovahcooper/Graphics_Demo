#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D u_texture;

layout(location = 0) out vec4 fragColour;

layout(location = 0) in struct VertexData{
    vec4 pos;
    vec4 colour;
    vec3 normal;
    vec2 texCoord;
} vIn;

void main(){
    fragColour = texture(u_texture, vIn.texCoord).rgba;
}