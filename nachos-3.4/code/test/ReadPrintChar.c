#include"syscall.h"
int main(){
	char c;
	PrintString("input char: ");
	c = ReadChar();
	PrintChar(c);
	PrintString("\n");
	Halt();
}
