// Customer-Salesman Test
// By Lu Sir, Linda, Yao, Stanley
// @ Jun 2nd, 2012, Unversity Of Southern California
#include <iostream>
#include "system.h"
#include "copyright.h"

using namespace std;

#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

typedef enum {
	FREE = 0,
	BUSY,
	ONBREAK,
	READY
} SalesStatus;

// Shared Data
Lock trollyLock("trollyLock");

// Customer Thread
void Customer(int ind) {
    // Customer gets a trolly for shopping
    cout << "Customer [" << ind
         << "] enters the SuperMarket" << endl;
    cout << "Customer [" << ind
         << "] gets in line for a trolly" << endl;
    trollyLock.Acquire();

    cout << "Customer [" << ind
         << "] has a trolly for shopping" << endl;

    trollyLock.Release();
}

void Salesman(int ind) {

}

void TestCustSales() {
    for (int i = 0; i < 30; i++) {
        Thread *t = new Thread("Customer");
        t->Fork((VoidFunctionPtr)Customer, i);
    }
}

#endif