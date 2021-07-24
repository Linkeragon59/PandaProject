#pragma once

namespace Render
{
	class Model
	{
	public:
		struct RenderData
		{
			glm::mat4 myMatrix = glm::mat4(1.0f);

			bool myIsTransparent = false;
			bool myIsAnimated = false;
		};

		Model(const RenderData& someRenderData)
		{
			myRenderData = someRenderData;
		}

		virtual ~Model() {};

		virtual void Update() = 0;

		glm::vec3 GetPosition() const { return myRenderData.myMatrix[3]; }

		bool IsTransparent() const { return myRenderData.myIsTransparent; }

	protected:
		RenderData myRenderData;
	};
}