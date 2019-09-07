#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

#define LD lockdescriptor
 
int ldelete(int lockdescriptor) {
	STATWORD ps;
	int pid;
	struct lentry *lptr;
	if(isbadlock(LD)) {
		return SYSERR;
	}
	disable(ps);
	if(locktab[LD].lstate = LFREE) {
		restore(ps);
		return SYSERR;
	}
	lptr = &locktab[LD];
	lptr->lstate = DELETED;
	//check what is there in queue
	if(nonempty(lptr->lhead)) {
		while((pid = getfirst(lptr->lhead)) != EMPTY) {
			proctab[pid].plockret = DELETED;
			ready(pid, RESCHNO);
		}
		resched();
	}
	lptr->nreaders = 0;
	lptr->nwriters = 0;

	int i;
	int *held = &(locktab[LD].held[0]);
	for(i = 0; i < NPROC; i++) {
		if(held[i]) {
			restore(ps);
			return OK;		
		}
	}
	//none is holding this lock
	lptr->lstate = LFREE;
	return OK;
	
}
