#pragma once

namespace Render
{
	struct PointLight
	{
		glm::vec4 myPosition;
		glm::vec4 myColor; // alpha channel used for intensity
	};
}
