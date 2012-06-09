/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>

void RunSupermarketSimulation(){
    Thread *t;
    char *name;
    int i;

    for(i=0; i<NUM_CUSTOMER; i++){
    	CustDataArr[i].CustID = i;
    	CustDataArr[i].CashAmount = 14.00; // for test
    	CustDataArr[i].BillAmount = 0.0;
    	CustDataArr[i].CustRole = REGULAR; // for test
    	for(int j=0; j<NUM_ITEM; j++){
    		CustDataArr[i].ShoppingList[j] = CustShoppingLists[i][j];
    	}
    }

    //>> Test Cust_Sales
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==1) )
    {
        printf("Test Cust_Sales\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_SALESMAN = %d\n",NUM_SALESMAN);

        // init customers
        for(i=0; i<NUM_CUSTOMER; i++){
            name = new char[20];
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
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==2) )
    {
        printf("Test Cust_Cashier\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_CASHIER = %d\n",NUM_CASHIER);

        // init customers
        for(i=0; i<NUM_CUSTOMER; i++){
            name = new char[20];
            sprintf(name, "Customer_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Customer,i);
        }

        // init cashiers
		for(i=0; i<NUM_CASHIER; i++){
			name = new char[20];
			sprintf(name, "EachCashierLineCV_%d", i);
			EachCashierLineCV[i] = new Condition(name);
			EachCashierLineLength[i] = 0;
			EachCashierIsBusy[i] = true;
			CustIDforEachCashier[i] = -1;
			CurCustTotal[i] = 0.0;
			name = new char[20];
			sprintf(name, "EachCashierScanItemLock_%d", i);
			EachCashierScanItemLock[i] = new Lock(name);
			name = new char[20];
			sprintf(name, "EachCashierScanItemCV_%d", i);
			EachCashierScanItemCV[i] = new Condition(name);
		}

        for(i=0; i<NUM_CASHIER; i++){
            name = new char[20];
            sprintf(name, "Cashier_%d", i);
            t = new Thread(name);
            t->Fork((VoidFunctionPtr)Cashier,i);
        }
    }
    //<< Test Cust_Cashier
}
