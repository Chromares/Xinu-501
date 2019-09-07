#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

int lcreate() {
	STATWORD ps;
	disable(ps);

	int i = 0, h = 0;
	int lock, *hptr;
	for(i = 0; i < NLOCK; i++) {
		lock = nextlock--;
		if(nextlock < 0)
			nextlock = NLOCK - 1;
		if(locktab[lock].lstate == LFREE) {
			locktab[lock].lstate = LUSED;
			locktab[lock].nreaders = 0;
			locktab[lock].nwriters = 0;
			locktab[lock].lprio = 0;
			hptr = &(locktab[lock].held);
			for(h = 0; h < NPROC; h++) {
				hptr[h] = 0;
			}
			restore(ps);
			return(lock);
		}
	}
	restore(ps);
	return SYSERR;
} 
