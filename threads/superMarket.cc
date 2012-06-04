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

#define MAX_CUSTOMER 30
#define MAX_SALESMAN 3

typedef enum {
    FREE = 0,
    BUSY,
    ONBREAK,
    READYFORSERVICE
} SalesStatus;

// Shared Data
Lock trollyLock("trollyLock");
Lock custSalesLock("custSalesLock"), salesmanLock[3]("salesmanLock");

Condition custWaitingCV("custWaitingCV"), salesmanCV[3]("salemanCV");

int salesCustNum[MAX_SALESMAN];    // monitor var

SalesStatus salesStatus[MAX_SALESMAN] = {BUSY, BUSY, BUSY};

int custWaitingLineCount = 0; // greeting customer
int complaintingWaitingLineCount = 0; // complainting customer
int goodsLoaderWaitingLineCount = 5;    // stock boy
int whoImTalkingTo[MAX_SALESMAN]; // 0 - CC(Complainting Customer)
                                  // 1 - GC(Greeting Customer)
                                  // 2 - GL(Goods Loader)
                                  // 3 - AVAILABLE

Lock complaintingCustSalesLock("complaintingCustSalesLock");
Condition complaintCustWaitingCV("complaintCustWaitingCV");

// Customer Thread
void Customer(int ind) {
    // Customer gets a trolly for shopping
    // cout << "Customer [" << ind
    //      << "] enters the SuperMarket" << endl;
    // cout << "Customer [" << ind
    //      << "] gets in line for a trolly" << endl;

    // trollyLock.Acquire();

    // cout << "Customer [" << ind
    //      << "] has a trolly for shopping" << endl;

    // trollyLock.Release();

    // Interact with multiple(up to 3) salesmen
    custSalesLock.Acquire();

    bool allBusy = true;
    for (int i = 0; i < 3; i++) {
        if (salesStatus[i] == FREE) {
            allBusy = false;
            break;
        }
    }

    if (allBusy) {
        cout << "Customer [" << ind
             << "] gets in line for Department 1" << endl;
        custWaitingLineCount++;
        custWaitingCV.Wait(&custSalesLock);
    }

    int mySalesInd;
    for (int i = 0; i < 3; i++) {
        if (salesStatus[i] == FREE) {
            salesStatus[i] = READYFORSERVICE;
            mySalesInd = i;
            break;
        } else if (salesStatus[i] == READYFORSERVICE) {
            mySalesInd = i;
            break;
        }
    }

    custSalesLock.Release();

    salesmanLock[mySalesInd].Acquire();
    salesStatus[mySalesInd] = BUSY;
    salesCustNum[mySalesInd] = ind; // salesman[mySalesInd] is interacting with customer[ind]
    cout << "Customer [" << ind
         << "] is interacting with DepartmentSalesman [" << mySalesInd
         << "] of Department 1" << endl;
    salesmanCV[mySalesInd].Signal(&salesmanLock[mySalesInd]); // signal the salesman who is waiting for me
    salesmanCV[mySalesInd].Wait(&salesmanLock[mySalesInd]); // 
    salesStatus[mySalesInd] = FREE;
    salesmanLock[mySalesInd].Release();

    // Start Shopping
    /*
        // shopping list
        custShoppingList[10] = {1, 0, 2, 0, 0, 1, 4, 1, 0, 0};
        
        for (int i = 0; i < 10; i++) {
            if (custShoppingList[i] == 0) {
                continue;
            }

            itemLock[i].Acquire();

            if (item[i] >= custShoppingList[i]) {
                cout << "Customer has found [item] and place [number] in the trolly" << endl;
                item[i] -= itemWantdNum;
                if (item[i] < 0)
                    item[i] = 0;

                itemLock[i].Release();
            } else {
                custSalesLock.Acquire();
                itemLock[i].Release();

                if (salesmen are all busy) {
                    complaintCustWaitingLineCount++;
                    complaintCustWaitingCV.Wait(&custSalesLock);
                }

                get mySalesInd;
                custSalesLock.Release();

                salesmanLock[mySalesInd].Acquire();
                salesStatus[mySalesInd] = BUSY;
                whoImTalkingTo[mySalesInd] = ind;
                cout << "Customer is asking for assistance about restocking of [item] from DepartmentSalesman[ind]" << endl;
                salesmanCV[mySalesInd].Signal(&salesmanLock[mySalesInd]); // Signal salesman who is waiting for me

                // Here I am, I will wait for your response
                salesmanCV[mySalesInd].Wait(&salesmanLock[mySalesInd]); // wait for salesman to inform the item is ready


                salesStatus[mySalesInd] = FREE;
                cout << "Customer has received assistance about restocking [item] from DepartmentSalesman[ind]" << endl;
                salesmanCV[mySalesInd].Release();

            }

        }
    */

    // cout << "Customer has found [item] and place [number] in the trolly" << endl;
    // cout << "Customer is not able to find [item] and is searching for DepartmentSalesmen[ind]" << endl;
    // cout << "Customer is asking for assistance about restocking of [item] from DepartmentSalesman[ind]" << endl;
    // cout << "Customer has received assistance about restocking [item] from DepartmentSalesman[ind]" << endl;
    // cout << "Customer has finished shopping in Department[]" << endl;
    // Finish Shopping
}

