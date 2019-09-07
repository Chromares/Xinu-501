#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
   // kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	if((npages < 1) || (npages > BSD_SIZE_PAGE) || (bs_id < 0) || (bs_id >= BSD_SIZE_PAGE)) {
		restore(ps);
		return SYSERR;
	}
	if(bsm_tab[bs_id].bs_pid == BADPID){
		restore(ps);
		return SYSERR;
	}
	bsm_tab[bs_id].bs_npages = npages;
	bsm_tab[bs_id].bs_pid = currpid;
	//bsm_map(currpid, BSM_BASE_PAGE, bs_id, npages);
	restore(ps);
	return npages;

}


