/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>
CustomerData* CustDataArr[NUM_CUSTOMER];
void RunSupermarketSimulation(){
    Thread *t;
    char *name = new char[50];
    int i;
    //>> Init CUSTOMER
    for(i=0; i<NUM_CUSTOMER; i++){
		CustomerData* CurNew = (CustomerData*)malloc(sizeof(CustomerData));
		CurNew->CustID = i;
		CurNew->BillAmount = 0.0;
		CurNew->CashAmount = 14.00;
		if(i==0){CurNew->CustRole = 0;}
		else{CurNew->CustRole = 1;}
		for(int j=0; j<NUM_ITEM; j++){
			CurNew->ShoppingList[j] = CustShoppingLists[i][j];
		}
		CustDataArr[i] = CurNew;
    }
    //<< Init CUSTOMER
    /*
    for(i=0; i<NUM_CUSTOMER; i++){
    	printf("\nCust [%d]\n",i);
    	printf("ID == [%d], ", CustDataArr[i].CustID);
    	printf("CashAmount == [%.2f], ", CustDataArr[i].CashAmount);
    	printf("BillAmount == [%.2f], ", CustDataArr[i].BillAmount);
    	printf("CustRole == [%d], ", CustDataArr[i].CustRole);
    	printf("\n Shopping List: ");
    	for(int j=0; j<NUM_ITEM; j++){
    		printf("[%d] ", CustDataArr[i].ShoppingList[j]);
    	}
    	printf("\n");
    }
    */
    //>> Test Cust_Sales
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==1) )
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
    if( !CustDebugMode|| (CustDebugMode && CustDebugIndex==2) )
    {
        printf("Test Cust_Cashier\n\n");
        printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
        printf("NUM_CASHIER = %d\n",NUM_CASHIER);

        // init cashiers
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

        // init customers
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
}
