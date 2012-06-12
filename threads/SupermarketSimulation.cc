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

    //>> Init Manager
    InsufCustWaitingCV = new Condition("InsufCustWaitingCV");
    WaitForCheckCV = new Condition("WaitForCheckCV");
    NumWaitingCashier = 0;
    CashierWaitingCV = new Condition("CashierWaitingCV");
    //<< Init Manager

    //>> Init Cashier
	for(i=0; i<NUM_CASHIER; i++){
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

    // <<<< Initialization <<<< Initialization <<<< Initialization

    //------------------------------------------------------------------

    // >>>> Testing >>>> Testing >>>> Testing >>>> Testing >>>> Testing

    //>> Test Cust_Sales
    if(CustDebugMode && CustDebugModeName==Cust_Sales)
    {
        printf("Test Cust_Sales\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_SALESMAN = %d\n",NUM_SALESMAN);

        // init customers
        for(i=0; i<NUM_CUSTOMER; i++){
            sprintf(name, "Customer_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,i);
        }
        // init salesmen
        for(i=0; i<NUM_SALESMAN; i++){
            SalesmenStatus[i] = 1; // init busy
        }
        for(i=0; i<NUM_SALESMAN; i++){
            char i_char = (char)i;
            t = new Thread(strcat("Salesman_",&i_char));
            t->Fork((VoidFunctionPtr)Salesman,i);
        }
    }
    //<< Test Cust_Sales

    //>> Test Cust_Cashier
    if(CustDebugMode && CustDebugModeName==Cust_Cashier)
    {
        printf("Test Cust_Cashier\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_CASHIER = %d\n",NUM_CASHIER);

        for(i=0; i<NUM_CUSTOMER; i++){
            sprintf(name, "Customer_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,i);
        }

        for(i=0; i<NUM_CASHIER; i++){
            sprintf(name, "Cashier_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Cashier,i);
        }
    }
    //<< Test Cust_Cashier
    if(ManagerCustCashierDebugMode && MCC_DebugName==Manager_Cust_Cashier)
    {
        printf("Test Manager_Cust_Cashier\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_CASHIER = %d\n",NUM_CASHIER);
        printf("Don't forget the Manager :) \n\n");

        for(i=0; i<NUM_CUSTOMER; i++){
            sprintf(name, "Customer_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,i);
        }

        for(i=0; i<NUM_CASHIER; i++){
            sprintf(name, "Cashier_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Cashier,i);
        }

        sprintf(name, "Manager");
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Manager, 0);

    }

    // <<<< Testing <<<< Testing <<<< Testing <<<< Testing <<<< Testing

}
