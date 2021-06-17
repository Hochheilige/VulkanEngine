#version 450

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform Scene {
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} scene;

void main() {
	outColor = vec4(color + scene.ambientColor.xyz, 1.0f);
}