#include <kernel.h>
//#include <conf.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

#define NLR (lptr->nreaders)
#define NLW (lptr->nwriters)

#define HELD(n, x) proctab[n].heldin[x]
#define PRI(x) proctab[x].pprio

void cascadep(int pid) {
	int check = proctab[pid].lockid;
	int i;
	if(check == -1) {
		return;
	}
	if(locktab[check].lprio < proctab[pid].pprio) {
		if(newlprio(check) == MININT) {
			return;
		}
		locktab[check].lprio = proctab[pid].pprio;
		for(i = 0; i < NPROC; i++) {
			if((proctab[i].heldin[check]) && (proctab[i].pprio < proctab[pid].pprio)) {
				if(!proctab[i].pinh)
					proctab[i].pinh = proctab[i].pprio;
				proctab[i].pprio = proctab[pid].pprio;
				cascadep(i);
			}
		}
	}
	return;
}

int wait_lock(int lock, int type, int key) {
	//kprintf("[lock.c] Putting %d(%d) for lock %d.\n", key, currpid, lock);
	struct pentry *pptr = &proctab[currpid];
	pptr->pstate = PRLOCK;
	pptr->lockid = lock;
	pptr->plockret = OK;
	int iter;
	if(pptr->pprio > locktab[lock].lprio) {
		locktab[lock].lprio = pptr->pprio;
		for(iter = 0; iter < NPROC; iter++) {
			if((HELD(iter, lock)) && (PRI(iter) < pptr->pprio)) {
				//kprintf("[lock.c] Updating %d(%d) to %d.\n", proctab[iter].pprio, iter, pptr->pprio);
				if(proctab[iter].pinh == 0)
					proctab[iter].pinh = proctab[iter].pprio;
				proctab[iter].pprio = pptr->pprio;
				cascadep(iter);
			}
		}
	}
	insert(currpid, locktab[lock].lhead, key);
	q[currpid].qtype = type;
	q[currpid].qtime = ctr1000;
	locktab[lock].held[currpid] = 1;
	return OK;
}

int lock(int ldes1, int type, int priority) {
	STATWORD ps;
	if(isbadlock(ldes1))
		return SYSERR;
	
	struct pentry *pptr;
	struct lentry *lptr;
	lptr = &locktab[ldes1];
	if(lptr->lstate == LFREE) { //no such lock exists
		restore(ps);
		return SYSERR;
	}
	
	if(lptr->lstate == DELETED) {
		if(lptr->held[currpid])
			lptr->held[currpid] = 0;
		int ktr;
		for(ktr = 0; ktr < NPROC; ktr++) {
			if(lptr->held[currpid])
				return SYSERR;
		}
		lptr->lstate = LFREE;
		return SYSERR;
	}
	disable(ps);
	int iter;
	if(type == LTREAD) {
		if(!NLW) {
			iter = q[lptr->lhead].qnext;
			while(iter != lptr->ltail) {
				if((q[iter].qtype == LTWRITE) && (priority < q[iter].qkey)) {
					wait_lock(ldes1, LTREAD, priority);
					resched();
					restore(ps);
					return proctab[currpid].plockret;
				}
				iter = q[iter].qnext;
			}
			//kprintf("[lock.c] Granted %d(%d) on lock %d.\n", priority, currpid, ldes1);
			proctab[currpid].heldin[ldes1]++;
			proctab[currpid].plockret = OK;
			NLR++;
			restore(ps);
			return OK;
				
		}
		if(NLW) {
			wait_lock(ldes1, LTREAD, priority);
			iter = proctab[currpid].plockret;
			resched();
			restore(ps);
			return iter;
		}
	}
	else {
		//type is write, need to wait for !NLR && !NLW
		if(!NLR && !NLW) {
			//kprintf("[lock.h] Granted %d(%d) on lock %d.\n", priority, currpid, ldes1);
			proctab[currpid].heldin[ldes1]++;
			proctab[currpid].plockret = OK;
			NLW++;
			restore(ps);
			return OK;
			
		}
		wait_lock(ldes1, LTWRITE, priority);
		iter = proctab[currpid].plockret;
		resched();
		restore(ps);
		return iter;
	}
	return SYSERR;
}
