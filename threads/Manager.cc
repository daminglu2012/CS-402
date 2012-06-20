

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

Lock TotalAmountLock("TotalAmountLock");
float TotalAmount = 0.0;
float PrevTotal = TotalAmount; // local

void CalBill(int CustID);
void Manager(int ManagerID){
	while(true){
		FinishedCustLock.Acquire();
		printf("Manager starts a work cycle, Finished Cust==[%d]\n",
				FinishedCust);

		if(PrevTotal!=TotalAmount){
			printf("Manager reports current total is [%.2f], after [%d] Customers finished shopping\n",
					TotalAmount, FinishedCust);
			PrevTotal = TotalAmount;
		}


		if(FinishedCust>=NUM_CUSTOMER){
			printf("\n ! MANAGER: All customers finished shopping, let's call it a day :)\n");
			printf("\n ! MANAGER: We've sold a total of [%.2f]\n\n",TotalAmount);
			return;
		}
		FinishedCustLock.Release();

		//printf("Manager One Iter\n");
		CashierToManagerLock.Acquire();
		if(NumWaitingCashier>0){
			/*
			//printf("Manager finds there is waiting cashier\n");
			//>>
			InsufCustCount++;
			if(InsufCustCount%2==1){
				//  odd : break Cashier[1]
	        	CashierOnBreakLock.Acquire();
				printf("\tManager sets Cashier 1 on break\n");
				CashierIsOnBreak[1] = true;
				CashierIsOnBreak[2] = false;
	        	CashierOnBreakCV.Signal(&CashierOnBreakLock);
	        	CashierOnBreakLock.Release();
			}else{
				// even : break Cashier[2]
	        	CashierOnBreakLock.Acquire();
				printf("\tManager sets Cashier 2 on break\n");
	        	CashierIsOnBreak[1] = false;
	        	CashierIsOnBreak[2] = true;
	        	CashierOnBreakCV.Signal(&CashierOnBreakLock);
	        	CashierOnBreakLock.Release();
			}
			//<<
			*/
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
			TotalAmountLock.Acquire();
			TotalAmount += CustDataArr[CurInsufCustID]->BillAmount;
			TotalAmountLock.Release();
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

