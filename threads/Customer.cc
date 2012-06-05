/*
Customer interacts with:
1) Salesman
2) Cashier
3) Manager
*/

#include "SupermarketSimulation.h"
bool DebugMode = true;//If Debug is off, then all the code will be executed
int CustomerTestCode = 1;
//1: Cust-Sales
//2: Cust-Cashier
//3: ...

//>> Trolly is just one lock for now(Jun 4)
Lock TrollyLock("OneTrollyLock");
//<< Trolly is just one lock for now(Jun 4)

	int CustShoppingLists[8][10] = {
		{1, 5, 2, 1, 3, 5, 2, 0, 1, 4},
		{1, 5, 5, 1, 5, 5, 0, 3, 0, 4},
		{2, 3, 2, 4, 4, 0, 1, 0, 4, 5},
		{2, 3, 5, 2, 4, 2, 5, 5, 1, 2},
		{1, 1, 5, 3, 2, 2, 4, 4, 3, 1},
		{3, 0, 0, 0, 3, 2, 5, 4, 5, 5},
		{4, 3, 1, 4, 3, 2, 4, 1, 1, 2},
		{3, 2, 4, 4, 3, 2, 4, 0, 1, 5}
	};


void Customer(int CustID){
    //>> Customer get Trolly
    TrollyLock.Acquire();
    printf("Customer [%d] got a trolly\n",CustID);
    TrollyLock.Release();
    //<< Customer get Trolly

    // After securing the trolly, they can shop in any Department(there can be 1 to 5).

    if( !DebugMode|| (DebugMode && CustomerTestCode==1) )
    {//>> Interacts with Salesman
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
        printf("C: Customer [%d] is entering the store\n", CustID);
        SalesWaitingCV.Signal(&CustToSalesLineLock);
        CustToSalesLineLock.Release();
    } //<< Interacts with Salesman

    if( !DebugMode|| (DebugMode && CustomerTestCode==2) )
    {//>> Interacts with Cashier
        // Assume Cust k has got all the items on his list CustShoppingLists[k][10]


    }//<< Interacts with Cashier

}
