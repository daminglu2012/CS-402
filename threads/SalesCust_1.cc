//	DL: Test Sales-Cust Relation

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

//------------------------------------------------------------------------
// Daming's Test for Tennis

Lock tennisLock("tennisLock");
bool aHitBall = false;
Condition tennisCV("tennisCV");

void A() {
	while(true){
		tennisLock.Acquire();
		printf("A is hitting the ball\n");
		aHitBall = true;
		tennisCV.Signal(&tennisLock);
		tennisCV.Wait(&tennisLock);
        tennisLock.Release();
	}
}

void B(){
    while(true){
        tennisLock.Acquire();
        if(!aHitBall){
            tennisCV.Wait(&tennisLock);
        }
        printf("B is hitting the ball\n");
        aHitBall = false;
        tennisCV.Signal(&tennisLock);
        tennisLock.Release();
    }
}

void TestTennis() {
    Thread *t;
    t = new Thread("Player A");
    t->Fork((VoidFunctionPtr)A,0);

    t = new Thread("Player B");
    t->Fork((VoidFunctionPtr)B,0);
}

#endif
