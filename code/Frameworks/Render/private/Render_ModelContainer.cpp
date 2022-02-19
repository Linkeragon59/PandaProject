#include "Render_ModelContainer.h"

#include "Render_glTFModel.h"
#include "Render_SimpleGeometryModel.h"

namespace Render
{
	ModelContainer::~ModelContainer()
	{
		for (uint i = 0; i < (uint)myModels.size(); ++i)
		{
			if (myModels[i])
				delete myModels[i];
		}
	}

	Handle ModelContainer::AddModel(const ModelData& someData)
	{
		uint index = (uint)myModels.size();
		for (uint i = 0; i < (uint)myModels.size(); ++i)
		{
			if (!myModels[i])
			{
				index = i;
				break;
			}
		}
		if (index >= (uint)myModels.size())
			myModels.push_back(nullptr);

		switch (someData.GetType())
		{
		case ModelData::Type::glTF:
			{
				const glTFModelData& glTFData = static_cast<const glTFModelData&>(someData);
				myModels[index] = new glTFModel(glTFData);
			}
			break;
		case ModelData::Type::SimpleGeometry:
			{
				const SimpleGeometryModelData& modelData = static_cast<const SimpleGeometryModelData&>(someData);
				myModels[index] = new SimpleGeometryModel(modelData);
			}
			break;
		default:
			Assert("Unsupported Model Type");
			break;
		}

		return Handle(index);
	}

	void ModelContainer::RemoveModel(Handle aModelHandle)
	{
		Assert(aModelHandle < (uint)myModels.size());
		if (myModels[aModelHandle])
		{
			delete myModels[aModelHandle];
			myModels[aModelHandle] = nullptr;
		}
	}

	void ModelContainer::UpdateModel(Handle aModelHandle, const ModelData& someData)
	{
		Assert(aModelHandle < (uint)myModels.size());
		if (myModels[aModelHandle])
		{
			myModels[aModelHandle]->Update(someData);
		}
	}

	Model* ModelContainer::GetModel(Handle aModelHandle) const
	{
		Assert(aModelHandle < (uint)myModels.size());
		return myModels[aModelHandle];
	}
}
