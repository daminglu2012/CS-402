#ifndef SUPERMARKET
#define SUPERMARKET

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cstring>
#include <cstdio>
#include <cassert>
#include <iostream>

using namespace std;

// define constants
#define NUM_SALESMAN 3
#define NUM_CUSTOMER 32 // for test
#define NUM_CASHIER 3
#define NUM_ITEM 10
#define MAX_ITEM 4
#define NUM_GOODSLOADER 4

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
	Manager_Cust_Cashier_Salesman,
	Manager_Cust_Cashier_Salesman_GoodsLoader,
	Test_Cust_Sales
};
extern MCC_DebugName_T MCC_DebugName;
//<< DEBUG Options

//----------------------------------------------------------------------------------
extern Lock CustWaitingLock;

extern Lock TotalAmountLock;
extern float TotalAmount;

//----------------------------------------------------------------------------------
static int CustShoppingLists[NUM_CUSTOMER][NUM_ITEM] = {
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 0: 13.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4},  // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5},  // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2},  // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5},  // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2},  // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5},  // Cust 7: 14.00
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 0: 13.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4},  // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5},  // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2},  // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5},  // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2},  // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5},   // Cust 7: 14.00
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 0: 13.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4},  // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5},  // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2},  // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5},  // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2},  // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5},  // Cust 7: 14.00
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 0: 13.00
    {1, 5, 5, 1, 5, 5, 0, 3, 0, 4},  // Cust 1: 14.50
    {2, 3, 2, 4, 4, 0, 1, 0, 4, 5},  // Cust 2: 12.50
    {2, 3, 5, 2, 4, 2, 5, 5, 1, 2},  // Cust 3: 15.50
    {1, 1, 5, 3, 2, 2, 4, 4, 3, 1},  // Cust 4: 13.00
    {3, 0, 0, 0, 3, 2, 5, 4, 5, 5},  // Cust 5: 13.50
    {4, 3, 1, 4, 3, 2, 4, 1, 1, 2},  // Cust 6: 12.50
    {3, 2, 4, 4, 3, 2, 4, 0, 1, 5}   // Cust 7: 14.00
};
extern int TotalItems[NUM_ITEM];

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
//>> Goods(ITEM)
extern int GoodsOnDemand[NUM_SALESMAN];
extern Condition* GoodsNotEnoughCV[NUM_ITEM];
extern Lock* GoodsLock[NUM_ITEM];
extern Lock FreeGoodsLoaderLock;
extern int GoodsLoaderStatus[NUM_GOODSLOADER];
extern int SalesmanWaitingLineCount;
extern Condition FreeGoodsLoaderCV;
extern Lock *GoodsLoaderLock[NUM_GOODSLOADER];
extern Condition *GoodsLoaderCV[NUM_GOODSLOADER];
extern int ImSalesmanNumber[NUM_GOODSLOADER];
extern int ImGoodsLoaderNumber[NUM_SALESMAN];

//<< Goods(ITEM)
//----------------------------------------------------------------------------------

//>> Variables for Cust_Sales
extern Lock CustWaitingLock;//Cust to Sales
extern Condition CustWaitingCV, SalesWaitingCV;
extern int CustWaitingLineCount;

//0:not busy,1:busy,2:on break,3:ready
extern int SalesmenStatus[NUM_SALESMAN];
//<< Variables for Cust_Sales

extern Lock *SalesmanLock[NUM_SALESMAN];
extern Condition *SalesmanCV[NUM_SALESMAN];
extern int ImCustNumber[NUM_SALESMAN];
extern int WhoImTalkingTo[NUM_SALESMAN];

extern bool CashierIsOnBreak[NUM_CASHIER];
extern Lock CashierOnBreakLock;
extern Condition CashierOnBreakCV;
extern Condition ManagerWaitCashierWakeUp;
extern Condition ManagerWaitCashierSleep;

extern int NumCashierOnBreak;
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
extern Lock FinishedCustLock;
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
void GoodsLoader(int id);
//----------------------------------------------------------------------------------
// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
