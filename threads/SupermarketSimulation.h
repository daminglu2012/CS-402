#ifndef SUPERMARKET
#define SUPERMARKET

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <cstring>
#include <cstdio>

// define constants
#define NUM_SALESMAN 3
#define NUM_CUSTOMER 10

//>> Variables for Cust_Sales
extern Lock CustToSalesLineLock;
extern Condition CustWaitingCV, SalesWaitingCV;
extern int CustWaitingLineCount;

//0:not busy,1:busy,2:on break,3:ready
extern int SalesmenStatus[NUM_SALESMAN];
//<< Variables for Cust_Sales

// below are thread functions for each role
void Salesman(int id);
void Customer(int id);

// this is our 'main' entrance
extern void RunSupermarketSimulation();
#endif
