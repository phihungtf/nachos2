#include"syscall.h"

int main(){
	int t;
	PrintString("input number: ");
	t = ReadInt();
	PrintInt(t);
	PrintString("\n");
	Halt();
}
