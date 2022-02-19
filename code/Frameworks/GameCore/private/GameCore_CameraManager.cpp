#include "GameCore_CameraManager.h"

#include "GameCore_Camera.h"
#include "GameCore.h"

namespace GameCore
{
	CameraManager::~CameraManager()
	{
		for (uint i = 0; i < myCameras.size(); ++i)
		{
			delete myCameras[i];
		}
	}

	void CameraManager::Update()
	{
		Render::Renderer* renderer = GameCore::GetInstance()->GetMainWindowRenderer();
		float aspectRatio = GameCore::GetInstance()->GetMainWindowAspectRatio();

		for (uint i = 0; i < myCameras.size(); ++i)
		{
			myCameras[i]->SetAspectRatio(aspectRatio);
			myCameras[i]->Update();
			if (myCameras[i] == myActiveCamera)
			{
				myCameras[i]->Bind(renderer);
			}
		}
	}

	Camera* CameraManager::AddCamera(bool aSetActive /*= true*/)
	{
		Camera* camera = new Camera();
		myCameras.push_back(camera);
		if (aSetActive)
		{
			SetActiveCamera(camera);
		}
		return camera;
	}

	void CameraManager::RemoveCamera(Camera* aCamera)
	{
		for (auto it = myCameras.begin(); it != myCameras.end(); ++it)
		{
			if ((*it) == aCamera)
			{
				delete (*it);
				myCameras.erase(it);
				return;
			}		
		}
	}
}
