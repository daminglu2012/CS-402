//	DL: Test Sales-Cust Relation

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include <cstring>
#endif

#ifdef CHANGED

//------------------------------------------------------------------------
// Daming's Cust-Sales Try #1
// 30 customers, 1 salesman

Lock custSalesLock("custSalesLock");
Condition custWaitingCV("custWaitingCV"), salesWaitingCV("salesWaitingCV");
int custWaitingLineCount = 0;
int salesStatus = 1;//0:not busy, 1:busy, 2:on break, 3:ready

void dl_Customer(int d){
    custSalesLock.Acquire();
    if(salesStatus == 1){
        custWaitingLineCount++;
        printf("One more cust waiting, now totally %d waiting in line",
               custWaitingLineCount);
        custWaitingCV.Wait(&custSalesLock);
    }
    salesStatus = 1;
    printf("Customer [%d] is enterning the store\n", d);
    salesWaitingCV.Signal(&custSalesLock);
    custSalesLock.Release();
}

void dl_Salesman(int d){
    custSalesLock.Acquire();
    if(custWaitingLineCount>0){
        custWaitingCV.Signal(&custSalesLock);
        custWaitingLineCount--;
        salesStatus = 1;
        printf("Salesman %d is taking care a cust, ", d);
        printf("now the # of cust waiting is %d", custWaitingLineCount);
    }else{
        salesStatus = 0;
    }
    salesWaitingCV.Wait(&custSalesLock);
    custSalesLock.Release();
}

void dl_SalesCust() {
    Thread *t;
    t = new Thread("Salesman");
    t->Fork((VoidFunctionPtr)dl_Salesman,0);

    for(int i=30; i>=1; i--){
        t = new Thread(strcat("Cust ",&((char)i);
        t->Fork((VoidFunctionPtr)dl_Customer,i);
    }
}

#endif
