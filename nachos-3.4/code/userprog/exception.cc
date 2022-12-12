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

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Tang Program Counter
void incProgCounter() {
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void handleRuntimeError(const char msg[]) {
    DEBUG('a', (char *)msg);
    printf("%s", msg);
    interrupt->Halt();
}

// ham luu tru ki tu thua.
void extraCharacterRecycle() {
    char *recycle = new char[1001];
    synchconsole->Read(recycle, 1000);
    delete[] recycle;
}

// Ham doc 1 ki tu.
char ReadChar() {
    char c;
    int numBytesRead = synchconsole->Read(&c, 1);  // doc ki tu.
    // Kiem tra nguoi nhap co huy qua trinh nhap.
    if (numBytesRead < 0) {
        printf("\nStop.\n");
    }
    // Luu ki tu thua.
    extraCharacterRecycle();
    return c;
}

// Ham doc mot so.
int ReadInt() {
    char *str = new char[101];

    int temp = synchconsole->Read(str, 100);  // doc chuoi str chua cac ki tu bieu dien so.
    // Kiem tra nguoi dung co huy qua trinh.
    if (temp < 0) {
        printf("Stop");
        delete[] str;
        return 0;
    }
    // Kiem tra so am hoac duong
    int flag = 0;
    if (str[0] == '-')
        flag = 1;
    int result = 0;
    // Dem so luong ki so
    int count = 0;
    for (int i = flag; i < 100; i++)
        if (str[i] <= '9' && str[i] >= '0')
            count++;
    // chuyen tu chuoi ki so sang so
    for (int i = flag; i < count + flag; i++)
        if (str[i] <= '9' && str[i] >= '0') {
            int pow = 1;
            for (int j = i; j < count + flag - 1; j++)
                pow = pow * 10;
            result += (str[i] - '0') * pow;
        }
    // Doi dau ket qua neu so am
    if (flag) {
        result = -result;
    }
    delete[] str;
    return result;
}

// Ham in ra ki tu
// input: character(char)
void PrintChar(char character) {
    // kiem tra co phai ki tu ma ASCII
    if (character < 0 || character > 255) {
        printf("\n Not ASCII code: %d\n", character);
        return;
    }
    // in ra
    synchconsole->Write(&character, 1);
    // synchconsole->Write("\n", 1);
}

// Ham in so ra man hinh
// input: number
void PrintInt(int number) {
    int flag = 0, temp, temp1;
    temp = number;
    // Kiem tra so am hoac duong
    if (number < 0) {
        flag = 1;
        temp = -1 * number;
    }
    temp1 = temp;
    // printf("%d\n",temp1);
    // Dem so luong ki chu so
    int count = 0;
    while (temp > 0) {
        count++;
        temp /= 10;
    }
    if (number == 0)
        count = 1;
    // phan tach so va dua vao chuoi
    if (count > 0) {
        char *str = new char[count + flag + 1];
        if (flag)
            str[0] = '-';
        for (int i = flag; i < count + flag; i++) {
            int pow = 1;
            for (int j = i; j < count + flag - 1; j++)
                pow *= 10;
            str[i] = temp1 / pow + '0';
            temp1 = temp1 - (temp1 / pow) * pow;
        }
        // in ra man hinh
        synchconsole->Write(str, count + flag);
        delete[] str;
    }
}

// Ham copy tu user sang system memory space tai vi tri virtAddr voi kich thuoc limit
char *User2System(int virtAddr, int limit) {
    int i;
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1];
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    for (i = 0; i < limit; i++) {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Ham copy buffer tu system sang user memory space
int System2User(int virtAddr, int len, char *buffer) {
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

// Ham doc chuoi.
// input: chuoi buffer(char*), kich thuoc chuoi length.
void ReadString(char *buffer, int length) {
    int addrBuf = machine->ReadRegister(4);  // doc dia chi cua buffer.
    buffer = new char[length + 1];
    int temp = synchconsole->Read(buffer, length);  // doc chuoi vao.
    // kiem tra nguoi nhap co huy qua trinh nhap khong.
    if (temp < 0) {
        printf("stop\n");
        delete[] buffer;
        return;
    }
    System2User(addrBuf, length, buffer);  // copy buffer tu system sang user memory space.
    extraCharacterRecycle();               // Luu tru ki tu thua neu so luong ki tu nhap vuot qua length.
}

void PrintString(char *buffer) {
    int addrBuf = machine->ReadRegister(4);  // doc dia chi buffer.
    buffer = User2System(addrBuf, 1000);     // copy buffer tu user sang system memory space.
    int lenBuf = 0;
    // dem kich thuoc buffer
    for (int i = 0; buffer[i] != 0; i++)
        lenBuf++;
    // in ra
    synchconsole->Write(buffer, lenBuf);
    // synchconsole->Write("\n",1);
}

// Ham tao file
void CreateFile(char *filename) {
    // Input: Dia chi tu vung nho user cua ten file
    // Output: -1 = Loi, 0 = Thanh cong
    // Chuc nang: Tao ra file voi tham so la ten file
    if (strlen(filename) == 0) {
        printf("\n File name is not valid");
        DEBUG('a', "\n File name is not valid");
        machine->WriteRegister(2, -1);  // Return -1 vao thanh ghi R2
        return;
    }

    if (filename == NULL)  // Neu khong doc duoc
    {
        printf("\n Not enough memory in system");
        DEBUG('a', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);  // Return -1 vao thanh ghi R2
        delete filename;
        return;
    }
    DEBUG('a', "\n Finish reading filename.");

    if (!fileSystem->Create(filename, 0))  // Tao file bang ham Create cua fileSystem, tra ve ket qua
    {
        // Tao file that bai
        printf("\n Error create file '%s'", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }

    // Tao file thanh cong
    machine->WriteRegister(2, 0);
    delete filename;
    return;
}

// Ham mo file
void OpenaFile(char *filename, int type) {
    int freeSlot = fileSystem->FindFreeSlot();
    if (freeSlot != -1)  // Chi xu li khi con slot trong
    {
        if (type == 0 || type == 1)  // chi xu li khi type = 0 hoac 1
        {
            if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL)  // Mo file thanh cong
            {
                machine->WriteRegister(2, freeSlot);  // tra ve OpenFileID
            }
        } else if (type == 2)  // xu li stdin voi type quy uoc la 2
        {
            machine->WriteRegister(2, 0);  // tra ve OpenFileID
        } else                             // xu li stdout voi type quy uoc la 3
        {
            machine->WriteRegister(2, 1);  // tra ve OpenFileID
        }
        delete[] filename;
        return;
    }
    machine->WriteRegister(2, -1);  // Khong mo duoc file return -1

    delete[] filename;
    return;
}

// Ham dong file
void CloseFile(OpenFileId fid) {
    if (fid >= 0 && fid <= 14)  // Chi xu li khi fid nam trong [0, 14]
    {
        if (fileSystem->openf[fid])  // neu mo file thanh cong
        {
            delete fileSystem->openf[fid];  // Xoa vung nho luu tru file
            fileSystem->openf[fid] = NULL;  // Gan vung nho NULL
            machine->WriteRegister(2, 0);
            return;
        }
    }
    machine->WriteRegister(2, -1);
}

// Ham doc file
void ReadFile(int charcount, OpenFileId id) {
    // Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
    // Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
    // Cong dung: Doc file voi tham so la buffer, so ky tu cho phep va id cua file
    int virtAddr = machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
    int OldPos;
    int NewPos;
    char *buf;
    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
    if (id < 0 || id > 14) {
        printf("\nKhong the read vi id nam ngoai bang mo ta file.");
        machine->WriteRegister(2, -1);
        return;
    }
    // Kiem tra file co ton tai khong
    if (fileSystem->openf[id] == NULL) {
        printf("\nKhong the read vi file nay khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }
    if (fileSystem->openf[id]->type == 3)  // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
    {
        printf("\nKhong the read file stdout.");
        machine->WriteRegister(2, -1);
        return;
    }
    OldPos = fileSystem->openf[id]->GetCurrentPos();  // Kiem tra thanh cong thi lay vi tri OldPos
    buf = User2System(virtAddr, charcount);           // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
    // Xet truong hop doc file stdin (type quy uoc la 2)
    if (fileSystem->openf[id]->type == 2) {
        // Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
        int size = synchconsole->Read(buf, charcount);
        System2User(virtAddr, size, buf);  // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
        machine->WriteRegister(2, size);   // Tra ve so byte thuc su doc duoc
        delete buf;
        return;
    }
    // Xet truong hop doc file binh thuong thi tra ve so byte thuc su
    if ((fileSystem->openf[id]->Read(buf, charcount)) > 0) {
        // So byte thuc su = NewPos - OldPos
        NewPos = fileSystem->openf[id]->GetCurrentPos();
        // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
        System2User(virtAddr, NewPos - OldPos, buf);
        machine->WriteRegister(2, NewPos - OldPos);
    } else {
        // Truong hop con lai la doc file co noi dung la NULL tra ve -2
        machine->WriteRegister(2, -2);
    }
    delete buf;
    return;
}

// Ham viet file
void WriteFile(int charcount, OpenFileId id) {
    // Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
    // Output: -1: Loi, So byte write thuc su: Thanh cong, -2: Thanh cong
    // Cong dung: Ghi file voi tham so la buffer, so ky tu cho phep va id cua file
    int virtAddr = machine->ReadRegister(4);  // Lay dia chi cua tham so buffer tu thanh ghi so 4
    int OldPos;
    int NewPos;
    char *buf;
    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
    if (id < 0 || id > 14) {
        printf("\nKhong the write vi id nam ngoai bang mo ta file.");
        machine->WriteRegister(2, -1);
        return;
    }
    // Kiem tra file co ton tai khong
    if (fileSystem->openf[id] == NULL) {
        printf("\nKhong the write vi file nay khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }
    // Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
    if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2) {
        printf("\nKhong the write file stdin hoac file only read.");
        machine->WriteRegister(2, -1);
        return;
    }
    OldPos = fileSystem->openf[id]->GetCurrentPos();  // Kiem tra thanh cong thi lay vi tri OldPos
    buf = User2System(virtAddr, charcount);           // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
    // Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
    if (fileSystem->openf[id]->type == 0) {
        if ((fileSystem->openf[id]->Write(buf, charcount)) > 0) {
            // So byte thuc su = NewPos - OldPos
            NewPos = fileSystem->openf[id]->GetCurrentPos();
            machine->WriteRegister(2, NewPos - OldPos);
            delete buf;
            return;
        }
    }
    if (fileSystem->openf[id]->type == 3)  // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
    {
        int i = 0;
        while (buf[i] != 0 && buf[i] != '\n')  // Vong lap de write den khi gap ky tu '\n'
        {
            synchconsole->Write(buf + i, 1);  // Su dung ham Write cua lop SynchConsole
            i++;
        }
        buf[i] = '\n';
        synchconsole->Write(buf + i, 1);   // Write ky tu '\n'
        machine->WriteRegister(2, i - 1);  // Tra ve so byte thuc su write duoc
        delete buf;
        return;
    }
}

void _Exec(char *name) {
    // Input: vi tri int
    // Output: Fail return -1, Success: return id cua thread dang chay
    // SpaceId Exec(char *name);
    if (name == NULL) {
        DEBUG('a', "\n Not enough memory in System");
        printf("\n Not enough memory in System");
        machine->WriteRegister(2, -1);
        return;
    }
    OpenFile *oFile = fileSystem->Open(name);
    if (oFile == NULL) {
        printf("\nExec:: Can't open this file.");
        machine->WriteRegister(2, -1);
        return;
    }

    delete oFile;

    // Return child process id
    int id = pTab->ExecUpdate(name);
    machine->WriteRegister(2, id);

    delete[] name;
    return;
}

void _Join(int id) {
    // int Join(SpaceId id)
    // Input: id dia chi cua thread
    // Output:
    int res = pTab->JoinUpdate(id);
    machine->WriteRegister(2, res);
}

void _Exit(int status) {
    // void Exit(int status);
    //  Input: status code
    if (status != 0) return;

    int res = pTab->ExitUpdate(status);

    currentThread->FreeSpace();
    currentThread->Finish();
    return;
}

void _createSemaphore(char *name, int semval) {
    if (name == NULL) {
        DEBUG('a', "\n Not enough memory in System");
        printf("\n Not enough memory in System");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    int res = semTab->Create(name, semval);

    if (res == -1) {
        DEBUG('a', "\n Khong the khoi tao semaphore");
        printf("\n Khong the khoi tao semaphore");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    delete[] name;
    machine->WriteRegister(2, res);
    return;
}

void _Wait(char *name) {
    if (name == NULL) {
        DEBUG('a', "\n Not enough memory in System");
        printf("\n Not enough memory in System");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    int res = semTab->Wait(name);

    if (res == -1) {
        DEBUG('a', "\n Khong ton tai ten semaphore nay!");
        printf("\n Khong ton tai ten semaphore nay!");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    delete[] name;
    machine->WriteRegister(2, res);
    return;
}

void _Signal(char *name) {
    if (name == NULL) {
        DEBUG('a', "\n Not enough memory in System");
        printf("\n Not enough memory in System");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    int res = semTab->Signal(name);

    if (res == -1) {
        DEBUG('a', "\n Khong ton tai ten semaphore nay!");
        printf("\n Khong ton tai ten semaphore nay!");
        machine->WriteRegister(2, -1);
        delete[] name;
        return;
    }

    delete[] name;
    machine->WriteRegister(2, res);
    return;
}

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);

    switch (which) {
        case NoException:
            return;
        case PageFaultException:
            handleRuntimeError("\n PageFaultException: No valid translation found.");
            break;

        case ReadOnlyException:
            handleRuntimeError("\n ReadOnlyException: Write attempted to page marked \"read-only\".");
            break;

        case BusErrorException:
            handleRuntimeError("\n BusErrorException: Translation resulted invalid physical address.");
            break;

        case AddressErrorException:
            handleRuntimeError("\n AddressErrorException: Unaligned reference or one that was beyond the end of the address space.");
            break;

        case OverflowException:
            handleRuntimeError("\n OverflowException: Integer overflow in add or sub.");
            break;

        case IllegalInstrException:
            handleRuntimeError("\n IllegalInstrException: Unimplemented or reserved instr.");
            break;

        case NumExceptionTypes:
            handleRuntimeError("\n NumExceptionTypes: Number exception types.");
            break;

        case SyscallException:
            switch (type) {
                case SC_Halt: {
                    DEBUG('a', "Shutdown, initiated by user program.\n");
                    printf("Shutdown, initiated by user program.\n");
                    interrupt->Halt();
                    break;
                }

                // Khi nguoi dung goi syscall ReadChar.
                case SC_ReadChar: {
                    char e;
                    e = ReadChar();
                    machine->WriteRegister(2, e);
                    break;
                }

                // Khi nguoi dung goi syscall PrintChar.
                case SC_PrintChar: {
                    char d;
                    d = machine->ReadRegister(4);
                    PrintChar(d);
                    break;
                }

                // Khi nguoi dung goi syscall ReadInt.
                case SC_ReadInt: {
                    int g;
                    g = ReadInt();
                    machine->WriteRegister(2, g);
                    break;
                }

                // Khi nguoi dung goi syscall PrintInt.
                case SC_PrintInt: {
                    int m;
                    m = machine->ReadRegister(4);
                    PrintInt(m);
                    break;
                }

                // Khi nguoi dung goi syscall ReadString.
                case SC_ReadString: {
                    int length;
                    length = machine->ReadRegister(5);
                    char *buffer;
                    ReadString(buffer, length);
                    break;
                }

                // Khi nguoi dung goi syscall PrintString.
                case SC_PrintString: {
                    char *buf;
                    PrintString(buf);
                    break;
                }

                    // Khi nguoi dung goi syscall Create
                case SC_Create: {
                    int virtAddr;
                    char *filename;
                    DEBUG('a', "\n SC_CreateFile call ...");
                    DEBUG('a', "\n Reading virtual address of filename");

                    virtAddr = machine->ReadRegister(4);
                    DEBUG('a', "\n Reading filename.");

                    // Sao chep khong gian bo nho User sang System, voi do dang toi da la (32 + 1) bytes
                    filename = User2System(virtAddr, 32 + 1);
                    CreateFile(filename);
                    break;
                }

                    // Khi nguoi dung goi syscall Open
                case SC_Open: {
                    // Input: arg1: Dia chi cua chuoi name, arg2: type
                    // Output: Tra ve OpenFileID neu thanh, -1 neu loi
                    // Chuc nang: Tra ve ID cua file.

                    // OpenFileID Open(char *name, int type)
                    int virtAddr = machine->ReadRegister(4);  // Lay dia chi cua tham so name tu thanh ghi so 4
                    int type = machine->ReadRegister(5);      // Lay tham so type tu thanh ghi so 5
                    char *filename;
                    filename = User2System(virtAddr, 32);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem name dai 32
                    // Kiem tra xem OS con mo dc file khong
                    OpenaFile(filename, type);
                    break;
                }

                    // Khi nguoi dung goi syscall Close
                case SC_Close: {
                    // Input id cua file(OpenFileID)
                    //  Output: 0: thanh cong, -1 that bai
                    int fid = machine->ReadRegister(4);  // Lay id cua file tu thanh ghi so 4
                    CloseFile(fid);
                    break;
                }

                    // Khi nguoi dung goi syscall Read
                case SC_Read: {
                    int charcount = machine->ReadRegister(5);  // Lay charcount tu thanh ghi so 5
                    int id = machine->ReadRegister(6);         // Lay id cua file tu thanh ghi so 6
                    ReadFile(charcount, id);
                    break;
                }

                    // Khi nguoi dung goi syscall Write
                case SC_Write: {
                    int charcount = machine->ReadRegister(5);  // Lay charcount tu thanh ghi so 5
                    int id = machine->ReadRegister(6);         // Lay id cua file tu thanh ghi so 6
                    WriteFile(charcount, id);
                    break;
                }

                    // Khi nguoi dung goi syscall Exec
                case SC_Exec: {
                    int virtAddr = machine->ReadRegister(4);     // doc dia chi ten chuong trinh tu thanh ghi r4
                    char *name = User2System(virtAddr, 32 + 1);  // Lay ten chuong trinh, nap vao kernel
                    _Exec(name);
                    break;
                }

                    // Khi nguoi dung goi syscall Join
                case SC_Join: {
                    int id = machine->ReadRegister(4);
                    _Join(id);
                    break;
                }

                    // Khi nguoi dung goi syscall Exit
                case SC_Exit: {
                    int exitStatus = machine->ReadRegister(4);
                    _Exit(exitStatus);
                    break;
                }

                    // Khi nguoi dung goi syscall CreateSemaphore
                case SC_CreateSemaphore: {
                    // int CreateSemaphore(char* name, int semval).
                    int virtAddr = machine->ReadRegister(4);
                    int semval = machine->ReadRegister(5);

                    char *name = User2System(virtAddr, 32 + 1);
                    _createSemaphore(name, semval);
                    break;
                }

                    // Khi nguoi dung goi syscall Wait
                case SC_Wait: {
                    // int Wait(char* name)
                    int virtAddr = machine->ReadRegister(4);
                    char *name = User2System(virtAddr, 32 + 1);
                    _Wait(name);
                    break;
                }

                    // Khi nguoi dung goi syscall Signal
                case SC_Signal: {
                    // int Signal(char* name)
                    int virtAddr = machine->ReadRegister(4);
                    char *name = User2System(virtAddr, 32 + 1);
                    _Signal(name);
                    break;
                }
            }
            incProgCounter();  // tang thanh ghi
    }
}