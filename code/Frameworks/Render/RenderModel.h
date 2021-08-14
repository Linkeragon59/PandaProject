#pragma once

namespace Render
{
	struct BaseModelData
	{
		virtual ~BaseModelData() {}
		enum class Type
		{
			glTF,
			Dynamic,
		};
		virtual Type GetType() const = 0;

		glm::mat4 myMatrix = glm::mat4(1.0f);
		bool myIsTransparent = false;
	};

	struct glTFModelData : public BaseModelData
	{
		Type GetType() const override { return Type::glTF; }

		std::string myFilename;
		bool myIsAnimated = false;
	};

	struct DynamicModelData : public BaseModelData
	{
		Type GetType() const override { return Type::Dynamic; }

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

#if DEBUG_BUILD
		static void GetVectorBaseWidget(std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices);
#endif
	};

	class Model
	{
	public:
		virtual ~Model() {};
		virtual void Update(const BaseModelData& someData) = 0;
	};
}