#pragma once
#include "GameCore_Module.h"
#include "GameCore_Entity.h"

#include <set>
#include <map>

namespace GameCore
{
	class Entity3DTransformComponent
	{
	public:
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

	protected:
		friend EntityHandle;
		static Entity3DTransformComponent* GetComponent(EntityHandle aHandle);
		static Entity3DTransformComponent* AddComponent(EntityHandle aHandle);
		static void RemoveComponent(EntityHandle aHandle);

	private:
		glm::vec3 myPosition = glm::vec3(0.0f);
		glm::quat myOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 myScale = glm::vec3(1.0f);
	};

	class EntityModule : public Module
	{
	DECLARE_GAMECORE_MODULE(EntityModule, "Entity")

	public:
		EntityHandle Create();
		void Destroy(EntityHandle aHandle);
		bool Exists(EntityHandle aHandle);

		Entity3DTransformComponent* GetPositionComponent(EntityHandle aHandle);
		Entity3DTransformComponent* AddPositionComponent(EntityHandle aHandle);
		void RemovePositionComponent(EntityHandle aHandle);

	private:
		std::set<EntityHandle> myUsedEntityIds;
		std::set<EntityHandle> myFreeEntityIds;

		std::map<EntityHandle, Entity3DTransformComponent> myPositionComponents;
	};
}
