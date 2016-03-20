#include <deque>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>

#include <boost/optional.hpp>

struct TaskContext
{
   std::string msg;
};

enum class TaskReturnCode { SUCCESS, FAIL, SUSPENDED };

struct TaskIf
{
   virtual TaskReturnCode run(TaskContext&) = 0;
};

struct NormalTask : public TaskIf
{
   TaskReturnCode run(TaskContext& ctx)
   {
      std::cout << "work: " << ctx.msg << ", type: NormalTask" << std::endl;
      return TaskReturnCode::SUCCESS;
   }
};

class SuspendedTask : public TaskIf
{
public:
   SuspendedTask()
   :returnCode(TaskReturnCode::SUSPENDED)
   {}

   TaskReturnCode run(TaskContext& ctx)
   {
      if (not deferredThread)
      {
         deferredThread = std::thread([this, &ctx]{
            sleep(10);
            std::cout << "work: " << ctx.msg << ", type: SuspendedTask" << std::endl;
            returnCode = TaskReturnCode::SUCCESS;
         });
      }
      return returnCode;
   }
   
   ~SuspendedTask()
   {
      if (deferredThread)
      {
         deferredThread.get().join();
      }
   }
private:
   boost::optional<std::thread> deferredThread;
   std::atomic<TaskReturnCode> returnCode;
};


using Tasks = std::vector<std::shared_ptr<TaskIf>>;

struct TasksRunnerContext
{
   TaskContext taskContext;
   Tasks tasks;
   Tasks::size_type resumePoint;
};

struct TasksRunner
{
   std::deque<TasksRunnerContext> pendingWork;

   void scheduleWork(TaskContext& taskContext)
   {
      TasksRunnerContext tasksRunnerContext;
      tasksRunnerContext.taskContext = taskContext;
      tasksRunnerContext.tasks.emplace_back(new NormalTask());
      tasksRunnerContext.tasks.emplace_back(new SuspendedTask());
      tasksRunnerContext.tasks.emplace_back(new NormalTask());
      tasksRunnerContext.resumePoint = 0;

      pendingWork.emplace_back(tasksRunnerContext); 
   }

   bool doWork()
   {
      auto work = pendingWork.front();
      pendingWork.pop_front();
      auto resultCode = runTasks(work);
      if (resultCode == TaskReturnCode::SUSPENDED)
      {
         pendingWork.push_back(work);
      }
      return not pendingWork.empty();
   }

   TaskReturnCode runTasks(TasksRunnerContext& work)
   {
      for(;work.resumePoint < work.tasks.size(); ++work.resumePoint)
      {
         auto& task = work.tasks[work.resumePoint];
         auto returnCode= task->run(work.taskContext);
         if (returnCode == TaskReturnCode::SUSPENDED) 
            return TaskReturnCode::SUSPENDED;
      }
      return TaskReturnCode::SUCCESS;
   }
};



int main(void)
{
   TasksRunner tasksRunner;
   
   TaskContext ctx1{"foo"};
   TaskContext ctx2{"bar"};
   TaskContext ctx3{"dar"};

   tasksRunner.scheduleWork(ctx1);
   tasksRunner.scheduleWork(ctx2);
   tasksRunner.scheduleWork(ctx3);
   
   bool workPending = true;

   while (workPending)
   {
      workPending = tasksRunner.doWork();
   }
}
