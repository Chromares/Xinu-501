/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */

#define mivml(pid) proctab[pid].vmemlist

WORD *vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;
	struct mblock *p, *q, *leftover;
	unsigned int pid;
	disable(ps);
	pid = currpid;
	if((nbytes == 0) || (mivml(pid)->mnext == (struct mblock *) NULL)) {
		restore(ps);
		return ((WORD *)SYSERR);
	}
	nbytes = (unsigned int) roundmb(nbytes);
	
	q = &mivml(pid);
	p = mivml(pid)->mnext;
	while(p != (struct mblock *)NULL) {
		if(p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			return((WORD *)p);
		} else if(p->mlen > nbytes) {
			leftover = (struct mblock *)((unsigned)p + nbytes);
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			return((WORD *)p);
		}
		q = p;
		p = p->mnext;
	
	}
	restore(ps);
	return((WORD *)SYSERR);
}


