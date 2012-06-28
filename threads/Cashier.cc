/*
Cashier interacts with:
1) Customer (regular, complain, restock ?)
2) Manager
*/

#include "SupermarketSimulation.h"

int FindShortestCashierLine(int arr[], int arrLength); // defined at the bottom

float GoodsPrices[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};

Lock CustToCashierLineLock("CustToCashierLineLock");
Lock PrvlCustLineLock("PrvlCustLineLock");//privilege
Lock WholeLine("WholeLine");

Condition* EachCashierLineCV[NUM_CASHIER];
Condition* EachCashierPrvlLineCV[NUM_CASHIER];//privilege


int EachCashierLineLength[NUM_CASHIER];
int EachCashierRegLen[NUM_CASHIER];
int EachCashierPrvLen[NUM_CASHIER];

bool EachCashierIsBusy[NUM_CASHIER];

// CustIDforEachCashier[ThisCashierID] == CustID this cashier is talking to
int CustIDforEachCashier[NUM_CASHIER];

float CurCustTotal[NUM_CASHIER];

Lock* EachCashierScanItemLock[NUM_CASHIER];
Condition* EachCashierScanItemCV[NUM_CASHIER];

bool CashierIsOnBreak[NUM_CASHIER];
Lock CashierOnBreakLock("CashierOnBreakLock");
Condition CashierOnBreakCV("CashierOnBreakCV");

Lock ReadyLock("ReadyLock");
Condition ReadyCV("ReadyCV");

int Ready_Cashier=0;
bool isReady = false;
void Cashier(int CashierIndex){

    while(true){
		printf("  ImCashier_[%d]: starts a work cycle\n", CashierIndex);
		printf("\tImCashier_[%d], PrvLen==[%d], RegLen==[%d]\n",
				CashierIndex,EachCashierPrvLen[CashierIndex],EachCashierRegLen[CashierIndex]);

		WholeLine.Acquire();
		PrvlCustLineLock.Acquire();
    	bool hasPrivilge = false;
        if(EachCashierPrvLen[CashierIndex]>0){
        	printf("\tEachCashierPrvlLineCV [%d] Signal PrvlCustLineLock\n",CashierIndex);
        	EachCashierPrvlLineCV[CashierIndex]->Signal(&PrvlCustLineLock);
            EachCashierLineLength[CashierIndex]--;
        	EachCashierPrvLen[CashierIndex]--;
            EachCashierIsBusy[CashierIndex] = true;
            hasPrivilge = true;
        }else{
            EachCashierIsBusy[CashierIndex] = false;
        }

        if(hasPrivilge == false){
			CustToCashierLineLock.Acquire();

        	PrvlCustLineLock.Release();//release PRIVILEGE lock
			//>> Interact with Customer
			// If there are Customers in the line, then
			// this Cashier must tell the 1st Customer to step up to the counter
			// He does this by Signaling the Condition Variable which puts the 1st Customer
			// on to the Ready Queue
			if(EachCashierRegLen[CashierIndex]>0){
				//debug
				printf("  ImCashier_[%d]: Waiting Line Length = [%d]. Will decrease one...\n",
					   CashierIndex, EachCashierLineLength[CashierIndex]);
				//printf("Cashier: EachCashierLineCV[%d]->Signal(&CustToCashierLineLock);\n",CashierIndex);
	        	printf("\tEachCashierLineCV [%d] Signal CustToCashierLineLock\n",CashierIndex);
				EachCashierLineCV[CashierIndex]->Signal(&CustToCashierLineLock);
				EachCashierRegLen[CashierIndex]--;
				EachCashierLineLength[CashierIndex]--;
				EachCashierIsBusy[CashierIndex] = true;
			}else{
				printf("  ImCashier_[%d]: Waiting Line Length = 0 !!!\n",
				CashierIndex);
				EachCashierIsBusy[CashierIndex] = false;
				/*  Do I need this? Busy waiting?
				CustToCashierLineLock.Release();
				continue;
				*/
			}
        }
        // Acquire this Cashier's lock
		// We will use this lock to control the interactions between the Customer
		// and the Cashier
        EachCashierScanItemLock[CashierIndex]->Acquire();

        // After acquiring the above lock, we release the CustToCashierLineLock so who ever is waiting for the
		// lock can then search for the shortest line and then get into the appropriate line
        if(hasPrivilge == false){
        	CustToCashierLineLock.Release();
        }else{
        	PrvlCustLineLock.Release();
        }
		WholeLine.Release();

        // The Cashier must now wait for the Customer to go up to his counter
		// and give him his items (Just Cust Num, the items are stored in the matrix)
		// Sleeping the Cashier frees up his EachCashierScanItemLock,
		// wakes up one Customer and puts him on the
		// Ready Queue
        printf(" ## ImCashier_[%d]: EachCashierScanItemCV[CashierIndex]->Wait\n",
        		CashierIndex);
        EachCashierScanItemCV[CashierIndex]->Wait(EachCashierScanItemLock[CashierIndex]);
        EachCashierIsBusy[CashierIndex] = true;//!!!
        int CurCustID = CustIDforEachCashier[CashierIndex];
        printf("  ImCashier_[%d]: EachCashierIsBusy[%d] = true, by Cust [%d]\n",
        		CashierIndex,CashierIndex, CurCustID);

        // Now Cashier starts scanning items
        // CustIDforEachCashier[MyCashierNum] = CustID;

        float CustTotal = 0.0;
        for(int s=0; s<NUM_ITEM; s++){
            CustTotal += GoodsPrices[s]*
          //  CustDataArr[CurCustID].ShoppingList[s];
            		CustShoppingLists[CurCustID][s];
    		//CustDataArr[i].ShoppingList[j] = CustShoppingLists[i][j];
        }
        CurCustTotal[CashierIndex] = CustTotal;

        if(CustTotal > CustDataArr[CurCustID]->CashAmount){
        	// Insufficient Money!!!
        	CustDataArr[CurCustID]->InsufMoney = true;
    		EachCashierScanItemCV[CashierIndex]->Signal(EachCashierScanItemLock[CashierIndex]);
        	CashierToManagerLock.Acquire();
        	NumWaitingCashier++;
        	CashierWaitingCV->Wait(&CashierToManagerLock);
        	printf("  ImCashier_[%d]: tells Manager Cust [%d] has insufficient money\n",
        			CashierIndex,CurCustID);
        	CashierToManagerLock.Release();
        }else{
        	printf("  ImCashier_[%d]: asks Customer [%d] to pay [%.2f]\n",
               CashierIndex, CurCustID, CustTotal);//Correct!

			TotalAmountLock.Acquire();
			TotalAmount += CustTotal;
			TotalAmountLock.Release();
            // The Cashier signals the Customer, who is asleep waiting for
            // Cashier to scan his items and tell him the total amount
    		EachCashierScanItemCV[CashierIndex]->Signal(EachCashierScanItemLock[CashierIndex]);
        }

        EachCashierScanItemLock[CashierIndex]->Release();
       // printf("   Cashier DONE\n");
        //<< Interact with Customer
    }
}

int FindShortestCashierLine(int ECLineLength[], int N){
    assert(N>=1);
    int ShortestLength = NUM_CUSTOMER+1;
    int ShortestIndex = -1;
    for(int i=0; i<N; i++){
    	if(CashierIsOnBreak[i])continue;
        if(ECLineLength[i]<ShortestLength){
            ShortestLength = ECLineLength[i];
            ShortestIndex = i;
        }
    }
    return ShortestIndex;
}