// Salesman Thread
// cout << "DepartmentSalesman [ind] is informed by Customer [ind] that [item] is out of stock" << endl;
// cout << "DepartmentSalesman [ind] informs the GoodsLoader [ind] that [item] is out of stock" << endl;
// cout << "DepartmentSalesman [ind] is informed by the GoodsLoader [ind] that [item] is restocked" << endl;
// cout << "DepartmentSalesman [ind] informs the Customer [ind] that [item] is restocked" << endl;
void Salesman(int ind) {
    while (1) {
        custSalesLock.Acquire();

        if (complaintingWaitingLineCount > 0) {
            salesmanCV[ind].Signal(salesmanLock[ind]);
        } else if (custWaitingLineCount > 0) {
            whoImTalkingTo[ind] = 1;
        } else if (goodsLoaderWaitingLineCount > 0) {
            whoImTalkingTo[ind] = 2;
        } else {
            whoImTalkingTo[ind] = 3;
        }

        /*
            Greeting Customer - GC
            Complainting Customer - CC
            Goods Loader - GL

            if (CC is in line) {
                
            } else if (GC is in line) {
                
            } else if (GL is in line) {
                
            } else { // I'm available
                
            }

            salesmanLock[ind].Acquire();
            custSalesLock.Release();
            salesmanCV[ind].Wait(&salesmanLock[ind]);
            int mycustInd = salesCustNum[ind];
            switch (whoImTalkingTo[ind]) {
                case 0: // CC
                    // TODO ... broadcase customers who are complainting for item X
                    break;
                case 1: // GC
                    // TODO ... cout << "Welcome Customer[myCustInd]" << endl;
                    break;
                case 2: // GL
                    // TODO ... load goods
                    break;
                case 3: // AVAILABLE
                    // TODO ...
                    salesStatus[ind] = FREE;
                    break;
            }
            salesmanLock[ind].Release();
        */

        if (custWaitingLineCount > 0) {
            custWaitingCV.Signal(&custSalesLock);
            custWaitingLineCount--;
            salesStatus[ind] = READYFORSERVICE;
        } else {
            salesStatus[ind] = FREE;
        }

        salesmanLock[ind].Acquire();
        custSalesLock.Release();

        salesmanCV[ind]->Wait(&salesmanLock[ind]);
        cout << "Salesman [" << ind
             << "] welcomes Customer [" << salesCustNum[ind]
             << "] for Department 1" << endl;
        salesStatus[ind] = BUSY;
        int myCustInd = salesCustNum[ind];

        salesmanCV[ind].Signal(&salesmanLock[ind]);
        salesmanLock[ind].Release();
    }
}

void TestCustSales() {
    for (int i = 1; i <= 30; i++) {
        Thread *t = new Thread("Customer");
        t->Fork((VoidFunctionPtr)Customer, i);
    }
}
