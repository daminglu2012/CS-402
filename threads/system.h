// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"

#define MAX_LOCKS 100

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
extern Lock PhysBitMapLock;
extern BitMap *PhysBitMap;

struct MachineLock {
	Lock* lock;
	AddrSpace* space;	// make sure the lock that Acquired/Released
						// is in the same process, process has its own AddrSpace

	bool isDestroyed;	// flag for the situation that if the lock 
						// has been Destroyed, then Acquire/Release will be ERROR

	bool isToBeDestroyed;	// flag for the situation that if one thread is to be released
							// the lock, context switch to DestroyLock
							// occurs, context switch back ERROR!!! The OS need to 'Remember'
							// the DestroyLock Request until there is no thread in the waitQ
};

extern MachineLock LockPool[MAX_LOCKS];
extern BitMap *LockPoolBitMap;	// Vacant Lock postion int the LockPool
extern Lock LockPoolLock;

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif


#endif // SYSTEM_H
