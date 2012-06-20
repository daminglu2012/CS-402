// Customer-Salesman Test
// By Lu Sir, Linda, Yao, Stanley
// @ Jun 2nd, 2012, Unversity Of Southern California
#include <iostream>
#include "system.h"
#include "copyright.h"
#include "thread.h"
#include "SupermarketSimulation.h"

using namespace std;

#ifdef CHANGED
#include "synch.h"
#endif

// Shared Data
// Lock trollyLock("trollyLock");

Lock *GoodsLock[NUM_ITEMS];
Lock *GoodsLoadingLock[NUM_ITEMS];
Lock *TotalItemsLock[NUM_ITEMS];
Condition *GoodsNotEnoughCV[NUM_ITEMS];

Lock CustWaitingLock("CustWaitingLock");
Condition CustWaitingCV("CustWaitingCV");

Lock SalesmanWaitingLock("SalesmanWaitingLock");

Lock FreeGoodsLoaderLock("FreeGoodsLoaderLock");
Condition FreeGoodsLoaderCV("FreeGoodsLoaderCV");
Lock *GoodsLoaderLock[NUM_GOODSLOADER];
Condition *GoodsLoaderCV[NUM_GOODSLOADER];

Lock StockRoomLock("StockRoomLock");
Condition StockRoomCV("StockRoomCV");

Lock *SalesmanLock[NUM_SALESMAN];
Condition *SalesmanCV[NUM_SALESMAN];

int TotalItems[NUM_ITEMS] = {MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM}; // not scalable
int GoodsOnDemand[NUM_SALESMAN];
int isBeingLoaded[NUM_ITEMS] = {0, 0, 0, 0, 0};
int WhoImTalkingTo[NUM_SALESMAN];
int ImCustNumber[NUM_SALESMAN];
int ImGoodsLoaderNumber[NUM_SALESMAN];
int ImSalesmanNumber[NUM_GOODSLOADER];
int SalesmenStatus[NUM_SALESMAN] = {1, 1, 1};
int GoodsLoaderStatus[NUM_GOODSLOADER] = {1, 1, 1};
bool GoodsIsFull[NUM_ITEMS] = {false, false, false, false, false}; // not scalable

int CustWaitingLineCount = 0;
int SalesmanWaitingLineCount = 0;
int StockRoomWaitingLineCount = 0;
int WhoIsInTheStockRoom;

