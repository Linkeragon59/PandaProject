#pragma once

#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace ThreadHelpers
{
	enum class WorkerPriority
	{
		High,
		Low
	};

	class WorkerPool;

	// Thread that run jobs one after the other
	class Worker
	{
		friend class WorkerPool;
	public:
		Worker(WorkerPool* aPool);
		~Worker();

		void AssignJob(std::function<void()> aJob);
		void NotifyWaitingJobs();
		void WaitJobs();

	private:
		void RunJobs();
		bool EvaluateWorkToDo();

		WorkerPool* myPool;

		std::thread myWorkerThread;

		std::mutex myJobQueueMutex;
		std::condition_variable myJobQueueCondition;
		std::queue<std::function<void()>> myJobQueue;

		bool myStopping = false;
	};

	class WorkerPool
	{
		friend class Worker;
	public:
		WorkerPool(WorkerPriority aPriority = WorkerPriority::High);

#if DEBUG_BUILD
		void SetWorkersName(const std::string& aBaseName) { myWorkersBaseName = aBaseName; }
#endif
		void SetWorkersCount(uint aCount = UINT_MAX);
		uint GetWorkersCount() const { return (uint)myWorkers.size(); }

		void RequestJob(std::function<void()> aJob, uint aWorkIndex = UINT_MAX);
		void Wait();

	private:
		bool AssignJob(Worker* aWorker);

#if DEBUG_BUILD
		std::string myWorkersBaseName;
#endif
		WorkerPriority myWorkersPriority;
		std::vector<std::unique_ptr<Worker>> myWorkers;

		mutable std::mutex myWaitingJobQueueMutex;
		std::queue<std::function<void()>> myWaitingJobQueue;
	};
}

