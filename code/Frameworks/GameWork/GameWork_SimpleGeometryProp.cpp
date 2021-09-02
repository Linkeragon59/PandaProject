#include "GameWork_SimpleGeometryProp.h"

namespace GameWork
{
	void SimpleGeometryProp::SetGeometry(Render::SimpleGeometryModelData::Preset aPreset)
	{
		static_cast<Render::SimpleGeometryModelData*>(myModelData)->FillWithPreset(aPreset);
	}

	void SimpleGeometryProp::SetGeometry(const std::vector<Render::SimpleGeometryModelData::Vertex>& someVertices, const std::vector<uint>& someIndices)
	{
		static_cast<Render::SimpleGeometryModelData*>(myModelData)->myVertices = someVertices;
		static_cast<Render::SimpleGeometryModelData*>(myModelData)->myIndices = someIndices;
	}

	void SimpleGeometryProp::SetTexture(const std::string& aTextureFilename)
	{
		static_cast<Render::SimpleGeometryModelData*>(myModelData)->myTextureFilename = aTextureFilename;
	}

	Render::ModelData* SimpleGeometryProp::CreateModelData(const Render::ModelData& someData)
	{
		Assert(someData.GetType() == Render::ModelData::Type::SimpleGeometry);
		return new Render::SimpleGeometryModelData(static_cast<const Render::SimpleGeometryModelData&>(someData));
	}
}
