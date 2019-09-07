/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
unsigned int new_table() {
	unsigned int frame, i;
	get_frm(&frame);
	pt_t *pt = (pt_t *)((FRAME0 + frame)*NBPG);
	for(i = 0; i < FRAME0; i++) {
		pt[i].pt_pres = 0;
		pt[i].pt_base = 0;
		pt[i].pt_dirty = 0;
		pt[i].pt_acc = 0;
		pt[i].pt_write = 0;
		pt[i].pt_pwt = 0;
		pt[i].pt_mbz = 0;
		pt[i].pt_user = 0;
		pt[i].pt_pcd = 0;
		pt[i].pt_global = 0;
		pt[i].pt_avail = 0;
	}
	return frame;
}

SYSCALL pfint() {
	STATWORD ps;
	disable(ps);
	unsigned long fault_addr = read_cr2();	
	virt_addr_t noo;
	noo = *(virt_addr_t *)&fault_addr;
	unsigned int vp = noo.pt_offset;
	unsigned long pd = proctab[currpid].pdbr;
	unsigned int table_th = noo.pd_offset;
	pd_t *dir_entry = pd + table_th*sizeof(pd_t);
	pt_t *ph_addr_table = (pt_t *)(dir_entry->pd_base*NBPG);
	unsigned int index, backing_store, bs_offset = -1;
	if(!dir_entry->pd_pres) {
		index = new_table();
		frm_tab[index].fr_pid = currpid;
		frm_tab[index].fr_status = FRM_MAPPED;
		frm_tab[index].fr_refcnt += 1;
		frm_tab[index].fr_dirty = 0;
		frm_tab[index].fr_type = FR_TBL;
		frm_tab[index].fr_vpno = fault_addr/NBPG;
		dir_entry->pd_pres = 1;
		dir_entry->pd_write = 1;
		dir_entry->pd_user = 0;
		dir_entry->pd_pwt = 0;
		dir_entry->pd_pcd = 0;
		dir_entry->pd_acc = 0;
		dir_entry->pd_mbz = 0;
		dir_entry->pd_fmb = 0;
		dir_entry->pd_global = 0;
		dir_entry->pd_avail = 0;
		dir_entry->pd_base = FRAME0 + index;
	}
	pt_t *pt_entry = (pt_t *)(dir_entry->pd_base*NBPG + vp*sizeof(pt_t));
	if(!(pt_entry->pt_pres)) {
		get_frm(&index);
		pt_entry->pt_pres = 1;
		frm_tab[index].fr_pid = currpid;
		frm_tab[index].fr_status = FRM_MAPPED;
		frm_tab[dir_entry->pd_base - 1024].fr_refcnt++;
		frm_tab[index].fr_dirty = 0;
		frm_tab[index].fr_type = FR_PAGE;
		frm_tab[index].fr_vpno = fault_addr/NBPG;
		pt_entry->pt_write = 1;
		pt_entry->pt_user = 0;
		pt_entry->pt_acc = 0;
		pt_entry->pt_dirty = 0;
		pt_entry->pt_mbz = 0;
		pt_entry->pt_global = 0;
		pt_entry->pt_avail = 0;
		pt_entry->pt_base = 1024 + index;
		restore(ps);
		bsm_lookup(currpid, fault_addr/NBPG, &backing_store, &bs_offset);
		//printf("[pfint %d] Reading %d at %d.\n", pd, FRAME0 + index, bs_offset);
		read_bs((char *)((FRAME0 + index)*NBPG), backing_store, bs_offset);
		disable(ps);
	}

	write_cr3(pd);	
	restore(ps);
	return OK;
}


