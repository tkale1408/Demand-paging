/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{

STATWORD intr;
disable(intr);

if (virtpage < 4096 || source < 0 || source > 7 || npages < 0 || npages > 255) {
	restore(intr);
	return SYSERR;
} else {
	get_bsm(&source);
}

bsm_map(currpid, virtpage, source, npages);
restore(intr);
return OK;
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{

STATWORD intr;
disable(intr);

if (virtpage < 4096 || virtpage < 0) {
	restore(intr);
  	return SYSERR;
} 

if (currpid > -1) {
	bsm_unmap(currpid, virtpage);
} else {
	kprintf("bsm cannot be unmapped");
}

free_bsm(virtpage);

restore(intr);
return OK;
}
