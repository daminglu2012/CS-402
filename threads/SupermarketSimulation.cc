/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>

void RunSupermarketSimulation(){
    // //>> Test Cust_Sales
    // Thread *t;
    // char *name;
    // // init customers
    // for(int i=0; i<NUM_CUSTOMER; i++){
    //     name = new char[20];
    //     sprintf(name, "Customer_%d", i);
    //     t = new Thread(name);
    //     t->Fork((VoidFunctionPtr)Customer,i);
    // }
    // // init salesmen
    // for(int i=0; i<NUM_SALESMAN; i++){
    //     SalesmenStatus[i] = 1; // init busy
    // }
    // for(int i=0; i<=NUM_SALESMAN; i++){
    //     char i_char = (char)i;
    //     t = new Thread(strcat("Salesman_",&i_char));
    //     t->Fork((VoidFunctionPtr)Salesman,i);
    // }
    // //<< Test Cust_Sales

    // // Test Customer Shopping
    // for (int i = 0; i < NUM_SALESMAN; i++) {
    //     char index = (char)i;
    //     Thread *t = new Thread(strcat("Customer ", &index));
    //     t->Fork((VoidFunctionPtr)Customer, i);
    // }

    for (int i = 0; i < NUM_ITEMS; i++) {
        char index = (char)i;
        GoodsLock[i] = new Lock(strcat("GoodsLock#", &index));
        GoodsNotEnoughCV[i] = new Condition(strcat("GoodsCV#", &index));
    }

    for (int i = 0; i < NUM_SALESMAN; i++) {
        char index = (char)i;
        SalesmanLock[i] = new Lock(strcat("SalesmanLock#", &index));
        SalesmanCV[i] = new Condition(strcat("SalesmanCV#", &index));
    }

    for (int i = 0; i < NUM_GOODSLOADER; i++) {
        char index = (char)i;
        GoodsLoaderLock[i] = new Lock(strcat("GoodsLoaderLock#", &index));
        GoodsLoaderCV[i] = new Condition(strcat("GoodsLoaderCV#", &index));
    }
    for (int i = 0; i < NUM_CUSTOMER; i++) {
        char index = (char)i;
        Thread *t = new Thread(strcat("Customer#", &index));
        t->Fork((VoidFunctionPtr)CustomerShopping, i);
    }
    
    for (int i = 0; i < NUM_SALESMAN; i++) {
        char index = (char)i;
        Thread *t = new Thread(strcat("Salesman#", &index));
        t->Fork((VoidFunctionPtr)SalesmanShopping, i);        
    }

    for (int i = 0; i < NUM_GOODSLOADER; i++) {
        char index = (char)i;
        Thread *t = new Thread(strcat("GoodsLoader#", &index));
        t->Fork((VoidFunctionPtr)GoodsLoader, i);                
    }

    // for (int i = 0; i < NUM_ITEMS; i++) {
    //     delete ShelfWaitingLock[i];
    // }
}
