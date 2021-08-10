#pragma once

namespace GameWork
{
	class Entity
	{
	public:
		Entity() {}
		virtual ~Entity() {}

		virtual void Update() = 0;

		void SetPosition(const glm::vec3& aPosition);
		void Translate(const glm::vec3& aPositionDelta);
		glm::vec3 GetPosition() const { return myPosition; }

		void SetRotation(const glm::vec3& aRotation);
		void Rotate(const glm::vec3& aRotationDelta);
		glm::vec3 GetRotation() const { return myRotation; }

	protected:
		virtual void OnPositionChanged() = 0;
		virtual void OnRotationChanged() = 0;

		glm::vec3 myPosition = glm::vec3();
		glm::vec3 myRotation = glm::vec3();
	};
}
