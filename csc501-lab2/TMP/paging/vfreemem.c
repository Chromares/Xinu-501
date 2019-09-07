/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */

#define mivml(pid) proctab[pid].vmemlist
#define UN (unsigned)

SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);
	unsigned int pid = currpid;
	if(size == 0 || block == (struct mblock *)NULL) {
		restore(ps);
		return SYSERR;
	}
	
	//if((unsigned)block > (unsigned)maxaddr || (unsigned)block < ((unsigned) &end)) {
	//	kprintf("[vfreemem.c <vfreemem>] Don't take was isn't yours.");
	//	restore(ps);
	//	return SYSERR;
	//}
	
	struct mblock *p, *q;
	size = UN roundmb(size);
	p = mivml(pid)->mnext;
	q = &mivml(pid);
	
	while((p != (struct mblock *)NULL) && (p < block)) {
		q = p;
		p = p->mnext;
	}
	
	unsigned int top = q->mlen + UN q;
	if(((top > UN block) && q != &mivml(pid)) || ((p != NULL) && (size + UN block) > UN p)) {
		kprintf("[vfreemem.c <vfreemem> This was already partly or completely free]");
		restore(ps);
		return SYSERR;
	}
	//top is end of last free block;
	//if is match, we make this block longer
	//merge blocks
	if(q != &mivml(pid) && top == UN block)
		q->mlen += size;
	else {	//add new box here
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	} //if we touch next box, merge that too
	//mega block
	if(UN (q->mlen + UN q) == UN p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);
	return(OK);
}
