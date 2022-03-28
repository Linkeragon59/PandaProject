#pragma once

#include "GameCore_Entity.h"

namespace Render
{
	class Renderer;
	class Model;
	class Gui;

	struct EntityModelComponent
	{
		virtual ~EntityModelComponent();

		virtual void Load() = 0;
		void Unload();
		void Update(const glm::mat4& aMatrix);

		Model* GetModel() const { return myModel; }

		bool myIsTransparent = false;

		Model* myModel = nullptr;
	};

	struct EntitySimpleGeometryModelComponent : EntityModelComponent
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

	struct EntityglTFModelComponent : EntityModelComponent
	{
		void Load() override;

		std::string myFilename;
		bool myIsAnimated = false;
	};

	struct EntityGuiComponent
	{
		EntityGuiComponent();
		~EntityGuiComponent();

		void Load();
		void Unload();
		void Update();

		Gui* GetGui() const { return myGui; }

		Gui* myGui = nullptr;
	};
}
