#pragma once

namespace Render
{
	struct PointLight
	{
		glm::vec4 myPosition = glm::vec4(0.0f);
		glm::vec4 myColor = glm::vec4(0.0f); // alpha channel used for intensity
	};
}
