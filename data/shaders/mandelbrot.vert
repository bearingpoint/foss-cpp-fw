attribute vec2 position;

uniform vec3 transform;	// (x,y) is translation, z is scale
uniform float aspectRatio; // w/h of viewport

// Output data ; will be interpolated for each fragment.
varying vec2 outUV;

void main() {
    gl_Position = vec4(position, 0.5, 1);
	vec2 aspectCorection = vec2(aspectRatio, 1.0);
	vec2 transformedPos = position * aspectCorection * transform.z + transform.xy;
    outUV = transformedPos;
}