// Customer Thread
void CustomerShopping(int ind) {
    /****************** Customer Greeting For Goods ******************/
    CustWaitingLock.Acquire();

    bool allBusy = true;
    for (int j = 0; j < NUM_SALESMAN; j++) {
        if (SalesmenStatus[j] == 0) { // if one salesman is FREE
            allBusy = false;
            break;
        }
    }

    int mySalesInd;
    if (allBusy) {
        CustWaitingLineCount++;
        cout << "Customer [" << ind
             << "] gets in line for the Department [1]"
             << endl;
        CustWaitingCV.Wait(&CustWaitingLock);
        for (int j = 0; j < NUM_SALESMAN; j++) {
            if (SalesmenStatus[j] == 3) {
                SalesmenStatus[j] = 1;
                mySalesInd = j;
                break;
            }
        }
    } else {
        for (int j = 0; j < NUM_SALESMAN; j++) {
            if (SalesmenStatus[j] == 0) {
                SalesmenStatus[j] = 1;
                mySalesInd = j;
                break;
            }
        }
    }

    SalesmanLock[mySalesInd]->Acquire();
    ImCustNumber[mySalesInd] = ind;
    WhoImTalkingTo[mySalesInd] = 0;
    cout << "Customer [" << ind 
         << "] is interacting with DepartmentSalesman [" << mySalesInd 
         << "] of Department [1]" << endl;        
    SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
    CustWaitingLock.Release();
    SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
    cout << "Customer [" << ind << "] starts shopping" << endl; // Debug purpose
    SalesmanLock[mySalesInd]->Release();

    /* ***************Start Shopping*************** */
    // Start Shopping
    // shopping list
    int ShoppingList[NUM_ITEMS] = {6, 0, 0, 0, 0};

    for (int i = 0; i < NUM_ITEMS; i++) {
        if (ShoppingList[i] == 0) {
            continue;
        }

        GoodsLock[i]->Acquire();
        // Not Enough Goods,
        // First go to waiting line to wait to see Salesman
        while (ShoppingList[i] > TotalItems[i]) {
            if (TotalItems[i] != 0) {
                cout << "Customer [" << ind
                    << "] has found [" << i
                    << "] and place [" << TotalItems[i]
                    << "] in the trolly" << endl;
                ShoppingList[i] -= TotalItems[i];
                TotalItems[i] = 0;
            }

            CustWaitingLock.Acquire(); // go to the waiting line

            // if salesmen are all busy, wait for some FREE salesman to signal me    
            allBusy = true;
            for (int j = 0; j < NUM_SALESMAN; j++) {
                if (SalesmenStatus[j] == 0) {
                    allBusy = false;
                    break;
                }
            }

            mySalesInd;
            if (allBusy) { // if salesmen are all busy, then wait till some salesman are READY
                CustWaitingLineCount++;
                CustWaitingCV.Wait(&CustWaitingLock);

                for (int j = 0; j < NUM_SALESMAN; j++) {
                    if (SalesmenStatus[j] == 3) {
                        SalesmenStatus[j] = 1;
                        mySalesInd = j;
                        break;
                    }
                }
            } else {
                for (int j = 0; j < NUM_SALESMAN; j++) {
                    if (SalesmenStatus[j] == 0) {
                        SalesmenStatus[j] = 1;
                        mySalesInd = j;
                        break;
                    }
                }                
            }

            // Ready to meet some salesman, record his ID
            cout << "Customer [" << ind 
                << "] is not able to find [" << i 
                << "] and is searching for DepartmentSalesmen[" << mySalesInd 
                << "]" << endl;

            // Interact with saleman with [mySalesInd]
            SalesmanLock[mySalesInd]->Acquire();
            // tell salesman[mySalesInd] who I am, my index?(I am Complainting customer complainting Item[i])
            // and which Item I am complainting for
            
            WhoImTalkingTo[mySalesInd] = 1; // I am complainting customer
            ImCustNumber[mySalesInd] = ind; // my index is ind
            GoodsOnDemand[mySalesInd] = i; // ith goods are out of stock
            // GoodsOnDemandNum[i] = (TotalItems[i] - ShoppingList[i]); // How many ith goods I still need

            cout << "Customer [" << ind
                 << "] is asking for assistance about restocking of [" 
                 << i << "] with DepartmentSalesman [" << mySalesInd
                 << "] of Department 1" << endl;

            SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
            CustWaitingLock.Release();
            SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
            SalesmanLock[mySalesInd]->Release();

            GoodsNotEnoughCV[i]->Wait(GoodsLock[i]);
            cout << "Customer [" << ind 
                 << "] has received assistance about restocking ["
                 << i << "] from DepartmentSalesman [" 
                 << mySalesInd << "]" << endl;      
        }

        TotalItems[i] -= ShoppingList[i];
        cout << "Customer [" << ind
            << "] has found [" << i
            << "] and place [" << ShoppingList[i]
            << "] in the trolly" << endl;
        GoodsLock[i]->Release(); 
    }
    cout << "Customer [" << ind << "] has finished shopping in Department[1]" << endl;
}

