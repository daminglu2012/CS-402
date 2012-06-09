/*
Customer interacts with:
1) Salesman
2) Cashier
3) Manager
*/

#include "SupermarketSimulation.h"
bool CustDebugMode = true;//If Debug is off, then all the code will be executed
int CustDebugIndex = 2;
//1: Cust-Sales
//2: Cust-Cashier
//3: ...

//>> Trolly is just one lock for now(Jun 4)
Lock TrollyLock("OneTrollyLock");
//<< Trolly is just one lock for now(Jun 4)


void Customer(int CustID){
    //>> Customer get Trolly
    TrollyLock.Acquire();
    printf("Customer [%d] got a trolly\n",CustID);
    TrollyLock.Release();
    //<< Customer get Trolly

    //-------------------------------------------------------------------------------

    // After securing the trolly, they can shop in any Department(there can be 1 to 5).

    //-------------------------------------------------------------------------------

    //>> Interacts with Salesman
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==1) )
    {
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
    }
    //<< Interacts with Salesman

    //-------------------------------------------------------------------------------

    // !!! Cust Shopping, YZ will take care

    //-------------------------------------------------------------------------------

    //>> Interacts with Cashier
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==2) )
    {
        // Assume Cust k has got all the items on his list CustShoppingLists[k][10]
        // Assume every Cust has enough money
        CustToCashierLineLock.Acquire();

        int MyCashierNum = FindShortestCashierLine(EachCashierLineLength, NUM_CASHIER);
        printf("Customer [%d] chose Cashier [%d] with a line of length [%d]\n",
               CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);
        if(EachCashierLineLength[MyCashierNum]>0 || EachCashierIsBusy[MyCashierNum]){
            EachCashierLineLength[MyCashierNum]++;
            printf("Cust [%d] just increased one, now Cashier [%d] Waiting Line Length = [%d]. Will decrease one...\n",
            CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);

            EachCashierLineCV[MyCashierNum]->Wait(&CustToCashierLineLock);

            // My Cashier finally calls me
            EachCashierIsBusy[MyCashierNum] = true; // dupli on purpose
        }

        // After acquiring my Cashier's lock, we release the Line Lock
        // so any Cashier that is waiting for the
		// Line Lock can call his next Customer
		// Or another Customer can line up for his Cashier
        CustToCashierLineLock.Release();

        /*
        printf("Customer [%d] waiting Cashier [%d] for EachCashierScanItemLock\n",
               CustID, MyCashierNum);
        */
        EachCashierScanItemLock[MyCashierNum]->Acquire();

        // 'give' my items to my Cashier
        CustIDforEachCashier[MyCashierNum] = CustID;
        EachCashierScanItemCV[MyCashierNum]->Signal(EachCashierScanItemLock[MyCashierNum]);

        // Wait Cashier to scan my items
        EachCashierScanItemCV[MyCashierNum]->Wait(EachCashierScanItemLock[MyCashierNum]);

        printf("The total amount for myself (Customer [%d]) is %.2f\n",
            CustID, CurCustTotal[MyCashierNum]);

        EachCashierScanItemLock[MyCashierNum]->Release();
    }
    //<< Interacts with Cashier

    //-------------------------------------------------------------------------------


}
