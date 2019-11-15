/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


int bs_no;
int virt_base = 4096;
//char enum bs_attributes[] = {'bs_status', 'bs_pid', 'bs_vpno', 'bs_npages', 'bs_sem', 'bs_priv_heap'}attr;

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
STATWORD intr;
disable(intr);

// set the attributes of the backing store to default
for (bs_no = 0; bs_no < 8; bs_no++) {
	bsm_tab[bs_no] = (bs_map_t){BSM_UNMAPPED, -1, virt_base, 0, 0, 0, 1};
}

restore(intr);
return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
STATWORD intr;
disable(intr);

if (*avail > 7 || *avail < 0) {
	kprintf("Cannot allocate the backing store");
}
for (bs_no = 0; bs_no < 8; bs_no++) {
	if(bsm_tab[bs_no].bs_status == BSM_UNMAPPED && bsm_tab[bs_no].bs_ntpub == 0){
		if (bsm_tab[bs_no].bs_sem == 0) {
			*avail = bs_no;
			bsm_tab[bs_no].bs_status = BSM_MAPPED;
			restore(intr);
			return OK;		
		} else {
			break;
		}
	}
}
restore(intr);
return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
STATWORD intr;
disable(intr);

bsm_tab[i] = (bs_map_t){BSM_UNMAPPED, -1, virt_base, 0, 0, 0, 1};

restore(intr);
return OK;

}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
STATWORD intr;
disable(intr);

// fetch the backing store with the pid and return the vacant entry
if (vaddr < 0) {
	kprintf("Cannot find the requested backing store\n");
} else {
	int base = vaddr/NBPG;
	for (bs_no = 0; bs_no < 8; bs_no++) {
		if(bsm_tab[bs_no].bs_pid == pid){
			if (pid < 0) {
				break;
			}
			*store = bs_no;
			if (base - bsm_tab[bs_no].bs_vpno < 0) {
				break;
			} else {
				*pageth = base - bsm_tab[bs_no].bs_vpno;
			}
			restore(intr);
			return OK;
		}
	}
}
restore(intr);
return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
STATWORD intr;
disable(intr);

if (pid < 0 || vpno < 0 || source < 0 || source > 7) {
	kprintf("Cannot map the requested backing store\n");
}

bsm_tab[source] = (bs_map_t){BSM_MAPPED, pid, vpno, npages, 1, 0, 1, 1};		
proctab[pid].vhpno = vpno;
proctab[pid].store = source;
proctab[pid].vhpnpages = npages;

restore(intr);
return OK;
}


/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno)
{
STATWORD intr;
disable(intr);
int frm_no = 0, pageth;
unsigned long vaddr = vpno*NBPG;

while(frm_no < NFRAMES){
	if(frm_tab[frm_no].fr_pid == pid)
  		{
			if (pid < 0) {
				break;
			} else {
				if (frm_tab[frm_no].fr_type == FR_PAGE) {
					bsm_lookup(pid, vaddr, &bs_no, &pageth);
					if (bs_no < 0 || bs_no > 7) {
						break;
					}
					write_bs((frm_no + NFRAMES)*NBPG, bs_no, pageth);
				}
			} 
  		}
	frm_no++;
}
bsm_tab[bs_no] = (bs_map_t){BSM_UNMAPPED, -1, virt_base, 0, 0, 0, 1};
free_bsm(bs_no);

restore(intr);
return OK;
}


