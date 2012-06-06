/*
Customer interacts with:
1) Salesman
2) Cashier
3) Manager
*/

#include "SupermarketSimulation.h"

void Customer(int CustID){
    //>> Interacts with Salesman
    // CustToSalesLineLock.Acquire();
    // int TotalReadySalesmen = 0; //for varifying, if TotalReadySalesmen>1, ERROR!
    // int ReadySalesmanID = -1;
    // for(int i=0; i<NUM_SALESMAN; i++){
    //     if(SalesmenStatus[i] == 3){
    //         TotalReadySalesmen++;
    //         ReadySalesmanID = i;
    //     }
    // }
    // if(TotalReadySalesmen>1){
    //     printf("\n\nMore than one Salesman ready!!!\n\n\n");
    //     exit(1);
    // }
    // if(TotalReadySalesmen == 0){
    //     CustWaitingLineCount++;
    //     printf("C: Customer [%d] is entering waiting line, now totally %d waiting in line\n",
    //            CustID,CustWaitingLineCount);
    //     CustWaitingCV.Wait(&CustToSalesLineLock);
    // }
    // // Here TotalReadySalesmen == 1, let's find the Ready One!
    // for(int i=0; i<NUM_SALESMAN; i++){
    //     if(SalesmenStatus[i] == 3){
    //         ReadySalesmanID = i;
    //     }
    // }
    // if(ReadySalesmanID == -1){
    //     printf("\n\nCannot locate the ready Salesman\n\n\n");
    //     exit(1);
    // }
    // SalesmenStatus[ReadySalesmanID] = 1;
    // printf("C: Salesman [%d] is taking care Customer [%d]\n", ReadySalesmanID, CustID);
    // printf("C: Customer [%d] is enterning the store\n", CustID);
    // SalesWaitingCV.Signal(&CustToSalesLineLock);
    // CustToSalesLineLock.Release();
    //<< Interacts with Salesman
}
