/*
Salesman interacts with:
1) Customer (regular, complain, restock ?)
2) Goods Loader
3) Manager
*/

#include "SupermarketSimulation.h"

//>> Interact with Customer
Lock CustToSalesLineLock("CustToSalesLineLock");
Condition CustWaitingCV("CustWaitingCV"), SalesWaitingCV("SalesWaitingCV");
int CustWaitingLineCount = 0;

//0:not busy,1:busy,2:on break,3:ready
int SalesmenStatus[NUM_SALESMAN];
//<< Interact with Customer

void Salesman(int SalesIndex){
    while(true){
        //>> Interact with Customer
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
        //<< Interact with Customer
    }
}
