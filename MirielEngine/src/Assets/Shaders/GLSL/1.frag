#version 460 core
in vec3 oNorm;
in vec3 oColor;
in vec2 oTexCoord;

out vec4 fragColor;

uniform sampler2D oTexture;

void main() {
	//fragColor = (vec4(abs(oUseTex - 1)) + texture(oTexture, oTexCoord)) * vec4(oNorm, 1.0);
	fragColor = vec4(oNorm, 1.0);
}