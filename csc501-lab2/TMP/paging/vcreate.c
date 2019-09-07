/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{

	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	unsigned int pid = create(procaddr, ssize, priority, name, nargs, args);
	unsigned int store;
	if(get_bsm(&store) == SYSERR) {
		restore(ps);
		return SYSERR;
	}	
	
	if(bsm_map(pid, 4096, store, hsize) == SYSERR) {
		restore(ps);
		return SYSERR;
	}

	proctab[pid].vhpnpages = hsize;
	proctab[pid].vhpno = 4096;
	//Each process has a vemlist since we
	//still can have a segmented view of paged memory;
	//OR
	//TO EXPAND VHEAP inside backing store
	//
	//the pages may not be continuous
	//this can be used to find who is where?
	//WE KEEP THESE WHERE FREE MEM IS? SEE GETMEM.C
	struct mblock *temp;
	temp = BACKING_STORE_BASE + store*BACKING_STORE_UNIT_SIZE;
	temp->mlen = hsize*NBPG;
	temp->mnext = NULL;
	proctab[pid].vmemlist->mnext = 4096*NBPG;
	//proctab[pid].vmemlist->mnext = temp;
	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