// GoodsLoader
void GoodsLoader(int ind) {
    
    while (1) {
        FreeGoodsLoaderLock.Acquire();

        if (SalesmanWaitingLineCount > 0) {
            FreeGoodsLoaderCV.Signal(&FreeGoodsLoaderLock);
            SalesmanWaitingLineCount--;
            GoodsLoaderStatus[ind] = 3; // Ready
        } else {
            cout << "GoodsLoader [" << ind
                 << "] is waiting for orders to restock" << endl;
            GoodsLoaderStatus[ind] = 0;
        }

        GoodsLoaderLock[ind]->Acquire();
        FreeGoodsLoaderLock.Release();
        GoodsLoaderCV[ind]->Wait(GoodsLoaderLock[ind]);

        cout << "GoodsLoader [" << ind 
             << "] is informed by DepartmentSalesman [" 
             << ImSalesmanNumber[ind] 
             << "] of Department [1] to restock [" 
             << GoodsOnDemand[ImSalesmanNumber[ind]] << "]" << endl;
        GoodsLoaderCV[ind]->Signal(GoodsLoaderLock[ind]);
        GoodsLoaderLock[ind]->Release();

        // Im going to Load Goods
        for (int i = 0; i < MAX_ITEM; i++) {
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Acquire();
            if (TotalItems[GoodsOnDemand[ImSalesmanNumber[ind]]] == MAX_ITEM) {
                GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();
                break;
            }
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();

            StockRoomLock.Acquire();
            if (StockRoomWaitingLineCount == 0) {
                WhoIsInTheStockRoom = ind;
            } else {
                cout << "GoodsLoader [" << ind
                     << "] is waiting for GoodsLoader [" 
                     << WhoIsInTheStockRoom
                     << "] to leave the StockRoom" << endl;
                StockRoomWaitingLineCount++;
                StockRoomCV.Wait(&StockRoomLock);
                WhoIsInTheStockRoom = ind;
            }
            
            if (StockRoomWaitingLineCount > 0) {
                StockRoomWaitingLineCount--;
                StockRoomCV.Signal(&StockRoomLock);
            }
            
            StockRoomLock.Release();

            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Acquire();
            cout << "GoodsLoader [" << ind << "] leaves StockRoom" << endl;
            for (int j = 0; j < 10; j++) {
                currentThread->Yield();
            }
            TotalItems[GoodsOnDemand[ImSalesmanNumber[ind]]]++;
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();
        }
        // Load Goods Done!

        // Go to Cust Waiting Line to inform FREE salesman that the [item] is ready 
        cout << "GoodsLoader [" << ind
             << "] has restocked [" << GoodsOnDemand[ImSalesmanNumber[ind]] 
             << "] in Department [1]" << endl;            
    
        CustWaitingLock.Acquire();

        bool allBusy = true;
        for (int j = 0; j < NUM_SALESMAN; j++) {
            if (SalesmenStatus[j] == 0) { // if one salesman is FREE
                allBusy = false;
                break;
            }
        }

        int mySalesInd;
        if (allBusy) {
            CustWaitingLineCount++;
            cout << "GoodsLoader [" << ind
                 << "] gets in line for the Department [1]"
                 << endl;
            CustWaitingCV.Wait(&CustWaitingLock);
            for (int j = 0; j < NUM_SALESMAN; j++) {
                if (SalesmenStatus[j] == 3) {
                    SalesmenStatus[j] = 1;
                    mySalesInd = j;
                    break;
                }
            }
        } else {
            for (int j = 0; j < NUM_SALESMAN; j++) {
                if (SalesmenStatus[j] == 0) {
                    SalesmenStatus[j] = 1;
                    mySalesInd = j;
                    break;
                }
            }
        }

        SalesmanLock[mySalesInd]->Acquire();
        ImGoodsLoaderNumber[mySalesInd] = ind;
        WhoImTalkingTo[mySalesInd] = 2; // I am Goods Loader
        // cout << "GoodsLoader [" << ind 
        //      << "] is interacting with DepartmentSalesman [" << mySalesInd 
        //      << "] of Department [1]" << endl;        
        SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
        CustWaitingLock.Release();
        SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
        // Go back to be a FREE GoodsLoader who waits for the next salesman to inform me
        SalesmanLock[mySalesInd]->Release();
    }
}

