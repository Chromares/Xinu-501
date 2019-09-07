/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
extern fr_map_t frm_tab[];
extern unsigned int pick_pid;

SYSCALL init_frm() {
	STATWORD ps;
	disable(ps);
	pick_pid = 0;
	unsigned int i = 0;
	for(i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = BADPID;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_NU;
		frm_tab[i].fr_dirty = 0;
	}	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail) {
	STATWORD ps;
	disable(ps);
	unsigned int i = 0;
	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_status == FRM_UNMAPPED) {
			*avail = i;
			restore(ps);
			return OK;
		}
	}
	virt_addr_t hue;
	unsigned int page_table_num;
	unsigned int page_number;
	pd_t *dir_entry;
	pt_t *table;
	unsigned long adr;
	i = (pick_pid + 1) % NFRAMES;
	while(1) {
	//	kprintf(" frame(%d) pid(%d) type(%d) status(%d)\n", i, frm_tab[i].fr_pid, frm_tab[i].fr_type, frm_tab[i].fr_status);
		
		if(frm_tab[i].fr_type) {
			i = (i + 1) % NFRAMES;
			continue;
		}
		adr = (frm_tab[i].fr_vpno*NBPG);
		hue = *(virt_addr_t *)&adr;
		page_number = hue.pt_offset;
		page_table_num = hue.pd_offset;
		dir_entry = (pd_t *)(proctab[frm_tab[i].fr_pid].pdbr + page_table_num*sizeof(pd_t));
		table = (pt_t *)(dir_entry->pd_base*NBPG + page_number*sizeof(pt_t));
		if(table->pt_acc) {
			table->pt_acc = 0;
		}
		else {
			*avail = i;
			pick_pid = i;
			if(table->pt_dirty) {
				frm_tab[i].fr_dirty = 1;
			}
			table->pt_pres = 0;
			free_frm(i);
			frm_tab[dir_entry->pd_base - 1024].fr_refcnt--;
			if(!frm_tab[dir_entry->pd_base - 1024].fr_refcnt) {
				dir_entry->pd_pres = 0;
				dir_entry->pd_write = 1;
				dir_entry->pd_user = 0;
				dir_entry->pd_pwt = 0;
				dir_entry->pd_pcd = 0;
				dir_entry->pd_acc = 0;
				dir_entry->pd_mbz = 0;
				dir_entry->pd_fmb = 0;
				dir_entry->pd_global = 0;
				dir_entry->pd_avail =  0;
				dir_entry->pd_base =  0;
				i = dir_entry->pd_base - 1024;
				frm_tab[i].fr_status = FRM_UNMAPPED;
				frm_tab[i].fr_pid = BADPID;
				frm_tab[i].fr_type = FR_NU;
				frm_tab[i].fr_refcnt = 0;
				frm_tab[i].fr_dirty = 0;	
			}
			restore(ps);
			return OK;
		}
		i = (i + 1) % NFRAMES;
	}
	restore(ps);
	return OK;
}
/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i) {
	STATWORD ps;
	disable(ps);
	unsigned int pid = frm_tab[i].fr_pid;
	unsigned int vpn = frm_tab[i].fr_vpno; 
	if(frm_tab[i].fr_type != FR_PAGE) {
		kprintf("[frame.c <free_frm>] Invalid frame selction.\n");
		restore(ps);
		return SYSERR;
	}
	if(frm_tab[i].fr_dirty) {
		unsigned int store, page;
		bsm_lookup(pid, vpn, &store, &page);
		write_bs((1024 + i)*NBPG, store, page);
	}
	frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid = BADPID;
	frm_tab[i].fr_type = FR_NU;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_dirty = 0;
	restore(ps);
	return OK;
}
