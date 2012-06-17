/*
Customer interacts with:
1) Salesman
2) Cashier
3) Manager
*/

#include "SupermarketSimulation.h"
//bool CustDebugMode = true;//If Debug is off, then all the code will be executed
//int CustDebugIndex = 2;

//1: Cust-Sales
//2: Cust-Cashier
//3: ...

//>> Trolly is just one lock for now(Jun 4)
Lock TrollyLock("OneTrollyLock");
//<< Trolly is just one lock for now(Jun 4)


CustDebugModeName_T CustDebugModeName = Cust_Cashier;


void RemoveOneItem(int CustID);

void Customer(int CustID){
    //>> Customer get Trolly
	/*
    TrollyLock.Acquire();
    printf("Customer [%d] got a trolly\n",CustID);
    TrollyLock.Release();
    */
    //<< Customer get Trolly

    //-------------------------------------------------------------------------------

    // After securing the trolly, they can shop in any Department(there can be 1 to 5).

    //-------------------------------------------------------------------------------

    //>> Interacts with Salesman
    if(CustDebugMode && CustDebugModeName==Cust_Sales)
    {
    	CustWaitingLock.Acquire();
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
            CustWaitingCV.Wait(&CustWaitingLock);
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
        SalesWaitingCV.Signal(&CustWaitingLock);
        CustWaitingLock.Release();
    }
    //<< Interacts with Salesman

    //-------------------------------------------------------------------------------

    //>> Interacts with Cashier
    // ADD: and Manager
    if(ManagerCustCashierDebugMode && MCC_DebugName==Manager_Cust_Cashier)
    {
    	//>> Interacts with Salesman

    	//<< Interacts with Salesman

    	//>> Interacts with Cashiers
        // Assume Cust k has got all the items on his list CustShoppingLists[k][10]
        // Assume every Cust has enough money
        if(CustDataArr[CustID]->CustRole == PRIVILEGE){
        	PrvlCustLineLock.Acquire();
        }else{
        	CustToCashierLineLock.Acquire();
        }

        int MyCashierNum = FindShortestCashierLine(EachCashierLineLength, NUM_CASHIER);
        printf("Customer [%d] chose Cashier [%d] with a line length of [%d]\n",
               CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);

        if(EachCashierLineLength[MyCashierNum]>0 || EachCashierIsBusy[MyCashierNum]){
            EachCashierLineLength[MyCashierNum]++;
            if(CustDataArr[CustID]->CustRole == PRIVILEGE){
            	//privilege
                printf("Cust [%d] waits in Prvl Q, now Cashier [%d] Waiting Line Length = [%d]. \n",
                		CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);
                EachCashierPrvLen[MyCashierNum]++;
                EachCashierPrvlLineCV[MyCashierNum]->Wait(&PrvlCustLineLock);
				// My Cashier finally calls me
                EachCashierIsBusy[MyCashierNum] = true; // dupli on purpose
            }else{
            	//regular
				printf("Cust [%d] waits in Regu Q, now Cashier [%d] Waiting Line Length = [%d]. \n",
						CustID, MyCashierNum, EachCashierLineLength[MyCashierNum]);
				EachCashierRegLen[MyCashierNum]++;
				EachCashierLineCV[MyCashierNum]->Wait(&CustToCashierLineLock);
				// My Cashier finally calls me
				EachCashierIsBusy[MyCashierNum] = true; // dupli on purpose
            }
        }

        // After acquiring my Cashier's lock, we release the Line Lock
        // so any Cashier that is waiting for the
		// Line Lock can call his next Customer
		// Or another Customer can line up for his Cashier
        if(CustDataArr[CustID]->CustRole == PRIVILEGE){
        	PrvlCustLineLock.Release();
        }else{
        	CustToCashierLineLock.Release();
        }

        EachCashierScanItemLock[MyCashierNum]->Acquire();

        // 'give' my items to my Cashier
        CustIDforEachCashier[MyCashierNum] = CustID;
        printf("Cust %d signals CASHIER %d\n",CustID, MyCashierNum);
        EachCashierScanItemCV[MyCashierNum]->Signal(EachCashierScanItemLock[MyCashierNum]);

        // Wait Cashier to scan my items
        EachCashierScanItemCV[MyCashierNum]->Wait(EachCashierScanItemLock[MyCashierNum]);

        if(CustDataArr[CustID]->InsufMoney){
            // Insufficient Money, needs to talk to Manager
        	CustToManagerLock.Acquire();
        	CurInsufCustID = CustID;//MyID
        	printf("Cust %d needs to meet Manager\n", CustID);
        	InsufCustWaitingCV->Signal(&CustToManagerLock);

        	while(CustDataArr[CustID]->InsufMoney){
        		WaitForCheckCV->Wait(&CustToManagerLock);
        		if(CustDataArr[CustID]->InsufMoney){
        			printf("  still insuf, needs to remove one item\n");
        			RemoveOneItem(CustID);
        		}else{
        			printf("  I can finally afford it! Goodbye! \n");
        			WaitForCheckCV->Signal(&CustToManagerLock);
        			CustToManagerLock.Release();
        			break;
        		}
    			WaitForCheckCV->Signal(&CustToManagerLock);
        	}

        }else{
        	printf("The total amount for myself (Customer [%d]) is %.2f\n",
        			CustID, CurCustTotal[MyCashierNum]);
        }

        EachCashierScanItemLock[MyCashierNum]->Release();
        printf("  Cust [%d] DONE!\n", CustID);
        FinishedCust++;
        //printf("FinishedCust == %d",FinishedCust);
    }
    //<< Interacts with Cashier

    //>>>> Cust Sales Cashier Manager

    //>> First test interactions between Cust and Sales
    if(ManagerCustCashierDebugMode && MCC_DebugName==Test_Cust_Sales){
    	printf("Test_Cust_Sales\n");
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
            cout << "Customer [" << CustID
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
        ImCustNumber[mySalesInd] = CustID;
        WhoImTalkingTo[mySalesInd] = 0;// Greeting Customer
        cout << "Customer [" << CustID
             << "] is interacting with DepartmentSalesman [" << mySalesInd
             << "] of Department [1]" << endl;
        SalesmanCV[mySalesInd]->Signal(SalesmanLock[mySalesInd]);
        CustWaitingLock.Release();
        SalesmanCV[mySalesInd]->Wait(SalesmanLock[mySalesInd]);
        cout << "Customer [" << CustID << "] starts shopping" << endl; // Debug purpose
        SalesmanLock[mySalesInd]->Release();
    }
    //<<

}

void RemoveOneItem(int CustID){
	for(int i=0; i<NUM_ITEM; i++){
		int temp = CustDataArr[CustID]->ShoppingList[i];
		if(temp>0){
			CustDataArr[CustID]->ShoppingList[i] = temp-1;
			return;
		}
	}
}
