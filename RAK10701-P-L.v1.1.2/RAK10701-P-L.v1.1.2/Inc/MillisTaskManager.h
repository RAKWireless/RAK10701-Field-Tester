/**
  **************************************************** ****************************
  * @file MillisTaskManager.h
  * @version v1.0
  * @date April 28, 2022
  * @brief An ultra-lightweight time-sharing cooperative task scheduler that can replace the old millis() polling scheme without relying on Arduino API
  * @Upgrade 2018.7.26 v1.0 Change the task status flag type to bool type
			Move the typedef into the class
			Fixed a bug that caused the task to stop due to numerical overflow after 50 days
			Change TaskCtrl to TaskStateCtrl, add an interface for modifying the task interval, and add TaskFind to traverse the list to find tasks
			Add destructor for freeing memory
			Change FuncPos to ID and add TaskFind(void_TaskFunction_t Function)
			Support setting priority, the priority is arranged as task ID number, the smaller the number, the higher the priority
			Add GetCPU_Useage() to get CPU usage
			Add anti-collision judgment to TaskRegister
			Add TimeCost task time cost calculation
			Use singly linked list to manage tasks, add GetTickElaps to handle uint32 overflow, add time error records
  **************************************************** ****************************
  * @attention
  * You need to provide a system clock accurate to the millisecond level, and then call the Running function periodically
  **************************************************** ****************************
  */

#ifndef __MILLISTASKMANAGER_H
#define __MILLISTASKMANAGER_H

#define MTM_USE_CPU_USAGE 1

#include "stdint.h"

class MillisTaskManager
{
public:
	typedef void (*TaskFunction_t)(void); // Task callback function.
	struct Task
	{
		bool State;				 // Task state.
		bool FirstExecut;		 // Whether the first execution flag.
		TaskFunction_t Function; // Task function pointer.
		uint32_t Time;			 // Task execution cycle time.
		uint32_t TimePrev;		 // The last trigger time of the task.
		uint32_t TimeCost;		 // Task cost (us) time.
		uint32_t TimeError;		 // Error time.
		struct Task *Next;		 // next node.
	};
	typedef struct Task Task_t;

	MillisTaskManager(bool priorityEnable = false);
	~MillisTaskManager();

	Task_t *Register(TaskFunction_t func, uint32_t timeMs, bool state = true);
	Task_t *Find(TaskFunction_t func);
	Task_t *GetPrev(Task_t *task);
	bool Logout(TaskFunction_t func);
	bool SetState(TaskFunction_t func, bool state);
	bool SetIntervalTime(TaskFunction_t func, uint32_t timeMs);
	bool ReSetTaskTime(TaskFunction_t func, uint32_t timeMs);
	uint32_t GetTimeCost(TaskFunction_t func);
	uint32_t GetTickElaps(uint32_t nowTick, uint32_t prevTick);
#if (MTM_USE_CPU_USAGE == 1)
	float GetCPU_Usage();
#endif
	void Running(uint32_t tick);

private:
	Task_t *Head;		 // Task list header.
	Task_t *Tail;		 // Tail of the task list.
	bool PriorityEnable; // Priority enable.
};

#endif
