// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

void Yield_Syscall() {
    currentThread->Yield();
}

int CreateLock_Syscall(unsigned int vaddr, int len) {
    char *buf = new char [len + 1];

    // input arg validation
    if (!buf) {
        printf("%s","Can't allocate kernel buffer in CreateLock\n");
        return -1;
    }

    if (copyin(vaddr,len,buf) == -1) {
        printf("%s","Bad pointer passed to CreateLock\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0';

    LockPoolLock.Acquire();
    int nextLockPos = LockPoolBitMap->Find();
    if (nextLockPos == -1) {
      printf("%s", "LockPool is full\n");
      delete[] buf;
      return -1;
    }
    char ind = (char)nextLockPos;
    LockPool[nextLockPos].lock = new Lock(strcat("Lock#", &ind));

    if (!LockPool[nextLockPos].lock) {
      printf("%s", "Create Lock fails in CreateLock");
      delete[] buf;
      return -1;
    }

    LockPool[nextLockPos].space = currentThread->space; // lock is in the currentThread's space
    LockPool[nextLockPos].isDestroyed = false;
    LockPool[nextLockPos].isToBeDestroyed = false;
    LockPool[nextLockPos].threadsCount = 0;

    int rv = nextLockPos;
    LockPoolLock.Release();

    delete[] buf;
    return rv;
}

void DestroyLock_Syscall(int LockId) {
    // Boundary check
    if (LockId<0 || LockId>MAX_LOCKS) {
      printf("%s", "LockId is out of boundary");
      return;
    }

    LockPoolLock.Acquire();
    if (LockPool[LockId].isDestroyed == true) {
      printf("%s", "Lock is NULL, Cannot be Destroyed again\n");
      return;
    }

    // the Lock is not owned by the current process
    if (LockPool[LockId].space != currentThread->space) {
      printf("%s", "Lock is not in the current process\n");
      return;
    }

    // one more case: if the lock is in use, how to 'remember' this destroy request???
    if (LockPool[LockId].threadsCount > 0) {
      printf("%s", "Lock cannot be destroyed right now, it is in use\n");
      LockPool[LockId].isToBeDestroyed = true;
      return;
    }

    delete LockPool[LockId].lock;
    LockPool[LockId].lock = NULL;
    LockPool[LockId].isDestroyed = true;
    LockPoolBitMap->Clear(LockId);

    LockPoolLock.Release();
}

void Acquire_Syscall(int LockId) {
    // Boundary check
    if (LockId<0 || LockId>MAX_LOCKS) {
        printf("%s", "LockId is out of boundary");
        return;
    }

    LockPoolLock.Acquire();
    if (LockPool[LockId].isDestroyed == true) {
        printf("%s", "Lock is NULL, Cannot be Acquire again\n");
        return;
    }

    // the Lock is not owned by the current process
    if (LockPool[LockId].space != currentThread->space) {
        printf("%s", "Lock is not in the current process\n");
        return;
    }

    LockPool[LockId].threadsCount++;
    LockPool[LockId].lock->Acquire();

    LockPoolLock.Release();
}

void Release_Syscall(int LockId) {
    // Boundary check
    if (LockId<0 || LockId>MAX_LOCKS) {
      printf("%s", "LockId is out of boundary");
      return;
    }

    LockPoolLock.Acquire();
    if (LockPool[LockId].isDestroyed == true) {
        printf("%s", "Lock is NULL, Cannot be Release again\n");
        return;
    }

    // the Lock is not owned by the current process
    if (LockPool[LockId].space != currentThread->space) {
        printf("%s", "Lock is not in the current process\n");
        return;
    }

    if (LockPool[LockId].threadsCount > 0) {
        LockPool[LockId].threadsCount--;
        if (LockPool[LockId].threadsCount==0) {
            if(LockPool[LockId].isToBeDestroyed == true) {
                LockPool[LockId].lock->Release();
                delete LockPool[LockId].lock;
                LockPool[LockId].lock = NULL;
                LockPool[LockId].isDestroyed = true;
                LockPool[LockId].isToBeDestroyed = false;
            } else {
                LockPool[LockId].lock->Release();
            }
        } 
    } else {
        printf("%s", "Release the Lock too many times\n");
        return;
    }

    LockPoolLock.Release();
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;

      case SC_Yield:
    DEBUG('a', "Yield syscall.\n");
    Yield_Syscall();
    break;
    
    // Lock syscall
      case SC_CreateLock:
    DEBUG('a', "Create a Lock.\n");
    rv = CreateLock_Syscall(machine->ReadRegister(4),
                       machine->ReadRegister(5));
    break;
      case SC_DestroyLock:
    DEBUG('a', "Destroy a Lock.\n");
    DestroyLock_Syscall(machine->ReadRegister(4));
    break;
      case SC_Acquire:
    DEBUG('a', "Acquire a Lock.\n");
    Acquire_Syscall(machine->ReadRegister(4));
    break;
      case SC_Release:
    DEBUG('a', "Acquire a Lock.\n");
    Release_Syscall(machine->ReadRegister(4));
    break;

    // CV syscall: TO Be Continued...
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
