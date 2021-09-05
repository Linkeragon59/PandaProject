#include "Base_Time.h"

namespace Time
{
	TimeManager* TimeManager::ourInstance = nullptr;

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
		myFrameCounter++;
	}

}
