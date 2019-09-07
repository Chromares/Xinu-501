/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages) {
	//kprintf("xmmap - to be implemented!\n");
	STATWORD ps;
	disable(ps);
	if((source >= NBSM_TAB) || (npages > BSD_SIZE_PAGE) || (virtpage < 4096)) {
		//Below 4096, its all global
		kprintf("[xm.c <xmmap>] Nice parameters.\n");
		restore(ps);
		return SYSERR;
	
	}
 	if(bsm_map(currpid, virtpage, source, npages) == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	restore(ps);
	return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage) {
	STATWORD ps;
	disable(ps);
	if(virtpage < 4096) {
		kprintf("[xm.c <xmunmap>] Nice parameter.\n");
		restore(ps);
		return SYSERR;
		
	}
	restore(ps);
	return bsm_unmap(currpid, virtpage, 0);
}
