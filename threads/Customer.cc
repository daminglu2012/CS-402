/*
 Customer interacts with:
 1) Salesman
 2) Cashier
 3) Manager
 */

#include "SupermarketSimulation.h"
#include <vector>
#include <algorithm>
//bool CustDebugMode = true;//If Debug is off, then all the code will be executed
//int CustDebugIndex = 2;

//1: Cust-Sales
//2: Cust-Cashier
//3: ...

//>> Trolly is just one lock for now(Jun 4)
Lock TrollyLock("OneTrollyLock");
//<< Trolly is just one lock for now(Jun 4)

vector<int> DoneCust;

CustDebugModeName_T CustDebugModeName = Cust_Cashier;

Lock FinishedCustLock("FinishedCustLock");

void RemoveOneItem(int CustID);

void Customer(int CustID) {
	if (ManagerCustCashierDebugMode && MCC_DebugName == Test_Everything) {

		if(false){
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
	        // cout << "Customer [" << ind
	        //      << "] gets in line for the Department [1]"
	        //      << endl;
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
	    ImCustNumber[mySalesInd] = CustID;
	    WhoImTalkingTo[mySalesInd] = 0;
	    // cout << "Customer [" << ind
	    //      << "] is interacting with DepartmentSalesman [" << mySalesInd
	    //      << "] of Department [1]" << endl;
	    SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
	    CustWaitingLock.Release();
	    SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
	    cout << "Customer [" << CustID << "] starts shopping" << endl; // Debug purpose
	    SalesmanLock[mySalesInd]->Release();

	    /* ***************Start Shopping*************** */
	    // Start Shopping
	    // shopping list
	    int ShoppingList[NUM_ITEM];
	    for(int i=0; i<NUM_ITEM; i++){
	    	ShoppingList[i] = CustDataArr[CustID]->ShoppingList[i];
	    }

	    for (int i = 0; i < NUM_ITEM; i++) {
	        if (ShoppingList[i] == 0) {
	            continue;
	        }

	        GoodsLock[i]->Acquire();
	        // Not Enough Goods,
	        // First go to waiting line to wait to see Salesman
	        while (ShoppingList[i] > TotalItems[i]) {
	            if (TotalItems[i] != 0) {
	                // cout << "Customer [" << ind
	                //     << "] has found [" << i
	                //     << "] and place [" << TotalItems[i]
	                //     << "] in the trolly" << endl;
	                ShoppingList[i] -= TotalItems[i];
	                TotalItems[i] = 0;
	            }
	            GoodsLock[i]->Release();

	            CustWaitingLock.Acquire(); // go to the waiting line

	            // if salesmen are all busy, wait for some FREE salesman to signal me
	            allBusy = true;
	            for (int j = 0; j < NUM_SALESMAN; j++) {
	                if (SalesmenStatus[j] == 0) {
	                    allBusy = false;
	                    break;
	                }
	            }

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
	            // cout << "Customer [" << ind
	            //     << "] is not able to find [" << i
	            //     << "] and is searching for DepartmentSalesmen[" << mySalesInd
	            //     << "]" << endl;

	            // Interact with saleman with [mySalesInd]
	            SalesmanLock[mySalesInd]->Acquire();
	            // tell salesman[mySalesInd] who I am, my index?(I am Complainting customer complainting Item[i])
	            // and which Item I am complainting for

	            WhoImTalkingTo[mySalesInd] = 1; // I am complainting customer
	            ImCustNumber[mySalesInd] = CustID; // my index is ind
	            GoodsOnDemand[mySalesInd] = i; // ith goods are out of stock

	            // cout << "Customer [" << ind
	            //      << "] is asking for assistance about restocking of ["
	            //      << i << "] with DepartmentSalesman [" << mySalesInd
	            //      << "] of Department 1" << endl;

	            SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
	            CustWaitingLock.Release();
	            SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
	            SalesmanLock[mySalesInd]->Release();

	            GoodsLock[i]->Acquire();
	            GoodsNotEnoughCV[i]->Wait(GoodsLock[i]);
	            // cout << "Customer [" << ind
	            //      << "] has received assistance about restocking ["
	            //      << i << "] from DepartmentSalesman ["
	            //      << mySalesInd << "]" << endl;
	        }

	        TotalItems[i] -= ShoppingList[i];
	        // cout << "Customer [" << ind
	        //     << "] has found [" << i
	        //     << "] and place [" << ShoppingList[i]
	        //     << "] in the trolly" << endl;
	        GoodsLock[i]->Release();
	    }
	    //cout << "Customer [" << ind << "] has finished shopping in Department[1]" << endl;
		}//debug
		//-------------------------------------------------------------------------
		printf("Customer [%d] finished shopping and is about to check out\n",
				CustID);
		//-------------------------------------------------------------------------

		//>> Interacts with Cashiers
		// Assume Cust k has got all the items on his list CustShoppingLists[k][10]
		// Assume every Cust has enough money

		WholeLine.Acquire();
		bool isReleased = false;
		printf("Cust[%d] holds WholeLine1.\n",CustID);

		if (CustDataArr[CustID]->CustRole == PRIVILEGE) {
			PrvlCustLineLock.Acquire();
			printf("Cust[%d] holds PrvlCustLineLock1.\n",CustID);

		} else {
			CustToCashierLineLock.Acquire();
			printf("Cust[%d] holds CustToCashierLineLock1.\n",CustID);

		}

		int MyCashierNum = FindShortestCashierLine(EachCashierLineLength,
				NUM_CASHIER);
		printf("Customer [%d] chose Cashier [%d] with a line length of [%d]\n",
				CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);

		if (EachCashierLineLength[MyCashierNum] > 0 // problem, race condition
				|| EachCashierIsBusy[MyCashierNum]) {
			EachCashierLineLength[MyCashierNum]++;
			if (CustDataArr[CustID]->CustRole == PRIVILEGE) {
				//privilege
				printf(
						"Cust [%d] waits in Prvl Q, now Cashier [%d] Waiting Line Length = [%d]. \n",
						CustID, MyCashierNum,
						EachCashierLineLength[MyCashierNum]);
				EachCashierPrvLen[MyCashierNum]++;
				printf("Cust[%d] waits Cashier[%d] (Privilege)\n",
						CustID, MyCashierNum);
				WholeLine.Release();
				isReleased = true;
				EachCashierPrvlLineCV[MyCashierNum]->Wait(&PrvlCustLineLock);
				printf("Cust[%d]wokeupPrvl\n",CustID);
				//WholeLine.Acquire();
				// My Cashier finally calls me
				EachCashierIsBusy[MyCashierNum] = true; // dupli on purpose
			} else {
				//regular
				printf(
						"Cust [%d] waits in Regu Q, now Cashier [%d] Waiting Line Length = [%d]. \n",
						CustID, MyCashierNum,
						EachCashierLineLength[MyCashierNum]);
				EachCashierRegLen[MyCashierNum]++;
				printf("Cust[%d] waits Cashier[%d] (Regular)\n",
						CustID, MyCashierNum);
				WholeLine.Release();
				isReleased = true;
				printf("Cust[%d] released WholeLine1.\n",CustID);
				EachCashierLineCV[MyCashierNum]->Wait(&CustToCashierLineLock);
				printf("Cust[%d]wokeupReg\n",CustID);
				//WholeLine.Acquire();
				//printf("Cust[%d] holds WholeLine2.\n",CustID);
				// My Cashier finally calls me
				EachCashierIsBusy[MyCashierNum] = true; // dupli on purpose
			}
		}

		EachCashierScanItemLock[MyCashierNum]->Acquire();
		EachCashierIsBusy[MyCashierNum] = true;
		// After acquiring my Cashier's lock, we release the Line Lock
		// so any Cashier that is waiting for the
		// Line Lock can call his next Customer
		// Or another Customer can line up for his Cashier
		if (CustDataArr[CustID]->CustRole == PRIVILEGE) {
			PrvlCustLineLock.Release();
		} else {
			CustToCashierLineLock.Release();
		}
		if(!isReleased){
			WholeLine.Release();
			printf("Cust[%d] released WholeLine2.\n",CustID);
		}


		// 'give' my items to my Cashier
		CustIDforEachCashier[MyCashierNum] = CustID;
		//EachCashierIsBusy[MyCashierNum] = true;//!!!
		printf("Cust %d signals CASHIER %d\n", CustID, MyCashierNum);
		EachCashierScanItemCV[MyCashierNum]->Signal(
				EachCashierScanItemLock[MyCashierNum]);

		// Wait Cashier to scan my items
		EachCashierScanItemCV[MyCashierNum]->Wait(
				EachCashierScanItemLock[MyCashierNum]);
		printf(" Customer [%d]: Cashier [%d] finished scanning\n",
				CustID, MyCashierNum);
		if (CustDataArr[CustID]->InsufMoney) {
			// Insufficient Money, needs to talk to Manager
			CustToManagerLock.Acquire();
			CurInsufCustID = CustID; //MyID
			printf("Cust %d needs to meet Manager\n", CustID);
			InsufCustWaitingCV->Signal(&CustToManagerLock);

			while (CustDataArr[CustID]->InsufMoney) {
				WaitForCheckCV->Wait(&CustToManagerLock);
				if (CustDataArr[CustID]->InsufMoney) {
					printf(" Cust[%d] still insuf, needs to remove one item\n",
							CustID);
					RemoveOneItem(CustID);
				} else {
					printf(" I can finally afford it! Goodbye! \n");
					WaitForCheckCV->Signal(&CustToManagerLock);
					CustToManagerLock.Release();
					break;
				}
				WaitForCheckCV->Signal(&CustToManagerLock);
			}

		} else {
			printf("The total amount for myself (Customer [%d]) is %.2f\n",
					CustID, CurCustTotal[MyCashierNum]);
		}

		EachCashierScanItemLock[MyCashierNum]->Release();

		FinishedCustLock.Acquire();
		DoneCust.push_back(CustID);
		sort(DoneCust.begin(), DoneCust.end());
		printf(" CUSTOMER: Cust [%d] DONE!  ", CustID);
		printf(" Total Done Cust == [%d] : ", DoneCust.size());
//		for(int i=0; i<DoneCust.size(); i++){
//			printf("%d ", DoneCust[i]);
//		}
		printf("\n");
		FinishedCust++;
		FinishedCustLock.Release();
		//printf("FinishedCust == %d",FinishedCust);
	}
}

void RemoveOneItem(int CustID) {
	for (int i = 0; i < NUM_ITEM; i++) {
		int temp = CustDataArr[CustID]->ShoppingList[i];
		if (temp > 0) {
			CustDataArr[CustID]->ShoppingList[i] = temp - 1;
			return;
		}
	}
}
