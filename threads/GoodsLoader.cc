/*
 * GoodsLoader.cc
 *
 *  Created on: Jun 14, 2012
 *      Author: hearcockcrow
 */


#include "SupermarketSimulation.h"

Lock* GoodsLock[NUM_ITEM];
int GoodsOnDemand[NUM_SALESMAN];
Condition* GoodsNotEnoughCV[NUM_ITEM];
int GoodsLoaderStatus[NUM_GOODSLOADER];
int SalesmanWaitingLineCount = 0; // for Goods Loader

Lock FreeGoodsLoaderLock("FreeGoodsLoaderLock");
Condition FreeGoodsLoaderCV("FreeGoodsLoaderCV");

Lock *GoodsLoaderLock[NUM_GOODSLOADER];
Condition *GoodsLoaderCV[NUM_GOODSLOADER];

bool GoodsIsFull[NUM_ITEM] = {
		false, false, false, false, false,
		false, false, false, false, false
};

int TotalItems[NUM_ITEM] = {
	MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM,
	MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM, MAX_ITEM
};

Lock StockRoomLock("StockRoomLock");
Condition StockRoomCV("StockRoomCV");

int StockRoomWaitingLineCount = 0;
int WhoIsInTheStockRoom = -1;
int ImSalesmanNumber[NUM_GOODSLOADER];
int ImGoodsLoaderNumber[NUM_SALESMAN];

// GoodsLoader
void GoodsLoader(int ind) {

    while (1) {
		FinishedCustLock.Acquire();
		if(FinishedCust>=NUM_CUSTOMER){
			return;
		}
		FinishedCustLock.Release();

        FreeGoodsLoaderLock.Acquire();

        if (SalesmanWaitingLineCount > 0) {
            FreeGoodsLoaderCV.Signal(&FreeGoodsLoaderLock);
            SalesmanWaitingLineCount--;
            GoodsLoaderStatus[ind] = 3; // Ready
        } else {
            cout << "GoodsLoader [" << ind
                 << "] is waiting for orders to restock" << endl;
            GoodsLoaderStatus[ind] = 0; // not busy
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
        // cout << "GoodsLoader [" << ind << "] responds to DepartmentSalesman [" << ImSalesmanNumber[ind] << "]: Roger That" << endl;
        GoodsLoaderLock[ind]->Release();

        // Load Goods
        for (int i = 0; i < MAX_ITEM; i++) {
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Acquire();
            if (TotalItems[GoodsOnDemand[ImSalesmanNumber[ind]]] == MAX_ITEM) {
                GoodsIsFull[GoodsOnDemand[ImSalesmanNumber[ind]]] = true;
                GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();
                break;
            }
            // TotalItems[GoodsOnDemand[ImSalesmanNumber[ind]]]++;
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();

            printf("GL[%d]: Salesman [%d] told me Item [%d] needs reload\n",
            		ind,ImSalesmanNumber[ind], GoodsOnDemand[ImSalesmanNumber[ind]]);

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

            // cout << "GoodsLoader [" << ind
            //      << "] is in the StockRoom and got ["
            //      << GoodsOnDemand[ImSalesmanNumber[ind]]
            //      << "]" << endl;

            for (int j = 0; j < 10; j++) {
                currentThread->Yield();
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
            // cout << "GoodsLoader [" << ind << "] add one item to [" << GoodsOnDemand[ImSalesmanNumber[ind]] << "]" << endl;
            GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();
        }

		// GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Acquire();
		// if (!GoodsIsFull[GoodsOnDemand[ImSalesmanNumber[ind]]]) {
		cout << "GoodsLoader [" << ind
			 << "] has restocked [" << GoodsOnDemand[ImSalesmanNumber[ind]]
			 << "] in Department [1]" << endl;
		ImGoodsLoaderNumber[ImSalesmanNumber[ind]] = ind;
		// Load Goods Done!

		/* Go to Cust Waiting Line to inform FREE salesman that the [item] is ready */
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
		ImCustNumber[mySalesInd] = ind;
		WhoImTalkingTo[mySalesInd] = 2; // I am Goods Loader
		// cout << "GoodsLoader [" << ind
		//      << "] is interacting with DepartmentSalesman [" << mySalesInd
		//      << "] of Department [1]" << endl;
		SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
		CustWaitingLock.Release();
		SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
		// Go back to be a FREE GoodsLoader who waits for the next salesman to inform me
		SalesmanLock[mySalesInd]->Release();
	// }
	// GoodsLock[GoodsOnDemand[ImSalesmanNumber[ind]]]->Release();
    }
}
