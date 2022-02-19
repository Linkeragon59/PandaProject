#include "GameWork_PropManager.h"

#include "GameWork_glTFProp.h"
#include "GameWork_SimpleGeometryProp.h"
#include "GameWork.h"

namespace GameWork
{
	PropManager::PropManager(Render::Renderer::DrawType aDrawType /*= Render::Renderer::DrawType::Default*/)
		: myDrawType(aDrawType)
	{
	}

	PropManager::~PropManager()
	{
		for (uint i = 0; i < myProps.size(); ++i)
		{
			delete myProps[i];
		}
	}

	void PropManager::Update()
	{
		Render::Renderer* renderer = GameWork::GetInstance()->GetMainWindowRenderer();

		for (uint i = 0; i < myProps.size(); ++i)
		{
			myProps[i]->Update();
			myProps[i]->Draw(renderer, myDrawType);
		}
	}

	Prop* PropManager::Spawn(const Render::ModelData& someData,
		const glm::vec3& aPosition /*= glm::vec3(0.0f)*/,
		const glm::vec3& anOrientation /*= glm::vec3(0.0f)*/,
		const glm::vec3& aScale /*= glm::vec3(1.0f) */)
	{
		Prop* prop = nullptr;

		switch (someData.GetType())
		{
		case Render::ModelData::Type::glTF:
			prop = new glTFProp();
			break;
		case Render::ModelData::Type::SimpleGeometry:
			prop = new SimpleGeometryProp();
			break;
		default:
			Assert(false, "Unsupported Model Type");
			break;
		}

		if (prop)
		{
			prop->Init(someData);

			prop->SetPosition(aPosition);
			prop->SetOrientation(anOrientation);
			prop->SetScale(aScale);

			prop->Spawn();

			myProps.push_back(prop);
		}

		return prop;
	}

	void PropManager::Despawn(Prop* aProp)
	{
		for (auto it = myProps.begin(); it != myProps.end(); ++it)
		{
			if ((*it) == aProp)
			{
				(*it)->Despawn();
				delete (*it);
				myProps.erase(it);
				return;
			}
		}
	}
}
