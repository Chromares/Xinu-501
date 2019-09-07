/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */

SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if(pptr->pinh > 0) {
		if(pptr->pprio < newprio) {
			pptr->pprio = newprio;
			pptr->pinh = 0;
			cascadep(pid);
			restore(ps);
			return OK;
		}
		else if(pptr->pprio > newprio) {
			pptr->pinh = newprio;
			restore(ps);
			return OK;
		}
	}
	else
		pptr->pprio = newprio;
	if(pptr->lockid >= 0)
		cascadep(pid);
	
	restore(ps);
	return(newprio);
}
