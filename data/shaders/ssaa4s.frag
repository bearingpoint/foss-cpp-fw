#version 330 core

in vec2 fragUV;

uniform sampler2D frameBufferTexture;
uniform vec2 sampleOffsets[4];

out vec4 color;

void main(){
	vec3 color0 = texture2D(frameBufferTexture, fragUV + sampleOffsets[0]);
	vec3 color1 = texture2D(frameBufferTexture, fragUV + sampleOffsets[1]);
	vec3 color2 = texture2D(frameBufferTexture, fragUV + sampleOffsets[2]);
	vec3 color3 = texture2D(frameBufferTexture, fragUV + sampleOffsets[3]);

	color = vec4((color0+color1+color2+color3) * 0.25, 1.0);
}
