/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int page_replace_policy;
int create_page_table()
{
int frame = 0, frno;

get_frm(&frno);
unsigned int frame_addr = (FRAME0 + frno) * NBPG;
pt_t *pg_table = (pt_t*)frame_addr;

while(frame < NFRAMES){
	pg_table[frame] = (pt_t) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};		
	frame++;
}
	return frno;
}

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
STATWORD intr;
disable(intr);
 
int new_frame, pg_table, source, pg_offset;
unsigned long v_addr = read_cr2();
virt_addr_t *virt_addr = (virt_addr_t*)&v_addr;
unsigned int virt_pt_offset = virt_addr->pt_offset;
unsigned int virt_pd_offset = virt_addr->pd_offset;
unsigned long  pdbr = proctab[currpid].pdbr;
pd_t *pd_entry = pdbr + virt_pd_offset * sizeof(pd_t);

if (pd_entry->pd_pres == 0) {
	pg_table = create_page_table();
	(*pd_entry).pd_pres = (*pd_entry).pd_write = 1;
	(*pd_entry).pd_user = (*pd_entry).pd_pwt = (*pd_entry).pd_pcd = (*pd_entry).pd_acc = (*pd_entry).pd_mbz = (*pd_entry).pd_fmb = (*pd_entry).pd_global = 0;
	(*pd_entry).pd_base = FRAME0 + pg_table;
	frm_tab[pg_table] = (fr_map_t) {FRM_MAPPED, currpid, frm_tab[pg_table].fr_vpno, frm_tab[pg_table].fr_refcnt, FR_TBL, frm_tab[pg_table].fr_dirty};	
}
pt_t *pt_entry = (pt_t*)(pd_entry->pd_base*NBPG + virt_pt_offset*sizeof(pt_t));
if (pt_entry->pt_pres == 0) {
	get_frm(&new_frame);
	(*pt_entry).pt_pres = (*pt_entry).pt_write = 1;
	pt_entry->pt_base = (FRAME0 + new_frame);
	frm_tab[pd_entry->pd_base - FRAME0].fr_refcnt++;
	frm_tab[new_frame] = (fr_map_t) {FRM_MAPPED, currpid, v_addr/NBPG, frm_tab[pg_table].fr_refcnt, FR_PAGE, frm_tab[pg_table].fr_dirty};
	bsm_lookup(currpid, v_addr, &source, &pg_offset);
	read_bs((char*)((FRAME0+new_frame)*NBPG), source, pg_offset);
	insert_frame(new_frame);
}
write_cr3(pdbr);
restore(intr);
return OK;
}