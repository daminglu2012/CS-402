// Customer-Salesman Test
// By Lu Sir, Linda, Yao, Stanley
// @ Jun 2nd, 2012, Unversity Of Southern California
#include <iostream>
#include "system.h"
#include "copyright.h"
#include "SupermarketSimulation.h"

using namespace std;

#ifdef CHANGED
#include "synch.h"
#endif

// Shared Data
// Lock trollyLock("trollyLock");

Lock *GoodsLock[NUM_ITEMS];
Condition *GoodsNotEnoughCV[NUM_ITEMS];

Lock CustWaitingLock("CustWaitingLock");
Condition CustWaitingCV("CustWaitingCV");

Lock *SalesmanLock[NUM_SALESMAN];
Condition *SalesmanCV[NUM_SALESMAN];

int TotalItems[NUM_ITEMS] = {1000, 1000, 1000, 1000, 1000};
int GoodsOnDemand[NUM_SALESMAN];
int ImCustNumber[NUM_SALESMAN];
int SalesmenStatus[NUM_SALESMAN] = {1, 1, 1};
int CustWaitingLineCount = 0;

// Customer Thread
void CustomerShopping(int ind) {
    // Start Shopping
    // shopping list
    int ShoppingList[NUM_ITEMS] = {1, 2, 1, 2, 1};

    for (int i = 0; i < NUM_ITEMS; i++) {
        if (ShoppingList[i] == 0) {
            continue;
        }

        GoodsLock[i]->Acquire();

        // Not Enough Goods, 
        // First go to waiting line to wait to see Salesman
        if (ShoppingList[i] > TotalItems[i]) {
            CustWaitingLock.Acquire(); // go to the waiting line

            // if salesmen are all busy, wait for some FREE salesman to signal me    
            bool allBusy = true;
            for (int j = 0; j < NUM_SALESMAN; j++) {
                if (SalesmenStatus[j] == 1) {
                    allBusy = false;
                    break;
                }
            }

            if (allBusy) {
                CustWaitingLineCount++;
                CustWaitingCV.Wait(&CustWaitingLock);
            }

            // Ready to meet some salesman, record his ID
            int mySalesInd;
            for (int j = 0; j < NUM_SALESMAN; j++) {
                if (SalesmenStatus[j] == 0) { // if some salesman is FREE, 
                    SalesmenStatus[j] = 3; // set his status to READY
                    mySalesInd = j;
                    break;
                }
            }

            cout << "Customer[" << ind 
                << "] is not able to find [" << i 
                << "] and is searching for DepartmentSalesmen[" << mySalesInd 
                << "]" << endl;

            CustWaitingLock.Release();

            // Interact with saleman with [mySalesInd]
            SalesmanLock[mySalesInd]->Acquire();
            SalesmenStatus[mySalesInd] = 1;        
            // tell salesman[mySalesInd] who I am, my index?(I am Complainting customer complainting Item[i])
            // and which Item I am complainting for
            
            WhoImTalkingTo[mySalesInd] = 1; // I am complainting customer
            ImCustNumber[mySalesInd] = ind; // my index is ind
            GoodsOnDemand[mySalesInd] = i; // ith goods are out of stock

            cout << "Customer [" << ind
                 << "] is asking for assistance about restocking of with DepartmentSalesman [" << mySalesInd
                 << "] of Department 1" << endl;

            SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
            GoodsNotEnoughCV[i]->Wait(GoodsLock[i]);
            cout << "Customer [" << ind 
                 << "] has received assistance about restocking ["
                 << i << "] from DepartmentSalesman" << endl;
            SalesmenStatus[mySalesInd] = 0;
            SalesmanLock[mySalesInd]->Release();
        }

        TotalItems[i] -= ShoppingList[i];
        cout << "Customer[" << ind
            << "] has found [" << i
            << "] and place [" << ShoppingList[i]
            << "] in the trolly" << endl;
        GoodsLock[i]->Release(); 
    }
    cout << "Customer[" << ind << "] has finished shopping in Department[1]" << endl;

    // cout << "Customer has found [item] and place [number] in the trolly" << endl;
    // cout << "Customer is not able to find [item] and is searching for DepartmentSalesmen[ind]" << endl;
    // cout << "Customer is asking for assistance about restocking of [item] from DepartmentSalesman[ind]" << endl;
    // cout << "Customer has received assistance about restocking [item] from DepartmentSalesman[ind]" << endl;
    // cout << "Customer has finished shopping in Department[]" << endl;
    // Finish Shopping
}

void SalesmanShopping(int ind) {
    while (1) {
        CustWaitingLock.Acquire();
        if (CustWaitingLineCount > 0) {
            CustWaitingCV.Signal(&CustWaitingLock);
            CustWaitingLineCount--;
            SalesmenStatus[ind] = 1; // Busy or READY?
        } else {
            SalesmenStatus[ind] = 0;
        }

        SalesmanLock[ind]->Acquire();
        CustWaitingLock.Release();
        SalesmanCV[ind]->Wait(SalesmanLock[ind]); // Wait for customer to come to me
        
        switch (WhoImTalkingTo[ind]) {
            case 0: // I am Talking to Greeting Customer

                break;
            case 1: // I am talking to Complainting Customer
                cout << "DepartmentSalesman[" << ind
                     << "] is informed by Customer[" << ImCustNumber[ind] 
                     << "] that [" << GoodsOnDemand[ind] 
                     << "] is out of stock" << endl;
                SalesmanCV[ind]->Release();

                GoodsLock[GoodsOnDemand[ind]]->Acquire();

                GoodsNotEnoughCV[GoodsOnDemand[ind]]->BroadCast(GoodsLock[GoodsOnDemand[ind]]);
                
                GoodsLock[GoodsOnDemand[ind]]->Release();

                break;
            case 2: // I am talking to Restocking man

                break;
            default:
                break;

        }

    }
}

void Stockman() {

}
