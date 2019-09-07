#include <stdio.h>
#include <kernel.h>


unsigned long *esp;
unsigned long *ebp;

void printtos() {
	int p = 0;
	asm("movl %esp, esp");
	asm("movl %ebp, ebp");
	kprintf("Before [0x%08x]: 0x%08x\n", (esp + 0x8), *(esp + 0x8));
	kprintf("After  [0x%08x]: 0x%08x\n", (esp + 0x7), *(esp + 0x7));
	while(p < 4) {
		kprintf("\telement[0x%08x]: 0x%08x\n", (esp + p), *(esp + p) );
		p++;
	}
	return;
}
