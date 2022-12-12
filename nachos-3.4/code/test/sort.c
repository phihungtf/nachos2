#include "syscall.h"
#define MAX_SIZE 50

int main() {
    int size = 0, i, arr[MAX_SIZE + 1], j, temp;
    do {
        PrintString("Enter the number size of array [0,50] n = ");
        size = ReadInt();
    } while (size < 0 || size > 50);

    for (i = 0; i < size; i++) {
        PrintString("Enter arr[");
        PrintChar(i + '0');
        PrintString("] = ");
        arr[i] = ReadInt();
    }

    for (i = 0; i < size - 1; i++) {
        for (j = i + 1; j < size; j++) {
            if (arr[i] > arr[j]) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }

    for (i = 0; i < size; i++) {
        PrintInt(arr[i]);
        PrintChar(' ');
    }
    Halt();
}
