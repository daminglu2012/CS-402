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
#define NUM_CASHIER 2
#define NUM_ITEM 10

//>> DEBUG Options
/*
extern bool CustDebugMode;
extern int CustDebugIndex;
*/
static bool CustDebugMode = false;//If Debug is off, then all the code will be executed
enum CustDebugModeName_T{
	Cust_Sales,
	Cust_Cashier,
	Cust_Manager
};
extern CustDebugModeName_T CustDebugModeName;

static bool ManagerCustCashierDebugMode = true;
enum MCC_DebugName_T{
	Manager_Cust_Cashier,
	Manager_Cashier
};
extern MCC_DebugName_T MCC_DebugName;
//<< DEBUG Options

//----------------------------------------------------------------------------------
static int CustShoppingLists[NUM_CUSTOMER][10] = {
    {1, 5, 2, 1, 3, 5, 2, 0, 1, 4},  // Cust 0: 12.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4},  // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5},  // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2},   // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5},  // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2},  // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5}   // Cust 7: 14.00
};
/*
static int CustShoppingLists[NUM_CUSTOMER][10] = {
    {1, 5, 2, 1, 3, 5, 2, 0, 1, 4}, // Cust 0: 12.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4}, // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5}, // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2}, // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1}, // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5}, // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2}, // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5}  // Cust 7: 14.00
};
*/

enum CustRole_T { REGULAR, PRIVILEGE, COMPLAIN, RESTOCK };

struct CustomerData{
	int CustID;
	int ShoppingList[NUM_ITEM];
	float CashAmount, BillAmount;
	CustRole_T CustRole;
	bool InsufMoney;
};

extern CustomerData* CustDataArr[NUM_CUSTOMER]; // *****

//----------------------------------------------------------------------------------

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
extern Lock PrvlCustLineLock;

extern Condition* EachCashierLineCV[NUM_CASHIER]; // ***
extern Condition* EachCashierPrvlLineCV[NUM_CASHIER]; // ***


extern int EachCashierLineLength[NUM_CASHIER];
extern int EachCashierRegLen[NUM_CASHIER];
extern int EachCashierPrvLen[NUM_CASHIER];

extern bool EachCashierIsBusy[NUM_CASHIER];
extern int CustIDforEachCashier[NUM_CASHIER];
extern float CurCustTotal[NUM_CASHIER];
extern Lock* EachCashierScanItemLock[NUM_CASHIER];
extern Condition* EachCashierScanItemCV[NUM_CASHIER];

extern int CustShoppingLists[NUM_CUSTOMER][10];
//<< Variables for Cashier_Cust

//----------------------------------------------------------------------------------

//>> Variables for Manager_Cust_Cashier
extern int FinishedCust;
extern Lock CustToManagerLock;
extern int CurInsufCustID;
extern Condition* InsufCustWaitingCV;
extern Condition* WaitForCheckCV;

extern Lock CashierToManagerLock;
extern int NumWaitingCashier;
extern Condition* CashierWaitingCV;

//<< Variables for Manager_Cust_Cashier

//----------------------------------------------------------------------------------
// below are thread functions for each role
void Salesman(int id);
void Customer(int id);
void Cashier(int id);
void Manager(int id);
//----------------------------------------------------------------------------------
// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
