#include "syscall.h"

int main() {
	int i;
	for(i = 32;i <= 126;i++){
		PrintInt(i);
		PrintChar(' ');
		PrintChar(i);
		PrintString("\n");
	}
	Halt();
}

