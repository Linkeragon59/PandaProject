#pragma once

#include "GameCore_Prop.h"
#include "Render_ModelData.h"

namespace GameCore
{
	class SimpleGeometryProp : public Prop
	{
	public:
		void SetGeometry(Render::SimpleGeometryModelData::Preset aPreset);
		void SetGeometry(const std::vector<Render::SimpleGeometryModelData::Vertex>& someVertices, const std::vector<uint>& someIndices);
		void SetTexture(const std::string& aTextureFilename);

	private:
		Render::ModelData* CreateModelData(const Render::ModelData& someData) override;
	};
}