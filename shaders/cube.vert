#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform Camera {
	mat4 view;
	mat4 projection;
	mat4 clip;
} camera;

layout (push_constant) uniform pushConstants {
	mat4 model;
} constants;

void main()
{
	gl_Position = camera.clip * camera.projection * camera.view * constants.model * vec4(vPosition, 1.0f);
	outColor = vColor;
}