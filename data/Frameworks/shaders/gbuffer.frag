#version 450

layout (set = 2, binding = 0) uniform sampler2D samplerTexture;

layout (std430, set = 3, binding = 0) readonly buffer MaterialData {
	vec4 color;
} materialData;

layout (location = 0) in vec3 inWorldPosition;
layout (location = 1) in vec3 inWorldNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{
	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0.0);
	
	// Store linearized depth in alpha component
	outPosition = vec4(inWorldPosition, linearDepth(gl_FragCoord.z));
	
	outNormal = vec4(normalize(inWorldNormal), 1.0);
	
	vec4 sampledColor = texture(samplerTexture, inUV);
	outAlbedo = inColor * sampledColor * materialData.color;
}