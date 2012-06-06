#ifndef SUPERMARKET
#define SUPERMARKET

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cstring>
#include <cstdio>

// define constants
#define NUM_SALESMAN 3
#define NUM_CUSTOMER 5
#define NUM_ITEMS	 5

//>> Variables for Cust_Sales
extern Lock CustToSalesLineLock;
extern Condition SalesWaitingCV;

extern Lock CustWaitingLock;
extern Condition CustWaitingCV;
extern Lock *SalesmanLock[NUM_SALESMAN];
extern Condition *SalesmanCV[NUM_SALESMAN];

extern Lock *GoodsLock[NUM_ITEMS];
extern Condition *GoodsNotEnoughCV[NUM_ITEMS];

extern int CustWaitingLineCount;

//0:not busy,1:busy,2:on break,3:ready
extern int SalesmenStatus[NUM_SALESMAN];
//<< Variables for Cust_Sales

extern int WhoImTalkingTo[NUM_SALESMAN];
extern int ImCustNumber[NUM_SALESMAN];
extern int GoodsOnDemand[NUM_SALESMAN];
extern int TotalItems[NUM_ITEMS];
extern float ItemPrices[NUM_ITEMS];

// below are thread functions for each role
void Salesman(int id);
void Customer(int id);

void CustomerShopping(int);

// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
