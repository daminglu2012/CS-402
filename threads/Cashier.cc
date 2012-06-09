/*
Cashier interacts with:
1) Customer (regular, complain, restock ?)
2) Manager
*/

#include "SupermarketSimulation.h"

int FindShortestCashierLine(int arr[], int arrLength); // defined at the bottom

float GoodsPrices[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};

Lock CustToCashierLineLock("CustToCashierLineLock");
Condition* EachCashierLineCV[NUM_CASHIER]; // init
int EachCashierLineLength[NUM_CASHIER]; // init
bool EachCashierIsBusy[NUM_CASHIER]; // init

// CustIDforEachCashier[ThisCashierID] == CustID this cashier is talking to
int CustIDforEachCashier[NUM_CASHIER]; // init

float CurCustTotal[NUM_CASHIER]; // init

Lock* EachCashierScanItemLock[NUM_CASHIER];
Condition* EachCashierScanItemCV[NUM_CASHIER];

void Cashier(int CashierIndex){
    while(true){
        //>> Interact with Customer
        CustToCashierLineLock.Acquire();
        // If there are Customers in the line, then
		// this Cashier must tell the 1st Customer to step up to the counter
		// He does this by Signaling the Condition Variable which puts the 1st Customer
		// on to the Ready Queue
        if(EachCashierLineLength[CashierIndex]>0){
            //debug
            printf("Cashier [%d] Waiting Line Length = [%d]. Will decrease one...\n",
                   CashierIndex, EachCashierLineLength[CashierIndex]);

            printf("Cashier [%d] calls Customer X to step up to counter\n",
                   CashierIndex);
            EachCashierLineCV[CashierIndex]->Signal(&CustToCashierLineLock);
            EachCashierLineLength[CashierIndex]--;
            EachCashierIsBusy[CashierIndex] = true;
        }else{
            printf("Cashier [%d] Waiting Line Length = 0 !!!\n",
            CashierIndex);
            EachCashierIsBusy[CashierIndex] = false;
            /*  Do I need this? Busy waiting?
            CustToCashierLineLock.Release();
            continue;
            */
        }

        // Acquire this Cashier's lock
		// We will use this lock to control the interactions between the Customer
		// and the Cashier
        EachCashierScanItemLock[CashierIndex]->Acquire();

        // After acquiring the above lock, we release the CustToCashierLineLock so who ever is waiting for the
		// lock can then search for the shortest line and then get into the appropriate line
        CustToCashierLineLock.Release();

        // The Cashier must now wait for the Customer to go up to his counter
		// and give him his items (Just Cust Num, the items are stored in the matrix)
		// Sleeping the Cashier frees up his EachCashierScanItemLock,
		// wakes up one Customer and puts him on the
		// Ready Queue
        EachCashierScanItemCV[CashierIndex]->Wait(EachCashierScanItemLock[CashierIndex]);

        // Now Cashier starts scanning items
        // CustIDforEachCashier[MyCashierNum] = CustID;

        float CustTotal = 0.0;
        int CurCustID = CustIDforEachCashier[CashierIndex];
        for(int s=0; s<NUM_ITEM; s++){
            CustTotal += GoodsPrices[s]*
            CustDataArr[CurCustID].ShoppingList[s];
        }

        CurCustTotal[CashierIndex] = CustTotal;
        printf("Cashier [%d] asks Customer [%d] to pay [%.2f]\n",
               CashierIndex, CustIDforEachCashier[CashierIndex], CustTotal);//Correct!
        // The Cashier signals the Customer, who is asleep waiting for
        // Cashier to scan his items and tell him the total amount
		EachCashierScanItemCV[CashierIndex]->Signal(EachCashierScanItemLock[CashierIndex]);
        EachCashierScanItemLock[CashierIndex]->Release();
        //<< Interact with Customer
    }
}

int FindShortestCashierLine(int ECLineLength[], int N){
    assert(N>=1);
    int ShortestLength = ECLineLength[0];
    int ShortestIndex = 0;
    for(int i=1; i<N; i++){
        if(ECLineLength[i]<ShortestLength){
            ShortestLength = ECLineLength[i];
            ShortestIndex = i;
        }
    }
    return ShortestIndex;
}

