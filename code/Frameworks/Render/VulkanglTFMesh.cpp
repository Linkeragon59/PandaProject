#include "VulkanglTFMesh.h"

#include "VulkanRenderCore.h"

namespace Render
{
namespace VulkanglTF
{
	Mesh::Mesh()
	{
		myUniformBuffer.Create(
			sizeof(myUniformData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUniformBuffer.Map();
		myUniformBuffer.SetupDescriptor();
	}

	Mesh::~Mesh()
	{
		myUniformBuffer.Destroy();
	}
}
}
