#pragma once

namespace Render
{
	struct Camera
	{
		glm::mat4 myView = glm::mat4(1.0f);
		glm::mat4 myPerspective = glm::mat4(1.0f);
	};
}
