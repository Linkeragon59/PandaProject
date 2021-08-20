#pragma once

namespace GameWork
{
	class Camera;

	class CameraManager
	{
	public:
		~CameraManager();

		void Update();

		Camera* AddCamera(bool aSetActive = true);
		void RemoveCamera(Camera* aCamera);

		void SetActiveCamera(Camera* aCamera) { myActiveCamera = aCamera; }
		Camera* GetActiveCamera() const { return myActiveCamera; }

	private:
		std::vector<Camera*> myCameras;
		Camera* myActiveCamera = nullptr;
	};
}
