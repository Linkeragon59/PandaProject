#pragma once

namespace Render
{
struct RenderData
{
	glm::mat4 myMatrix = glm::mat4(1.0f);

	bool myIsTransparent = false;
	bool myIsAnimated = false;
};
}