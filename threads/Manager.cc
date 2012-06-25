

#include "SupermarketSimulation.h"

MCC_DebugName_T MCC_DebugName = Test_Everything;

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

Condition ManagerWaitCashierWakeUp("ManagerWaitCashierWakeUp");
Condition ManagerWaitCashierSleep("ManagerWaitCashierSleep");

Lock *TotalItemsLock[NUM_ITEM];

void CalBill(int CustID);

void Manager(int ManagerID){
	NumWaitingCashier=0;
	InsufCustCount = 0;
	while(true){
		FinishedCustLock.Acquire();
		//printf("Manager starts a work cycle, Finished Cust==[%d]\n",
		//		FinishedCust);

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
			InsufCustCount++; // add one processed InsufCust
			// %3==1: Cashier 1 on break
			// %3==2: Cashier 2 on break
			// %3==0: Wake up all on break Cashiers
			/*
			CashierOnBreakLock.Acquire();
			switch(InsufCustCount%3){
			case 1:
				printf("case 1\n");
				CashierIsOnBreak[1] = true;
				//ManagerWaitCashierSleep.Wait(&CashierOnBreakLock);
				printf("case 1 DONE\n");
				break;
			case 2:
				printf("case 2\n");
				CashierIsOnBreak[2] = true;
				//ManagerWaitCashierSleep.Wait(&CashierOnBreakLock);
				printf("case 2 DONE\n");
				break;
			case 0:
				printf("case 0\n");
				//CashierOnBreakCV.Broadcast(&CashierOnBreakLock);
				//ManagerWaitCashierWakeUp.Wait(&CashierOnBreakLock);
				printf("case 0 DONE\n");
				break;
			}
			CashierOnBreakLock.Release();
			*/

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
				if(NumCashierOnBreak>0){
					NumCashierOnBreak--;
					CashierOnBreakCV.Signal(&CashierOnBreakLock);
					ManagerWaitCashierWakeUp.Wait(&CashierOnBreakLock);
				}
	        	CashierOnBreakLock.Release();
			}else{
				// even : break Cashier[2]
	        	CashierOnBreakLock.Acquire();
				printf("\tManager sets Cashier 2 on break\n");
	        	CashierIsOnBreak[1] = false;
	        	CashierIsOnBreak[2] = true;
				if(NumCashierOnBreak>0){
					NumCashierOnBreak--;
					CashierOnBreakCV.Signal(&CashierOnBreakLock);
					ManagerWaitCashierWakeUp.Wait(&CashierOnBreakLock);
				}
				CashierOnBreakLock.Release();
			}
			//<<
			*/
			CustToManagerLock.Acquire();
			CashierWaitingCV->Signal(&CashierToManagerLock);
			NumWaitingCashier--;// OK, got u, let the cashier go
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

