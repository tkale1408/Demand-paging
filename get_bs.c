#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages) {
STATWORD intr;
disable(intr);

if (bs_id > 7 || bs_id < 0 || npages > 255 || npages < 0) {
	restore(intr);
	return SYSERR;
}

if (bsm_tab[bs_id].bs_status == BSM_UNMAPPED && bsm_tab[bs_id].bs_sem == 0) {
	bsm_tab[bs_id] = (bs_map_t){BSM_MAPPED, currpid, bsm_tab[bs_id].bs_vpno, npages, 1, 1};
	restore(intr);
	return npages;
} else {
	restore(intr);
	return bsm_tab[bs_id].bs_npages;
}

restore(intr);
return npages;
}