void SalesmanShopping(int ind) {
    bool allBusy = true;
    while (1) {
        CustWaitingLock.Acquire();
        if (CustWaitingLineCount > 0) {
            CustWaitingCV.Signal(&CustWaitingLock);
            CustWaitingLineCount--;
            SalesmenStatus[ind] = 3; // Ready
        } else {
            SalesmenStatus[ind] = 0;
        }

        SalesmanLock[ind]->Acquire();
        CustWaitingLock.Release();
        SalesmanCV[ind]->Wait(SalesmanLock[ind]); // Wait for customer to come to me

        switch (WhoImTalkingTo[ind]) {
            case 0: // I am Talking to Greeting Customer
                cout << "DepartmentSalesman [" << ind
                     << "] welcomes Customer [" 
                     << ImCustNumber[ind] 
                     << "] to Department [1]" << endl;
                SalesmanCV[ind]->Signal(SalesmanLock[ind]);
                SalesmanLock[ind]->Release();
                break;
            case 1: // I am talking to Complainting Customer
                cout << "DepartmentSalesman [" << ind
                     << "] is informed by Customer [" << ImCustNumber[ind] 
                     << "] that [" << GoodsOnDemand[ind] 
                     << "] is out of stock" << endl;
                SalesmanCV[ind]->Signal(SalesmanLock[ind]);
                // cout << "DepartmentSalesman [" << ind << "] got Customer [" << ImCustNumber[ind] << "]'s request, Please Wait for my Notification" << endl;
                SalesmanLock[ind]->Release();

                GoodsLoadingLock[GoodsOnDemand[ind]]->Acquire();
                if (isBeingLoaded[GoodsOnDemand[ind]] == 0) {
                    isBeingLoaded[GoodsOnDemand[ind]] = 1;
                    // Find Available GoodsLoader to restock the item
                    FreeGoodsLoaderLock.Acquire();

                    allBusy = true;
                    for (int j = 0; j < NUM_GOODSLOADER; j++) {
                        if (GoodsLoaderStatus[j] == 0) {
                            allBusy = false;
                            break;
                        }
                    }

                    int myGoodsLoader;
                    if (allBusy) {
                        SalesmanWaitingLineCount++;
                        FreeGoodsLoaderCV.Wait(&FreeGoodsLoaderLock);
                        for (int j = 0; j < NUM_GOODSLOADER; j++) {
                            if (GoodsLoaderStatus[j] == 3) {
                                myGoodsLoader = j;
                                GoodsLoaderStatus[j] = 1;
                                break;
                            }
                        }
                    } else {
                        for (int j = 0; j < NUM_GOODSLOADER; j++) {
                            if (GoodsLoaderStatus[j] == 0) {
                                myGoodsLoader = j;
                                GoodsLoaderStatus[j] = 1;
                                break;
                            }
                        }
                    }

                    GoodsLoaderLock[myGoodsLoader]->Acquire();
                    ImSalesmanNumber[myGoodsLoader] = ind;
                    cout << "DepartmentSalesman [" << ind
                         << "] informs the GoodsLoader [" << myGoodsLoader
                         << "] that [" << GoodsOnDemand[ind]
                         << "] is out of stock" << endl;
                    GoodsLoaderCV[myGoodsLoader]->Signal(GoodsLoaderLock[myGoodsLoader]);
                    FreeGoodsLoaderLock.Release();
                    // cout << "DepartmentSalesman [" << ind
                    //      << "] is Waiting for GoodsLoader ["
                    //      << myGoodsLoader << "] to say Roger that" << endl;
                    GoodsLoaderCV[myGoodsLoader]->Wait(GoodsLoaderLock[myGoodsLoader]);
                    GoodsLoaderLock[myGoodsLoader]->Release();

                }
                GoodsLoadingLock[GoodsOnDemand[ind]]->Release();
                break;
            case 2: // I am talking to Restocking man
                cout << "DepartmentSalesman [" << ind 
                     << "] is informed by the GoodsLoader [" 
                     << ImGoodsLoaderNumber[ind] << "] that [" 
                     << GoodsOnDemand[ind] << "] is restocked" << endl;
                SalesmanCV[ind]->Signal(SalesmanLock[ind]);
                SalesmanLock[ind]->Release();

                GoodsLoadingLock[GoodsOnDemand[ind]]->Acquire();
                isBeingLoaded[GoodsOnDemand[ind]] = 0;
                GoodsLoadingLock[GoodsOnDemand[ind]]->Release();

                // After the salesman is informed by the Goods loader the ith item is ready
                // he broadcast customers waiting for ith item
                GoodsLock[GoodsOnDemand[ind]]->Acquire();
                cout << "DepartmentSalesman [" << ind 
                     << "] informs the Customer [" 
                     << ImCustNumber[ind] << "] that [" 
                     << GoodsOnDemand[ind] << "] is restocked" << endl;
                GoodsNotEnoughCV[GoodsOnDemand[ind]]->Broadcast(GoodsLock[GoodsOnDemand[ind]]);
                
                GoodsLock[GoodsOnDemand[ind]]->Release();
                break;        
        } // end of switch
    } // end of while
}
