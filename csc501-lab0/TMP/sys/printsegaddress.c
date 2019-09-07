#include <stdio.h>
extern unsigned long etext, edata, end;

void printsegaddress() {
	kprintf("Current: etext[0x%08x]=%08x, edata[0x%08x]=%08x, ebssa[0x%08x]=%08x\n", &etext, etext, &edata, edata, &end, end);
	kprintf("Preceding: etext[0x%08x]=%08x, edata[0x%08x]=%08x, ebssa[0x%08x]=%08x\n", (&etext - 1), *(&etext - 1), (&edata - 1), *(&edata - 1), (&end - 1), *(&end - 1));
	kprintf("After: etext[0x%08x]=%08x, edata[0x%08x]=%08x, ebssa[0x%08x]=%08x\n", (&etext + 1), *(&etext + 1), (&edata + 1), *(&edata + 1), (&end + 1), *(&end + 1));
	return;
}
