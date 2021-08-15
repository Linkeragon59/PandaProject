#pragma once

namespace GameWork
{
	class Entity
	{
	public:
		Entity() {}
		virtual ~Entity() {}

		virtual void Update() = 0;

		// Position - Translation
		void SetPosition(const glm::vec3& aPosition);
		void Translate(const glm::vec3& aTranslation);
		glm::vec3 GetPosition() const { return myPosition; }

		// Orientation - Rotation
		// Angles in degrees
		void SetOrientation(const glm::vec3& someEulerAngles);
		void SetOrientation(float anAngle, const glm::vec3& anAxis);
		void Rotate(const glm::vec3& someEulerAngles);
		void Rotate(float anAngle, const glm::vec3& anAxis);
		glm::quat GetOrientation() const { return myOrientation; }

		// Scale - Scaling
		void SetScale(float aScale);
		void SetScale(glm::vec3 aScale);
		void Scale(float aScaleMultiplier);
		void Scale(glm::vec3 aScaleMultiplier);
		glm::vec3 GetScale() const { return myScale; }

		glm::mat4 GetMatrix() const;

	protected:
		glm::vec3 myPosition = glm::vec3(0.0f);
		glm::quat myOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 myScale = glm::vec3(1.0f);
	};
}
