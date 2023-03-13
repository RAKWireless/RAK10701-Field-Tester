#include "../Inc/MillisTaskManager.h"

#ifndef NULL
#   define NULL 0
#endif

#define TASK_NEW(task) do{task = new Task_t;}while(0)
#define TASK_DEL(task) do{delete task;}while(0)

/**
   * @brief initialization task list
   * @param priorityEnable: Set whether to enable priority
   * @retval None
   */
MillisTaskManager::MillisTaskManager(bool priorityEnable)
{
    PriorityEnable = priorityEnable;
    Head = NULL;
    Tail = NULL;
}

/**
   * @brief scheduler destructor, release task list memory
   * @param none
   * @retval None
   */
MillisTaskManager::~MillisTaskManager()
{
	Task_t* now = Head; 	// Move to the head of the list.
	while(true)
	{
		if(now == NULL)
				break;
		Task_t* now_del = now;
		now = now->Next;
		TASK_DEL(now_del);
	}
}

/**
   * @brief Add a task to the task list and set the interval execution time
   * @param func: task function pointer
   * @param timeMs: cycle time setting (milliseconds)
   * @param state: task switch
   * @retval task node address
   */
MillisTaskManager::Task_t* MillisTaskManager::Register(TaskFunction_t func, uint32_t timeMs, bool state)
{
    Task_t* task = Find(func);
    
    if(task != NULL)
    {
        task->Time = timeMs;
        task->State = state;
				task->FirstExecut = true;
        return task;
    }

    TASK_NEW(task);

    if(task == NULL)
    {
        return NULL;
    }

    task->Function = func;     
    task->Time = timeMs;  
    task->State = state; 
		task->FirstExecut = true;
    task->TimePrev = 0;    
    task->TimeCost = 0;   
    task->TimeError = 0;  
    task->Next = NULL;    
    
    if(Head == NULL)
    {
        Head = task;
    }
    else
    {
        Tail->Next = task;
    }
    
    Tail = task;
    return task;
}

/**
   * @brief find the task, return the task node
   * @param func: task function pointer
   * @retval task node address
   */
MillisTaskManager::Task_t* MillisTaskManager::Find(TaskFunction_t func)
{
    Task_t* now = Head;
    Task_t* task = NULL;
    while(true)
    {
        if(now == NULL)
            break;

        if(now->Function == func)
        {
            task = now;
            break;
        }

        now = now->Next;
    }
    return task;
}

/**
   * @brief Get the previous node of the current node
   * @param task: current task node address
   * @retval previous task node address
   */
MillisTaskManager::Task_t* MillisTaskManager::GetPrev(Task_t* task)
{
    Task_t* now = Head;    
    Task_t* prev = NULL;   
    Task_t* retval = NULL; 
    
    while(true)
    {
        if(now == NULL)
        {
            break;
        }
        
        if(now == task)
        {
            retval = prev;
            break;
        }
        
        prev = now;
        
        now = now->Next;
    }
    return retval;
}

/**
   * @brief logout task (use with caution, thread-unsafe)
   * @param func: task function pointer
   * @retval true: success; false: failure
   */
bool MillisTaskManager::Logout(TaskFunction_t func)
{
    Task_t* task = Find(func);
    if(task == NULL)
        return false;

    Task_t* prev = GetPrev(task);
    Task_t* next = task->Next;
    
    if(prev == NULL && next != NULL)
    {
        Head = next;
    }
    else if(prev != NULL && next == NULL)
    {
        prev->Next = NULL;
    }
    else if(prev != NULL && next != NULL)
    {
        prev->Next = next;
    }
    TASK_DEL(task);
    return true;
}

/**
   * @brief task state control
   * @param func: task function pointer
   * @param state: task state
   * @retval true: success; false: failure
   */
bool MillisTaskManager::SetState(TaskFunction_t func, bool state)
{
    Task_t* task = Find(func);
    if(task == NULL)
        return false;

    task->State = state;
    return true;
}

/**
   * @brief task execution cycle setting
   * @param func: task function pointer
   * @param timeMs: task execution cycle
   * @retval true: success; false: failure
   */
bool MillisTaskManager::SetIntervalTime(TaskFunction_t func, uint32_t timeMs)
{
    Task_t* task = Find(func);
    if(task == NULL)
        return false;

    task->Time = timeMs;
    return true;
}

/**
   * @brief reset task execution time
   * @param func: task function pointer
   * @param timeMs: reset time
   * @retval true: success; false: failure
   */
bool MillisTaskManager::ReSetTaskTime(TaskFunction_t func, uint32_t timeMs)
{
    Task_t* task = Find(func);
    if(task == NULL)
        return false;

    task->TimePrev = timeMs;
    return true;
}

#if (MTM_USE_CPU_USAGE == 1)
#include "Arduino.h"                
static uint32_t UserFuncLoopUs = 0; 
/**
   * @brief Get CPU usage
   * @param none
   * @retval CPU usage, 0~100%
   */
float MillisTaskManager::GetCPU_Usage()
{
    static uint32_t MtmStartUs;
    float usage = (float)UserFuncLoopUs / (micros() - MtmStartUs) * 100.0f;

    if(usage > 100.0f)
        usage = 100.0f;

    MtmStartUs = micros();
    UserFuncLoopUs = 0;
    return usage;
}
#endif

/**
   * @brief time difference judgment
   * @param nowTick: current time
   * @param prevTick: previous time
   * @retval time difference
   */
uint32_t MillisTaskManager::GetTickElaps(uint32_t nowTick, uint32_t prevTick)
{
    uint32_t actTime = nowTick;

    if(actTime >= prevTick)
    {
        prevTick = actTime - prevTick;
    }
    else
    {
        prevTick = /*UINT32_MAX*/0xFFFFFFFF - prevTick + 1;
        prevTick += actTime;
    }

    return prevTick;
}

/**
   * @brief Get the time spent on a single task (us)
   * @param func: task function pointer
   * @retval task single time consumption (us)
   */
uint32_t MillisTaskManager::GetTimeCost(TaskFunction_t func)
{
    Task_t* task = Find(func);
    if(task == NULL)
        return 0;

    return task->TimeCost;
}

/**
   * @brief scheduler (kernel)
   * @param tick: provide a system clock variable accurate to milliseconds
   * @retval None
   */
void MillisTaskManager::Running(uint32_t tick)
{
    Task_t* now = Head;
    while(true)
    {
        if(now == NULL)
        {
            break;
        }

        if(now->Function != NULL && now->State)
        {
            uint32_t elapsTime = GetTickElaps(tick, now->TimePrev);
            if((elapsTime >= now->Time) || (now->FirstExecut == true))
            {
								now->FirstExecut = false;

                now->TimeError = elapsTime - now->Time;
                
                now->TimePrev = tick;

#if (MTM_USE_CPU_USAGE == 1)
                uint32_t start = micros();
                
                now->Function();
                
                uint32_t timeCost = micros() - start;
                
                now->TimeCost = timeCost;
                
                UserFuncLoopUs += timeCost;
#else
                now->Function();
#endif
                if(PriorityEnable)
                {
                    break;
                }
            }
        }

        now = now->Next;
    }
}
