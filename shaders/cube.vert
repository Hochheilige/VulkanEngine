#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform constants {
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 clip;
} pushConstants;

void main()
{
	gl_Position = pushConstants.clip * pushConstants.projection * pushConstants.view * pushConstants.model * vec4(vPosition, 1.0f);
	outColor = vColor;
}