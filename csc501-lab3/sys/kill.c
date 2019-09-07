/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

void update_lprio(int lock, int max) {
	int i;
	//kprintf("[KILL] Checking in for lock %d prvwait(%d).\n", lock, max);
	for(i = 0; i < NPROC; i++) {
		if(proctab[i].heldin[lock]) {
			//kprintf("(%d[%d]) ", i, proctab[i].pprio);
			if((proctab[i].pprio == max) && (proctab[i].pinh > 0)) {				
				//kprintf("[KILL] RESTORE %d(%d) to %d.\n", proctab[i].pprio, i, proctab[i].pinh);
				proctab[i].pprio = proctab[i].pinh;
				proctab[i].pinh = 0;
			}
		}
	}
	locktab[lock].lprio = newlprio(lock);
	int maxp = MININT;
	if(locktab[lock].lprio <= 0) {
		for(i = 0; i < NPROC; i++) {
			if(proctab[i].heldin[lock]) {
				if(proctab[i].pprio > maxp)
					maxp = proctab[i].pprio;
			}
		}
		locktab[lock].lprio = maxp;
	}
	//kprintf("[KILL] NEWLPRIO = %d\n", locktab[lock].lprio);
	for(i = 0; i < NPROC; i++) {
		if(proctab[i].heldin[lock]) {
			if(proctab[i].pprio < locktab[lock].lprio) {
				//kprintf("[KILL] UPDATE %d(%d) to %d.\n", proctab[i].pprio, i, locktab[lock].lprio);
				proctab[i].pinh = proctab[i].pprio;
				proctab[i].pprio = locktab[lock].lprio;
			}
		}
	}
	return;
}

SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	struct lentry *lptr;
	int i;
	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);
	
	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	if(pptr->lockid >= 0) {
				lptr = &locktab[pptr->lockid];
				//dequeue(pid);
				if(lptr->held[pid]) {
					lptr->held[pid] = 0;
					update_lprio(pptr->lockid, pptr->pprio);
				}
			}
			for(i = 0; i < NLOCK; i++) {
				if(proctab[pid].heldin[i]) {
					releasethis(pid, i);
					proctab[pid].heldin[i] = 0;
				}
			}
			pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:  
			if(pptr->lockid >= 0) {
				lptr = &locktab[pptr->lockid];
				//dequeue(pid);
				if(lptr->held[pid]) {
					lptr->held[pid] = 0;
					update_lprio(pptr->lockid, pptr->pprio);
				}
			}
			for(i = 0; i < NLOCK; i++) {
				if(proctab[pid].heldin[i]) {
					releasethis(pid, i);
					proctab[pid].heldin[i] = 0;
				}
			}
			dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	case PRLOCK:	i = newlprio(pptr->lockid);	
			if(pptr->lockid >= 0) {
				lptr = &locktab[pptr->lockid];
				//dequeue(pid);
				if(lptr->held[pid]) {
					lptr->held[pid] = 0;
					update_lprio(pptr->lockid, pptr->pprio);
				}
			}
			for(i = 0; i < NLOCK; i++) {
				if(proctab[pid].heldin[i]) {
					releasethis(pid, i);
					proctab[pid].heldin[i] = 0;
				}
			}
			pptr->pstate = PRFREE;
	default:	
			pptr->pstate = PRFREE;
			if(pptr->lockid >= 0) {
				lptr = &locktab[pptr->lockid];
				//dequeue(pid);
				if(lptr->held[pid]) {
					lptr->held[pid] = 0;
					update_lprio(pptr->lockid, pptr->pprio);
				}
			}
			for(i = 0; i < NLOCK; i++) {
				if(proctab[pid].heldin[i]) {
					releasethis(pid, i);
					proctab[pid].heldin[i] = 0;
				}
			}
	}
	restore(ps);
	return(OK);
}
