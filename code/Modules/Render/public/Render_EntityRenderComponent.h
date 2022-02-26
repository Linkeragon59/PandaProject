#pragma once

#include "GameCore_Entity.h"

namespace Render
{
	class Renderer;
	class Model;

	struct EntityRenderComponent
	{
		virtual void Load() {}
		void Unload();
		void Update();
		void Draw(Renderer* aRenderer);

		glm::mat4 myMatrix = glm::mat4(1.0f);
		bool myIsTransparent = false;

		Model* myModel = nullptr;
	};

	struct EntitySimpleGeometryModelComponent : EntityRenderComponent
	{
		void Load() override;

		struct Vertex
		{
			glm::vec3 myPosition;
			glm::vec3 myNormal;
			glm::vec2 myUV;
			glm::vec4 myColor;
		};

		enum class Preset
		{
			VectorBaseWidget,
			Square,
			Cube,
			Disc,
			Sphere,
			Panda,
		};
		void FillWithPreset(Preset aPreset);
		void FillVectorBaseWidget();
		void FillSquare();
		void FillCube();
		void FillDisc();
		void FillSphere();
		void FillPanda();

		std::vector<Vertex> myVertices;
		std::vector<uint> myIndices;
		std::string myTextureFilename;
	};

	struct EntityglTFModelComponent : EntityRenderComponent
	{
		void Load() override;

		std::string myFilename;
		bool myIsAnimated = false;
	};
}
