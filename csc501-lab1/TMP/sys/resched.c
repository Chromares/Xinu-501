/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 *
 */
extern struct qent q[];
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/
	
	int class = getschedclass();
	int next_proc;
	if(class == EXPDISTSCHED) {
		int random_val;
		random_val = (int) expdev(0.1);
		
		int next = q[rdyhead].qnext;
		while(next  < NPROC) {		
			if(random_val >= q[next].qkey)
				next = q[next].qnext;
			else if(q[next].qkey == q[q[next].qnext].qkey) {
				random_val = q[next].qkey;
				next = q[next].qnext;
			}
			else  {
				next_proc = next;
				break;
			}
		}
		//insert.c will put a proc with same priority nearer to head and we pick proc with same pprio that is nearer to tail;
		//This fact can be used for round robin part
		if(next >= NPROC) {
			next_proc = q[next].qprev;
		}
		if(next_proc >= NPROC)
			next_proc = NULLPROC;
		optr = &proctab[currpid];
		//The example considers ready and running process
		//So we check if current process had a prio larger than random value and smaller priority than the process picked otherwise
		//In this case the same process would have been picked
		
		//if((optr->pstate == PRCURR)) {
		//	if((optr->pprio > random_val)) {
		//		if((q[next_proc].qkey > optr->pprio)) {
		//			#ifdef RTLOCK
		//				preempt = QUANTUM;
		//			#endif
		//			return(OK);
		//		}
		//	}
		//}

		int check_1 = 0, check_2 = 0;
		next = 0;
		if(optr->pstate == PRCURR)
			check_1 = 1;
		if(q[next_proc].qkey > optr->pprio)
			check_2 = 1;
		if(optr->pprio > random_val)
			next = 1;
	
		if(check_1 && (check_2 && next)){
			#ifdef	RTCLOCK
				preempt = QUANTUM;
			#endif
			return(OK);
		}

	}
	else if(class == DEFAULT) {
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	}
	else if(class == LINUXSCHED) {
		optr = &proctab[currpid];
		//Update the number of ticks left and the goodness
		int temp = (optr->goodness - optr->counter);
		//optr->goodness = optr->goodness -optr->counter;
		optr->goodness = temp + preempt;
		optr->counter = preempt;
		//If counter is over, we should not schedule the same process in the same epoch again
		if((optr->counter <= 0)) {
			optr->goodness = 0;
			optr->counter = 0;
		}
		else if(optr->pstate == PRCURR) {
			//Someone called resched thats not currpid
			//We ignore to maintain no preemption policy
			preempt = optr->counter;
			return(OK);			
		}
		//Find next process
		int next = q[rdyhead].qnext;
		int goodness_max = 0;
		while(next < NPROC) {
			//not sorted on goodness;
			if(proctab[next].goodness >= goodness_max) {
				goodness_max = proctab[next].goodness;
				next_proc = next;
			}
			next = q[next].qnext;
		}
		
		//Either optr is in ready queue and goodness_max is still 0 -> optr->counter = 0
		//OR
		//Optr is PRCURR -> optr->counter = 0 since this is the first thing we check
		//OR
		//Optr is neither in PRDY not PRCURR, which means it doesn't matter
		//All these mean no runnable process and mark end of epoch
		if(!goodness_max) {
			struct pentry *tmp;
			int i;
			for(i = 0; i < NPROC; i++) {
				tmp = &proctab[i];
				if(tmp->pstate == PRFREE) //Do nothing if slot is free
					continue;
				if((tmp->counter < tmp->quantum) && (tmp->counter > 0)) //process ran
					tmp->quantum = (tmp->counter) / 2;	//carry on half of ticks left
				else
					tmp->quantum = 0;
				tmp->quantum += tmp->pprio; //update quantum;
				tmp->counter = tmp->quantum; //set up counter for next epoch
				tmp->goodness = tmp->pprio + tmp->counter; //update goodness					
			}
		}
	
		next = q[rdyhead].qnext;
		goodness_max = 0;
		while(next  < NPROC) {
		//not sorted on goodness;
			if(proctab[next].goodness >= goodness_max) {
				goodness_max = proctab[next].goodness;
				next_proc = next;
			}
			next = q[next].qnext;
		}
		//nothing in ready, nothing to run
		if((goodness_max == 0)) {
				//run null!
				next_proc = NULLPROC;
		}
		if((proctab[next_proc].goodness < optr->goodness) && (optr->counter > 0) && (optr->pstate == PRCURR)) {
				preempt = optr->counter;
				return OK;
		}
		optr = &proctab[currpid];
		nptr = &proctab[next_proc];
	}
	/* force context switch */
	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}
	/* remove highest priority process at end of ready list */
	if(class == DEFAULT) {
		nptr = &proctab[ (currpid = getlast(rdytail)) ];
	}
	else {	//getlast calls dequeue to remove the process from ready list;
		currpid = next_proc;
		nptr = &proctab[currpid];
		dequeue(currpid);
	}				//insert fills in from head, so its a safe insert. Should RR by itself
	if(class == LINUXSCHED) {
		nptr->pstate = PRCURR;
		preempt = nptr->counter;
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return OK;	
	}

	nptr->pstate = PRCURR;		/* mark it currently running	*/
	#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
	#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
