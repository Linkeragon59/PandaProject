#pragma once

#include "Prop.h"
#include "RenderModel.h"

namespace GameWork
{
	class SimpleGeometryProp : public Prop
	{
	public:
		SimpleGeometryProp();
		~SimpleGeometryProp() {}

		void SetGeometry(Render::SimpleGeometryModelData::Preset aPreset);
		void SetGeometry(const std::vector<Render::SimpleGeometryModelData::Vertex>& someVertices, const std::vector<uint>& someIndices);
		void SetTexture(const std::string& aTextureFilename);
	};
}
