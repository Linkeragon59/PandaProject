#pragma once

struct GLFWwindow;

namespace GameWork
{
	class Camera;
	class Gui;

	class Window
	{
	public:
		Window(uint aWidth, uint aHeight, const char* aTitle);
		~Window();

		void Init();
		bool Update();
		void Finalize();

		GLFWwindow* GetWindowHandle() const { return myWindow; }
		bool ShouldClose() const;
		void GetSize(int& anOutWidth, int& anOutHeight) const;

	private:
		GLFWwindow* myWindow = nullptr;

		Camera* myCamera = nullptr;
		std::vector<Gui*> myUIs;

		uint myRenderIndex = UINT_MAX;
		uint myInputIndex = UINT_MAX;

		// Temp, for testing
		void LoadTestAssets();
		void UnloadTestAssets();
		uint myCastleModel = UINT_MAX;
		uint myCastleWindows = UINT_MAX;
		uint myAvocadoModel = UINT_MAX;
		uint myAnimatedModel = UINT_MAX;
		uint myDummyModel = UINT_MAX;
		bool myTempAssetsLoaded = false;
	};
}
