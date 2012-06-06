#ifndef SUPERMARKET
#define SUPERMARKET

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cstring>
#include <cstdio>
#include <cassert>

// define constants
#define NUM_SALESMAN 3
#define NUM_CUSTOMER 8 // for test
#define NUM_CASHIER 5
#define NUM_ITEM 10

//>> DEBUG Options
extern bool CustDebugMode;
extern int CustDebugIndex;
//<< DEBUG Options

//>> Variables for Cust_Sales
extern Lock CustToSalesLineLock;
extern Condition CustWaitingCV, SalesWaitingCV;
extern int CustWaitingLineCount;

//0:not busy,1:busy,2:on break,3:ready
extern int SalesmenStatus[NUM_SALESMAN];
//<< Variables for Cust_Sales
//----------------------------------------------------------------------------------
//>> Variables for Cashier_Cust
extern int FindShortestCashierLine(int arr[], int arrLength);

extern float GoodsPrices[10]; // price for those 10 items

extern Lock CustToCashierLineLock;
extern Condition* EachCashierLineCV[NUM_CASHIER]; // ***
extern int EachCashierLineLength[NUM_CASHIER];
extern bool EachCashierIsBusy[NUM_CASHIER];
extern int CustIDforEachCashier[NUM_CASHIER];
extern float CurCustTotal[NUM_CASHIER];
extern Lock* EachCashierScanItemLock[NUM_CASHIER];
extern Condition* EachCashierScanItemCV[NUM_CASHIER];

extern int CustShoppingLists[NUM_CUSTOMER][10];

//<< Variables for Cashier_Cust
//----------------------------------------------------------------------------------
// below are thread functions for each role
void Salesman(int id);
void Customer(int id);
void Cashier(int id);
//----------------------------------------------------------------------------------
// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
