#pragma once

namespace Render
{
	struct glTFModelData
	{
		std::string myFilename;
		glm::mat4 myMatrix = glm::mat4(1.0f);
		bool myIsTransparent = false;
		bool myIsAnimated = false;
	};

	class Model
	{
	public:
		virtual ~Model() {};
		virtual void Update() = 0;
	};
}