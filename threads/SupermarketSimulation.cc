/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>

CustomerData* CustDataArr[NUM_CUSTOMER];

int FinishedCust;

void RunSupermarketSimulation(){
	FinishedCust = 0;
    Thread *t;
    char *name = new char[50];
    int i;

    // >>>> Initialization >>>> Initialization >>>> Initialization

    //>> Init CUSTOMER
    for(i=0; i<NUM_CUSTOMER; i++){
		CustomerData* CurNew = (CustomerData*)malloc(sizeof(CustomerData));
		CurNew->CustID = i;
		CurNew->BillAmount = 0.0;
		CurNew->CashAmount = 13.00;
		CurNew->InsufMoney = false;

		// Test Privilege
		if(i%3==0){CurNew->CustRole = PRIVILEGE ;}
		else{CurNew->CustRole = REGULAR;}
		//CurNew->CustRole = REGULAR;
		for(int j=0; j<NUM_ITEM; j++){
			CurNew->ShoppingList[j] = CustShoppingLists[i][j];
		}
		CustDataArr[i] = CurNew;
    }
    //<< Init CUSTOMER

    //>> Init SALESMAN
    for(i=0; i<NUM_SALESMAN; i++){
		char index = (char)i;
		SalesmanLock[i] = new Lock(strcat("SalesmanLock#", &index));
		SalesmanCV[i] = new Condition(strcat("SalesmanCV#", &index));

		ImCustNumber[i] = 0;
		WhoImTalkingTo[i] = 0;
		SalesmenStatus[i] = 1;

		GoodsOnDemand[i]=-1;//???
		ImGoodsLoaderNumber[i]=-1;
    }
    //<< Init SALESMAN

    //>> Init Manager
    InsufCustWaitingCV = new Condition("InsufCustWaitingCV");
    WaitForCheckCV = new Condition("WaitForCheckCV");
    NumWaitingCashier = 0;
    CashierWaitingCV = new Condition("CashierWaitingCV");
    //<< Init Manager

    //>> Init Cashier
	for(i=0; i<NUM_CASHIER; i++){
		/*
		if(i==0){
			CashierIsOnBreak[i] = false;
		}else{
			CashierIsOnBreak[i] = true;
		}
		*/
		CashierIsOnBreak[i] = false; // no break

		sprintf(name, "EachCashierLineCV_%d", i);
		EachCashierLineCV[i] = new Condition(name);

		sprintf(name, "EachCashierPrvlLineCV_%d", i);
		EachCashierPrvlLineCV[i] = new Condition(name);

		EachCashierLineLength[i] = 0;
		EachCashierRegLen[i] = 0;
		EachCashierPrvLen[i] = 0;

		EachCashierIsBusy[i] = true;
		CustIDforEachCashier[i] = -1;
		CurCustTotal[i] = 0.0;

		sprintf(name, "EachCashierScanItemLock_%d", i);
		EachCashierScanItemLock[i] = new Lock(name);

		sprintf(name, "EachCashierScanItemCV_%d", i);
		EachCashierScanItemCV[i] = new Condition(name);
	}
    //<< Init Cashier

	//>> Init Goods Loader
	for(i=0; i<NUM_GOODSLOADER; i++){
		GoodsLoaderStatus[i] = 1; // busy
		char index = (char)i;
		GoodsLoaderLock[i] = new Lock(strcat("GoodsLoaderLock#", &index));
		GoodsLoaderCV[i] = new Condition(strcat("GoodsLoaderCV#", &index));
		ImSalesmanNumber[i] = -1;
	}
	//<< Init Goods Loader

	//>> Init Item
	for (i = 0; i < NUM_ITEM; i++) {
		char index = (char)i;
		GoodsLock[i] = new Lock(strcat("GoodsLock#", &index));
		GoodsLoadingLock[i] = new Lock(strcat("GoodsLoadingLock", &index));
		TotalItemsLock[i] = new Lock(strcat("TotalItemsLock", &index));
		GoodsNotEnoughCV[i] = new Condition(strcat("GoodsCV#", &index));
	}
	//<< Init Item

    // <<<< Initialization <<<< Initialization <<<< Initialization

    //--------------------------------------------------------------------------------

    // >>>> Testing >>>> Testing >>>> Testing >>>> Testing >>>> Testing

    //>> Test_Everything
    if(ManagerCustCashierDebugMode && MCC_DebugName==Test_Everything)
    {
        printf("Test_Everything\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_CASHIER = %d\n",NUM_CASHIER);
        printf("NUM_SALESMAN = %d\n",NUM_SALESMAN);
        printf("NUM_GOODSLOADER = %d\n",NUM_GOODSLOADER);

        printf("Don't forget the Manager :) \n\n");

        for(i=0; i<NUM_SALESMAN; i++){
            sprintf(name, "Salesman_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Salesman,i);
        }

        for(i=0; i<NUM_GOODSLOADER; i++){
            sprintf(name, "GoodsLoader_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)GoodsLoader,i);
        }

        for(i=0; i<NUM_CASHIER; i++){
            sprintf(name, "Cashier_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Cashier,i);
        }

        sprintf(name, "Manager");
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Manager, 0);

        for(i=0; i<NUM_CUSTOMER; i++){
            sprintf(name, "Customer_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,i);
        }

    }
    //<< Test_Everything

    // <<<< Testing <<<< Testing <<<< Testing <<<< Testing <<<< Testing

}
