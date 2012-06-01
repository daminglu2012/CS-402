// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

//-> DL
Lock::Lock(char* debugName) {
	name = debugName;
	lockwaitQueue = new List;
	isFree = true;
	owner = NULL; // where to assign ? DONE
}

Lock::~Lock() {
	delete lockwaitQueue;
}

bool Lock::isHeldByCurrentThread(){
	return (owner == currentThread);
}

void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(isHeldByCurrentThread()){
        (void) interrupt->SetLevel(oldLevel);
        return;
    }
    if(isFree){
    	isFree = false;
    	owner = currentThread;
    }else{
    	lockwaitQueue->Append((void *)currentThread);
    	currentThread->Sleep();
    }
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(!isHeldByCurrentThread()){
    	printf("Error in Lock::Release: [%s] is not the owner of lock: %s\n", currentThread->getName(),name);
    	(void) interrupt->SetLevel(oldLevel);
    	return;
    }
    if(!lockwaitQueue->IsEmpty()){
    	Thread* cur = (Thread *)lockwaitQueue->Remove();
    	owner = cur;
    	scheduler->ReadyToRun(cur);
    	// Mesa semantics
    }else{
    	isFree = true;
    	owner = NULL;
    }
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

Condition::Condition(char* debugName) {
	name = debugName;
	waitinglockQueue = new List;
	waitingLock = NULL;
}

Condition::~Condition() {
	delete waitinglockQueue;
	waitingLock = NULL;
}

void Condition::Wait(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(conditionLock == NULL){
    	printf("Error in Condition::Wait: [%s] is NULL in Condition::Wait\n",conditionLock->getName());
        (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
        return;
    }
    if(waitingLock == NULL){
    	waitingLock = conditionLock;
    }
    if(conditionLock != waitingLock){
    	printf("Error in Condition::Wait: conditionLock(%s) != waitingLock(%s) \n",conditionLock->getName(),waitingLock->getName());
        (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
        return;
    }
    conditionLock->Release();
    waitinglockQueue->Append((void *)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Condition::Signal(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(waitinglockQueue->IsEmpty()){
        (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
        return;
    }
    if(waitingLock != conditionLock){
    	printf("Error in Condition::Signal: conditionLock(%s) != waitingLock(%s) \n",conditionLock->getName(),waitingLock->getName());
        (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
        return;
    }

    Thread* woken = (Thread *)waitinglockQueue->Remove();
  	scheduler->ReadyToRun(woken);

  	if(waitinglockQueue == NULL){
		waitingLock = NULL;
	}
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Condition::Broadcast(Lock* conditionLock) {
	while(!waitinglockQueue->IsEmpty()){
		Signal(conditionLock);
	}
}
//<- DL
