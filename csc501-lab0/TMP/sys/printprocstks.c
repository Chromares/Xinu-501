#include <stdio.h>
#include <kernel.h>
#include <proc.h>

extern struct pentry proctab[];

#define GET_PR(pid) (proctab[pid].pprio)
#define GET_NAME(pid) (proctab[pid].pname)
#define GET_SP(pid) (proctab[pid].pesp)
#define GET_BASE(pid) (proctab[pid].pbase)
#define GET_LIMIT(pid) (proctab[pid].plimit)
void printprocstks(int priority) {
	int index = 0;
	for(index; index < NPROC; index++) {
		if(GET_PR(index) > priority) {
			kprintf("Process [%s]\n", GET_NAME(index));
			kprintf("pid: %d\n\tpriority: %d\n\tbase: 0x%08x\n\tlimit: 0x%08x\n\tlen: %d\n\tpointer: 0x%08x\n", index, GET_PR(index), GET_BASE(index), GET_LIMIT(index), (GET_BASE(index) - GET_LIMIT(index)), GET_SP(index));
		}
		else
			continue;
	}
	return;
}
