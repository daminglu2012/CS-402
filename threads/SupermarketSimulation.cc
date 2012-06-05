/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>

void RunSupermarketSimulation(){
    //>> Test Cust_Sales
    Thread *t;
    char *name;
    // init customers
    for(int i=0; i<NUM_CUSTOMER; i++){
        name = new char[20];
        sprintf(name, "Customer_%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Customer,i);
    }
    // init salesmen
    for(int i=0; i<NUM_SALESMAN; i++){
        SalesmenStatus[i] = 1; // init busy
    }
    for(int i=0; i<=NUM_SALESMAN; i++){
        char i_char = (char)i;
        t = new Thread(strcat("Salesman_",&i_char));
        t->Fork((VoidFunctionPtr)Salesman,i);
    }
    //<< Test Cust_Sales
}
