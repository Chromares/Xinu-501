/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

int prX;
void halt();
prch(c)
char c;
{	
	int i = 500;
	kprintf("\nProc 2 (printsegaddress)\n");	
	printsegaddress();
	while(i--);
		sleep(1);
}
int main() {
	kprintf("\n\nHello World, Xinu lives\n\n");
	
	kprintf("Task 1 (zfunction)\n");
	long zcheck = zfunction(0xaabbccdd);
	kprintf("zfunction(0xaabbccdd) =  %lX\n", zcheck);

	kprintf("\nTask 2 (printsegaddress)\n");	
	printsegaddress();

	kprintf("\nTask 3 (printtos)\n");	
	printtos();

	resume(prX = create(prch, 2000, 20, "proc X", 1, 'A'));
	
	kprintf("\nTask 4 (printprocstks)\n");
	printprocstks(1);
	
	kprintf("\nTask 5 (syscallsummary)\n");
	syscallsummary_start();
	resume(prX = create(prch, 2000, 20, "proc X", 1, 'A'));
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();

	return 0;
}
