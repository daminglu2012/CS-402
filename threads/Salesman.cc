/*
Salesman interacts with:
1) Customer (regular, complain, restock ?)
2) Goods Loader
3) Manager
*/

#include "SupermarketSimulation.h"

Lock CustWaitingLock("CustToSalesLineLock");//Cust to Sales
Condition CustWaitingCV("CustWaitingCV");
Condition SalesWaitingCV("SalesWaitingCV");
int CustWaitingLineCount = 0;

//0:not busy,1:busy,2:on break,3:ready
int SalesmenStatus[NUM_SALESMAN];
//<< Interact with Customer

Lock *SalesmanLock[NUM_SALESMAN];
Condition *SalesmanCV[NUM_SALESMAN];
int ImCustNumber[NUM_SALESMAN];
int WhoImTalkingTo[NUM_SALESMAN];

void Salesman(int ind) {
    bool allBusy = true;
    while (true) {
        CustWaitingLock.Acquire();
        if (CustWaitingLineCount > 0) {
            CustWaitingCV.Signal(&CustWaitingLock);
            CustWaitingLineCount--;
            SalesmenStatus[ind] = 3; // Ready
        } else {
            SalesmenStatus[ind] = 0;
        }

        SalesmanLock[ind]->Acquire();
        CustWaitingLock.Release();
        printf("Salesman [%d] waits for Customer [X] to come\n", ind );
        SalesmanCV[ind]->Wait(SalesmanLock[ind]); // Wait for customer to come to me

        switch (WhoImTalkingTo[ind]) {
            case 0: // I am Talking to Greeting Customer
                cout << "DepartmentSalesman [" << ind
                     << "] welcomes Customer ["
                     << ImCustNumber[ind]
                     << "] to Department [1]" << endl;
                SalesmanCV[ind]->Signal(SalesmanLock[ind]);
                SalesmanLock[ind]->Release();
                break;
        }
    }
}

////>> Interact with Customer
//Lock CustToSalesLineLock("CustToSalesLineLock");
//Condition CustWaitingCV("CustWaitingCV"), SalesWaitingCV("SalesWaitingCV");
//int CustWaitingLineCount = 0;
//
////0:not busy,1:busy,2:on break,3:ready
//int SalesmenStatus[NUM_SALESMAN];
////<< Interact with Customer
//
//void Salesman(int SalesIndex){
//    while(true){
//        //>> Interact with Customer
//        CustToSalesLineLock.Acquire();
//        if(CustWaitingLineCount>0){
//            CustWaitingCV.Signal(&CustToSalesLineLock);
//            CustWaitingLineCount--;
//            printf("S: Salesman [%d] is Ready ",SalesIndex);
//            SalesmenStatus[SalesIndex] = 3; // This Salesman is Ready
//            // printf("Salesman %d is taking care a cust, ", SalesIndex);
//            // this should be in Customer
//            printf(", now the # of cust waiting is %d\n", CustWaitingLineCount);
//        }else{
//            SalesmenStatus[SalesIndex] = 0;
//        }
//        SalesWaitingCV.Wait(&CustToSalesLineLock);
//        SalesmenStatus[SalesIndex] = 3;
//        CustToSalesLineLock.Release();
//        //<< Interact with Customer
//    }
//}
