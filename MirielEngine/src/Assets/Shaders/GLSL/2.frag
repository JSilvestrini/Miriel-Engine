#version 460 core
in vec3 oNorm;
in vec3 oColor;
in vec2 oTexCoord;

out vec4 fragColor;

uniform sampler2D oTexture;

// TODO: Define light struct in vert and this, then pass lights through vert

void main() {
	//fragColor = (vec4(abs(oUseTex - 1)) + texture(oTexture, oTexCoord)) * vec4(oNorm, 1.0);
	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}