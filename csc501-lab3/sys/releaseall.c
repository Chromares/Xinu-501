#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

int getnextp(int lock) {
	int writer_index = -1, nextpick = -1;
	int min = MAXINT, min_index = -1;
	int i = q[locktab[lock].ltail].qprev;
	int current = q[i].qkey;
	int latest = q[i].qtime;
	i = q[i].qprev;
	while(i != locktab[lock].lhead) {
		if(current == q[i].qkey) {
			if(min < (q[i].qtime)) {
				min = q[i].qtime;
				min_index = i;
			}
			if(q[i].qtype == LTWRITE)
				writer_index = 1;
		}
		i = q[i].qprev;
	}
	if((latest - min) < 1000)
		nextpick = writer_index;
	else
		nextpick = min_index;
	return (nextpick == -1 ? q[locktab[lock].ltail].qprev : nextpick);
}

int newlprio(int lock) {
	int max = MININT, i;
	for(i = 0; i < NPROC; i++)
		if(locktab[lock].held[i]) {
			//kprintf("#(%d with %d) ", i, proctab[i].pprio);
			max = (max > proctab[i].pprio ? max : proctab[i].pprio);
	}
	return (max);
}

int releasethis(int pid, int lock) {
	if((!proctab[pid].heldin[lock]) || isbadlock(lock))
		return SYSERR;
	struct lentry *lptr = &locktab[lock];
	if(lptr->lstate != LUSED)
		return SYSERR;

	proctab[pid].heldin[lock]--;
	if(!lptr->nreaders)
		lptr->nwriters--;
	else
		lptr->nreaders--;
	if((lptr->nreaders) || (lptr->nwriters))
		return OK;
	if(q[locktab[lock].ltail].qprev == locktab[lock].lhead)
		return OK;  	
	int pick = getnextp(lock);
	dequeue(pick);
	ready(pick, RESCHNO);
	locktab[lock].held[pick]--;
	proctab[pick].lockid = -1;
	proctab[pick].heldin[lock]++;
		
	int nextprio = newlprio(lock);
	if(nextprio == MININT) {
		locktab[lock].lprio = proctab[pick].pprio;
	}
	else if(proctab[pick].pprio < nextprio) {
		//kprintf("Updating %d(%d) to %d.\n", proctab[pick].pprio, pick, nextprio);
		proctab[pick].pinh = proctab[pick].pprio;
		proctab[pick].pprio = nextprio;
		locktab[lock].lprio = nextprio;
	}
	else
		locktab[lock].lprio = proctab[pick].pprio;

	if(q[pick].qtype == LTREAD) {
		lptr->nreaders++;
		//while(q[(pick = getnextp(lock))].qtype == LTREAD) {
		int k = lptr->ltail;
		k = q[k].qprev;
		if((k >= NPROC) || (k <= 0))
			return OK;
		while((k != lptr->lhead) && (k != lptr->ltail)) {
			if(q[k].qtype == LTWRITE) {
				k = lptr->lhead;
				continue;
			}
			if((k >= NPROC) || (k <= 0))
				return OK;
			pick = k;
			dequeue(pick);
			ready(pick, RESCHNO);
			
			//kprintf("[relaseall] Also unlocking %d for next pick %d.\n", lock, pick);
			lptr->nreaders++;
			locktab[lock].held[pick]--;
			proctab[pick].lockid = -1;
			proctab[pick].heldin[lock]++;
			nextprio = newlprio(lock);
			if(nextprio == MININT) {
				locktab[lock].lprio = proctab[pick].pprio;
			}
			else if(proctab[pick].pprio < nextprio) {
				//kprintf("Updating %d(%d) to %d.\n", proctab[pick].pprio, pick, nextprio);
				proctab[pick].pinh = proctab[pick].pprio;
				proctab[pick].pprio = nextprio;
				locktab[lock].lprio = nextprio;
			}
			else
				locktab[lock].lprio = proctab[pick].pprio;

			k = q[k].qprev;
		}
	}
	else {
		lptr->nwriters++;
	}
	return OK;
}

int releaseall(int numlocks, int ldes1, ...) {
	STATWORD ps;
	int i, lock;
	int ret = 1;
	disable(ps);
	for(i = 0; i < numlocks; i++) {
		lock = (int)(*(&ldes1 + i));
		if(releasethis(currpid, lock) == SYSERR) {
			//restore(ps);
			ret = SYSERR;
		}
	}
	restore(ps);
	resched();
	return ret;
}
