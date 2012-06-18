

#include "SupermarketSimulation.h"

MCC_DebugName_T MCC_DebugName = Manager_Cust_Cashier;

// Cust
Lock CustToManagerLock("CustToManagerLock");
int CurInsufCustID;
Condition* InsufCustWaitingCV;

// Cashier
Lock CashierToManagerLock("CashierToManagerLock");
int NumWaitingCashier;
Condition* CashierWaitingCV;
Condition* WaitForCheckCV;//Cust

int InsufCustCount = 0;
int NumCashierOnBreak = 0;

void CalBill(int CustID);

void Manager(int ManagerID){
	while(true){
		if(FinishedCust>=NUM_CUSTOMER){
			printf("\n ! MANAGER: All customers finished shopping, let's call it a day :)\n\n");
			break;
		}
		//printf("Manager One Iter\n");
		CashierToManagerLock.Acquire();
		if(NumWaitingCashier>0){
			//printf("Manager finds there is waiting cashier\n");
			//>>
			InsufCustCount++;
			if(InsufCustCount%2==1){
				// odd : break Cashier[1]
				printf("\tManager sets Cashier 1 on break\n");
	        	CashierOnBreakLock.Acquire();
				CashierIsOnBreak[1] = true;
				CashierIsOnBreak[2] = false;
	        	CashierOnBreakLock.Release();
			}else{
				// even : break Cashier[2]
				printf("\tManager sets Cashier 2 on break\n");
	        	CashierOnBreakLock.Acquire();
	        	CashierOnBreakCV.Signal(&CashierOnBreakLock);
	        	CashierIsOnBreak[1] = false;
	        	CashierIsOnBreak[2] = true;
	        	CashierOnBreakLock.Release();
			}
			//<<
			CashierWaitingCV->Signal(&CashierToManagerLock);
			NumWaitingCashier--;
			CustToManagerLock.Acquire();
			CashierToManagerLock.Release();
			printf("Manager waits insuf cust coming\n");
			InsufCustWaitingCV->Wait(&CustToManagerLock);
			printf("Manager met insuf Cust [%d]\n", CurInsufCustID);
			while(CustDataArr[CurInsufCustID]->InsufMoney){
				CalBill(CurInsufCustID);//will also reset InsufMoney if needed!
				printf("  calc'ed new bill [%.2f], signal cust\n",
						CustDataArr[CurInsufCustID]->BillAmount);
				WaitForCheckCV->Signal(&CustToManagerLock);
				WaitForCheckCV->Wait(&CustToManagerLock);
			}
			printf("  Cust [%d] can afford the bill, done!\n ", CurInsufCustID);

			CustToManagerLock.Release();
		}else{
			CashierToManagerLock.Release();
		}
	}
}

void CalBill(int CustID){
	float NewBill = 0.0;
	for(int i=0; i<NUM_ITEM; i++){
		NewBill += GoodsPrices[i] *
		CustDataArr[CustID]->ShoppingList[i];
	}
	CustDataArr[CustID]->BillAmount = NewBill;
	if(NewBill > CustDataArr[CustID]->CashAmount){
		CustDataArr[CustID]->InsufMoney = true;
	}else{
		CustDataArr[CustID]->InsufMoney = false;
	}
}

