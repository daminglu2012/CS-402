#ifndef SUPERMARKET
#define SUPERMARKET

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cstring>
#include <cstdio>

// define constants
#define NUM_SALESMAN 	3
#define NUM_CUSTOMER 	20
#define NUM_GOODSLOADER 3
#define NUM_ITEMS	 	5
#define MAX_ITEM		3

//>> Variables for Cust_Sales
extern Lock CustToSalesLineLock;
extern Condition SalesWaitingCV;

extern Lock CustWaitingLock;
extern Condition CustWaitingCV;
extern Lock *SalesmanLock[NUM_SALESMAN];
extern Condition *SalesmanCV[NUM_SALESMAN];

extern Lock *GoodsLock[NUM_ITEMS];
extern Lock *GoodsLoadingLock[NUM_ITEMS];
extern Lock *TotalItemsLock[NUM_ITEMS];
extern Condition *GoodsNotEnoughCV[NUM_ITEMS];

extern Lock FreeGoodsLoaderLock;
extern Condition FreeGoodsLoaderCV;
extern Lock *GoodsLoaderLock[NUM_GOODSLOADER];
extern Condition *GoodsLoaderCV[NUM_GOODSLOADER];

extern Lock StockRoomLock;
extern Condition StockRoomCV;

extern int CustWaitingLineCount;
extern int SalesmanWaitingLineCount;
extern int StockRoomWaitingLineCount;

//0:not busy,1:busy,2:on break,3:ready
extern int SalesmenStatus[NUM_SALESMAN];
extern int GoodsLoaderStatus[NUM_GOODSLOADER]; // 0: free, 1: busy

//<< Variables for Cust_Sales

extern int WhoIsInTheStockRoom;

extern int WhoImTalkingTo[NUM_SALESMAN];
extern int ImCustNumber[NUM_SALESMAN];
extern int ImGoodsLoaderNumer[NUM_SALESMAN];
extern int GoodsLoaderLoadingGoodsNumber[NUM_SALESMAN];
extern int ImSalesmanNumber[NUM_GOODSLOADER];
extern int GoodsOnDemand[NUM_SALESMAN];
extern int isBeingLoaded[NUM_ITEMS];
extern int TotalItems[NUM_ITEMS];
extern float ItemPrices[NUM_ITEMS];

extern bool GoodsIsFull[NUM_ITEMS];

// below are thread functions for each role
void Salesman(int id);
void Customer(int id);
void GoodsLoader(int);
void CustomerShopping(int);
void SalesmanShopping(int);
// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
