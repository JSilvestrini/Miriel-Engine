#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

layout (std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};

out vec3 oNorm;
out vec3 oColor;
out vec2 oTexCoord;

uniform mat4 model;

void main() {
	mat4 mvp = projection * view * model;
	oNorm = vec3((mvp * vec4(aNorm,1.0)).xyz);
	oColor = aColor;
	oTexCoord = aTexCoord;
	gl_Position = mvp * vec4(aPos, 1.0);
}