//	DL: Test Sales-Cust Relation

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include <cstring>
#endif

#ifdef CHANGED

//------------------------------------------------------------------------
// Daming's Cust-Sales Try #2
// 30 customers, 3 salesman

#define NUM_SALESMAN 3
#define NUM_CUSTOMER 10

Lock CustToSalesLineLock("CustToSalesLineLock");
Condition CustWaitingCV("CustWaitingCV"), SalesWaitingCV("SalesWaitingCV");
int CustWaitingLineCount = 0;
int SalesmenStatus[NUM_SALESMAN];//0:not busy,1:busy,2:on break,3:ready

void dl_Customer(int CustID){
    CustToSalesLineLock.Acquire();
    int TotalReadySalesmen = 0; //for varifying, if TotalReadySalesmen>1, ERROR!
    int ReadySalesmanID = -1;
    for(int i=0; i<NUM_SALESMAN; i++){
        if(SalesmenStatus[i] == 3){
            TotalReadySalesmen++;
            ReadySalesmanID = i;
        }
    }
    if(TotalReadySalesmen>1){
        printf("\n\nMore than one Salesman ready!!!\n\n\n");
        exit(1);
    }
    if(TotalReadySalesmen == 0){
        CustWaitingLineCount++;
        printf("C: Customer [%d] is entering waiting line, now totally %d waiting in line\n",
               CustID,CustWaitingLineCount);
        CustWaitingCV.Wait(&CustToSalesLineLock);
    }
    // Here TotalReadySalesmen == 1, let's find the Ready One!
    for(int i=0; i<NUM_SALESMAN; i++){
        if(SalesmenStatus[i] == 3){
            ReadySalesmanID = i;
        }
    }
    if(ReadySalesmanID == -1){
        printf("\n\nCannot locate the ready Salesman\n\n\n");
        exit(1);
    }
    SalesmenStatus[ReadySalesmanID] = 1;
    printf("C: Salesman [%d] is taking care Customer [%d]\n", ReadySalesmanID, CustID);
    printf("C: Customer [%d] is enterning the store\n", CustID);
    SalesWaitingCV.Signal(&CustToSalesLineLock);
    CustToSalesLineLock.Release();
}

void dl_Salesman(int SalesIndex){
    while(true){
        CustToSalesLineLock.Acquire();
        if(CustWaitingLineCount>0){
            CustWaitingCV.Signal(&CustToSalesLineLock);
            CustWaitingLineCount--;
            printf("S: Salesman [%d] is Ready ",SalesIndex);
            SalesmenStatus[SalesIndex] = 3; // This Salesman is Ready
            // printf("Salesman %d is taking care a cust, ", SalesIndex);
            // this should be in Customer
            printf(", now the # of cust waiting is %d\n", CustWaitingLineCount);
        }else{
            SalesmenStatus[SalesIndex] = 0;
        }
        SalesWaitingCV.Wait(&CustToSalesLineLock);
        SalesmenStatus[SalesIndex] = 3;
        CustToSalesLineLock.Release();
    }
}

void dl_SalesCust() {
    Thread *t;
    char *name;
    // init customers
    for(int i=0; i<NUM_CUSTOMER; i++){
        name = new char[20];
        sprintf(name, "Customer_%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)dl_Customer,i);
    }
    // init salesmen
    for(int i=0; i<NUM_SALESMAN; i++){
        SalesmenStatus[i] = 1; // init busy
    }
    for(int i=0; i<=NUM_SALESMAN; i++){
        char i_char = (char)i;
        t = new Thread(strcat("Salesman_",&i_char));
        t->Fork((VoidFunctionPtr)dl_Salesman,i);
    }
}

#endif
