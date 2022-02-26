#pragma once

#include "GameCore_Module.h"
#include "GameCore_EntityComponentSystem.h"

namespace GameCore
{
	class EntityModule : public Module
	{
	DECLARE_GAMECORE_MODULE(EntityModule, "Entity")

	public:
		inline ECS::EntityManager* GetEntityManager() const { return myEntityManager; }
		inline ECS::ComponentManager* GetComponentManager() const { return myComponentManager; }

	protected:
		void OnRegister() override;
		void OnUnregister() override;

	private:
		ECS::EntityManager* myEntityManager = nullptr;
		ECS::ComponentManager* myComponentManager = nullptr;
	};

	class Entity3DTransformComponent
	{
	public:
		Entity3DTransformComponent(const glm::vec3& aPosition)
			: myPosition(aPosition)
		{}

		// Position - Translation
		void SetPosition(const glm::vec3& aPosition) { myPosition = aPosition; }
		void Translate(const glm::vec3& aTranslation) { myPosition += aTranslation; }
		glm::vec3 GetPosition() const { return myPosition; }

		// Orientation - Rotation
		// Angles in degrees
		void SetOrientation(const glm::vec3& someEulerAngles) { myOrientation = glm::quat(glm::radians(someEulerAngles)); }
		void SetOrientation(float anAngle, const glm::vec3& anAxis) { myOrientation = glm::angleAxis(glm::radians(anAngle), anAxis); }
		void Rotate(const glm::vec3& someEulerAngles) { myOrientation = glm::quat(glm::radians(someEulerAngles)) * myOrientation; }
		void Rotate(float anAngle, const glm::vec3& anAxis) { myOrientation = glm::angleAxis(glm::radians(anAngle), anAxis) * myOrientation; }
		glm::quat GetOrientation() const { return myOrientation; }

		// Scale - Scaling
		void SetScale(float aScale) { myScale = glm::vec3(aScale); }
		void SetScale(glm::vec3 aScale) { myScale = aScale; }
		void Scale(float aScaleMultiplier) { myScale *= aScaleMultiplier; }
		void Scale(glm::vec3 aScaleMultiplier) { myScale *= aScaleMultiplier; }
		glm::vec3 GetScale() const { return myScale; }

		// Result Transform Matrix
		glm::mat4 GetMatrix() const;

	private:
		glm::vec3 myPosition = glm::vec3(0.0f);
		glm::quat myOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 myScale = glm::vec3(1.0f);
	};
}
