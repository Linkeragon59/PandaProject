#pragma once

namespace Render
{
	struct ModelData
	{
		virtual ~ModelData() {}
		enum class Type
		{
			glTF,
			SimpleGeometry,
		};
		virtual Type GetType() const = 0;

		glm::mat4 myMatrix = glm::mat4(1.0f);
		bool myIsTransparent = false;
	};

	struct glTFModelData : ModelData
	{
		Type GetType() const override { return Type::glTF; }

		std::string myFilename;
		bool myIsAnimated = false;
	};

	struct SimpleGeometryModelData : ModelData
	{
		Type GetType() const override { return Type::SimpleGeometry; }

		struct Vertex
		{
			glm::vec3 myPosition;
			glm::vec3 myNormal;
			glm::vec2 myUV;
			glm::vec4 myColor;
		};
		std::vector<Vertex> myVertices;
		std::vector<uint> myIndices;
		std::string myTextureFilename;

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

	private:
		void FillVectorBaseWidget();
		void FillSquare();
		void FillCube();
		void FillDisc();
		void FillSphere();
		void FillPanda();
	};
}
