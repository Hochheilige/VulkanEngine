#version 450

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.3f, 0.7f, 0.5f, 1.0f);
}