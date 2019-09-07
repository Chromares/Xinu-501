/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
extern bs_map_t bsm_tab[];

SYSCALL init_bsm() {
	STATWORD ps;
	disable(ps);
	bsd_t i = 0;
	for(i = 0; i < NBSM_TAB; i++) {
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = BADPID;
		bsm_tab[i].bs_vpno = 4096;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem = 0;	
	}
	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail) {
	STATWORD ps;
	disable(ps);
	bsd_t i = 0;
	for(i = 0; i < NBSM_TAB; i++) {
		if(bsm_tab[i].bs_status == BSM_UNMAPPED) {
			*avail = i;
			restore(ps);
			return(OK);
		}
	}
	restore(ps);
	kprintf("[bsm.c <get_bsm>] BS full.\n");
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i) {
	STATWORD ps;
	disable(ps);
	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = BADPID;
	bsm_tab[i].bs_vpno = 4096;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_sem = 0;
	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth) {
	STATWORD ps;
	disable(ps);
	bsd_t i = 0;
	for(i = 0; i < NBSM_TAB; i++) {
		if(bsm_tab[i].bs_pid == pid) {
			*store = i;
			*pageth = ((vaddr) - bsm_tab[i].bs_vpno);
			restore(ps);
			return(OK);
		}
	}
	restore(ps);
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages) {
	STATWORD ps;
	disable(ps);
	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_pid = pid;
	bsm_tab[source].bs_sem = 0;
	bsm_tab[source].bs_npages = npages;
	bsm_tab[source].bs_vpno = vpno;
	proctab[pid].store = source;
	proctab[pid].vhpno = vpno;
	proctab[pid].vhpnpages = npages;
	restore(ps);
	return(OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag) {
	STATWORD ps;
	disable(ps);
	bsd_t store;
	int pageth;
	unsigned long va = vpno*NBPG;
	unsigned int i = 0;
	
	for(i = 0; i < NFRAMES; i++) {
		if((frm_tab[i].fr_pid == pid) && (frm_tab[i].fr_type == FR_PAGE) && (frm_tab[i].fr_dirty)) {
			bsm_lookup(pid, vpno, &store, &pageth);
			write_bs((FRAME0 + i)*NBPG, store, pageth);
		}
		if(frm_tab[i].fr_pid == pid) {
			frm_tab[i].fr_status = FRM_UNMAPPED;
			frm_tab[i].fr_type = FR_NU;	
		}
	}
	free_bsm(store);
	restore(ps);
	return(OK);
}


