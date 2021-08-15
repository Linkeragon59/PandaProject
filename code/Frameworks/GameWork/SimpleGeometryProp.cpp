#include "SimpleGeometryProp.h"

namespace GameWork
{

	SimpleGeometryProp::SimpleGeometryProp()
	{
		myModelData = new Render::SimpleGeometryModelData;
	}

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
}
