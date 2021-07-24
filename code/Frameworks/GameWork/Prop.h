#pragma once

#include <string>

namespace Render
{
	class Model;
}

namespace GameWork
{
	class Prop
	{
	public:
		Prop(const char* aFilepath, const glm::vec3& aPosition = glm::vec3(0.0f), const glm::vec3& anOrientation = glm::vec3(0.0f), float aScale = 1.0f);
		~Prop();

		void Update();

		void Spawn();
		void Unspawn();

		void Hide() { myIsVisible = false; }
		void Show() { myIsVisible = true; }

		void SetPosition(const glm::vec3& aPosition) { myPosition = aPosition; }
		glm::vec3 GetPosition() const { return myPosition; }

		void SetOrientation(const glm::vec3& anOrientation) { myOrientation = anOrientation; }
		glm::vec3 GetOrientation() const { return myOrientation; }

		void SetScale(float aScale) { myScale = aScale; }
		float GetScale() const { return myScale; }

	private:
		std::string myFilepath;
		Render::Model* myModel;

		bool myIsSpawned;
		bool myIsVisible;

		glm::vec3 myPosition;
		glm::vec3 myOrientation;
		float myScale;
	};
}
