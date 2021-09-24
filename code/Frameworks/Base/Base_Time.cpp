#include "Base_Time.h"

namespace Time
{
	TimeManager* TimeManager::ourInstance = nullptr;

	TimeManager::TimeManager()
	{
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		myStartTime = currentTime;
		myCurrentTime = currentTime;
	}

	void TimeManager::Create()
	{
		ourInstance = new TimeManager;
	}

	void TimeManager::Destroy()
	{
		SafeDelete(ourInstance);
	}

	void TimeManager::NextFrame()
	{
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		myTimeNs = currentTime - myStartTime;
		myDeltaTimeNs = currentTime - myCurrentTime;
		myTime = currentTime - myStartTime;
		myDeltaTime = currentTime - myCurrentTime;

		myCurrentTime = currentTime;
		
		myFrameCounter++;
	}
}
