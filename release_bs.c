#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* release the backing store with ID bs_id */
SYSCALL release_bs(bsd_t bs_id) {
STATWORD intr;
disable(intr);

free_bsm(bs_id);

restore(intr);
return(OK);
}

