#pragma once

#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace ThreadHelpers
{
	class WorkerPool;

	enum class WorkerPriority
	{
		High,
		Low
	};

	struct JobData
	{
	private:
		friend class WorkerPool;
		void OnDone();
		void Wait();

		std::function<void()> myFunction;
		std::mutex myDoneMutex;
		std::condition_variable myDoneCondition;
		bool myDone = false;
	};
	typedef std::shared_ptr<JobData> JobHandle;

	class WorkerPool
	{
	public:
		WorkerPool(WorkerPriority aPriority = WorkerPriority::High);

#if DEBUG_BUILD
		void SetWorkersName(const std::string& aBaseName) { myWorkersBaseName = aBaseName; }
#endif
		void SetWorkersCount(uint aCount = UINT_MAX);
		uint GetWorkersCount() const { return (uint)myWorkers.size(); }

		JobHandle RequestJob(std::function<void()> aJob, uint aWorkIndex = UINT_MAX);
		void WaitForJob(JobHandle aJobHandle);
		void WaitIdle();

	private:
		struct Worker
		{
			Worker(WorkerPool* aPool);
			~Worker();

			void AssignJob(JobHandle aJob);
			void NotifyWaitingJobs();
			void WaitJobs();
			void RunJobs();
			bool EvaluateWorkToDo();

			WorkerPool* myPool;

			std::thread myWorkerThread;

			std::mutex myJobQueueMutex;
			std::condition_variable myWorkToDoCondition;
			std::condition_variable myWaitForJobsCondition;
			std::queue<JobHandle> myJobQueue;

			bool myStopping = false;
		};

		bool AssignJobTo(Worker* aWorker);

#if DEBUG_BUILD
		std::string myWorkersBaseName;
#endif
		WorkerPriority myWorkersPriority;
		std::vector<std::unique_ptr<Worker>> myWorkers;

		mutable std::mutex myWaitingJobQueueMutex;
		std::queue<JobHandle> myWaitingJobQueue;
	};
}

