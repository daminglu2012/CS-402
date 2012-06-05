/*
This is our 'main' function
*/

#include "SupermarketSimulation.h"
#include <cstdio>

void RunSupermarketSimulation(){
    //>> Test Cust_Sales
    printf("NUM_CUSTOMER = %d\n",NUM_CUSTOMER);
    printf("NUM_SALESMAN = %d\n",NUM_SALESMAN);

    Thread *t;
    char *name;
    int i;
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
    //<< Test Cust_Sales
}
