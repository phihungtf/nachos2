#include "syscall.h"

int main() {
    PrintString("My group has 5 people\n");
    PrintString("20120470 - Nguyen Van Hao\n");
    PrintString("20120474 - Le Kim Hieu\n");
    PrintString("20120488 - Thai Nguyen Viet Hung\n");
    PrintString("20120489 - Vo Phi Hung\n");
    PrintString("20120496 - Nguyen Canh Huy\n");
    PrintString("\n");
    PrintString("ReadPrintInt: In directory, enter './userprog/nachos -rs 1023 -x ./test/ReadPrintInt' to run program\n");
    PrintString("ReadPrintChar: In directory, enter './userprog/nachos -rs 1023 -x ./test/ReadPrintChar' to run program\n");
    PrintString("ReadPrintString: In directory, enter './userprog/nachos -rs 1023 -x ./test/ReadPrintString' to run program\n");
    PrintString("ASCII: In directory, enter './userprog/nachos -rs 1023 -x ./test/ascii' to run program\n");
    PrintString("sort: In directory, enter './userprog/nachos -rs 1023 -x ./test/sort' to run program\n");
    PrintString("ping: In directory, enter './userprog/nachos -rs 1023 -x ./test/ping' to run program\n");
    PrintString("pong: In directory, enter './userprog/nachos -rs 1023 -x ./test/pong' to run program\n");
    PrintString("pingpong: In directory, enter './userprog/nachos -rs 1023 -x ./test/pingpong' to run program\n");
    PrintString("createFile: In directory, enter './userprog/nachos -rs 1023 -x ./test/createFile' to run program\n");
    Halt();
}
