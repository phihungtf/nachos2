#include "syscall.h"
int main(){
	
	int t;
	char* c;
	PrintString("length= ");
	t = ReadInt();
	PrintString("input: ");
	ReadString(c,t);
	PrintString(c);
	Halt();
}
