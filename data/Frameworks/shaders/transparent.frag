#version 450

layout (set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput samplerPositionDepth;
layout (set = 2, binding = 2) uniform sampler2D samplerTexture;

layout (std430, set = 2, binding = 3) readonly buffer MaterialData {
	vec4 color;
} materialData;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main () 
{
	// Sample depth from deferred depth buffer and discard if obscured
	float depth = subpassLoad(samplerPositionDepth).a;

	// Save the sampled texture color before discarding.
	// This is to avoid implicit derivatives in non-uniform control flow.
	vec4 sampledColor = texture(samplerTexture, inUV);
	if ((depth != 0.0) && (linearDepth(gl_FragCoord.z) > depth))
	{
		discard;
	};

	outColor = inColor * sampledColor * materialData.color;
}