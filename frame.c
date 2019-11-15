/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int pg_replace;
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
STATWORD intr;
disable(intr);
int frno = 0;

while(frno < NFRAMES){
	frm_tab[frno] = (fr_map_t){FRM_UNMAPPED, -1, 0, 0, FR_PAGE, 0, 0, -1};
	frno++;
}

restore(intr);
return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{

STATWORD intr;
disable(intr);
int frno = 0;
int frame_number = -1;

while(frno < NFRAMES){
	if(frm_tab[frno].fr_status == FRM_UNMAPPED && frm_tab[frno].fr_dirty == 0){
		*avail = frno;
		restore(intr);
		return OK;
	}
	frno++;
}
// if there is no frame left unmapped then we have to remove one of the frames back to the backing store.
frame_number = remove_frame(pg_replace);
//if the page replacement policies return any frame then assign it as available.
if (frame_number >= 0) {
	*avail = frame_number;
	restore(intr);
	return OK;
} 
restore(intr);
return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
STATWORD intr;
disable(intr);

if (i >= 0 && frm_tab[i].fr_type == FR_PAGE) {
	int frame_tab_pid = frm_tab[i].fr_pid;
	unsigned long v_addr = frm_tab[i].fr_vpno;
	unsigned long pdbr = proctab[frame_tab_pid].pdbr;
	virt_addr_t *virt_addr = (virt_addr_t*)&v_addr;
	unsigned int pt_offset = virt_addr->pt_offset;
	unsigned int pd_offset = virt_addr->pd_offset;
	pd_t *pd_entry = pdbr + (pd_offset*sizeof(pd_t));
	pt_t *pt_entry = (pd_entry->pd_base*NBPG) + (pt_offset*sizeof(pt_t));
	int store = proctab[frm_tab[i].fr_pid].store;
	int page_num = frm_tab[i].fr_vpno - proctab[frame_tab_pid].vhpno;
	if(i < FR_PAGE){
		kprintf ("Invalid frame number given.\n");
		restore (intr);
		return SYSERR;
	}
	write_bs((i+FRAME0)*NBPG, store, page_num);
	pt_entry->pt_pres = 0;
	frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt -= 1;
	int new_vaddr = pd_entry->pd_base-FRAME0;
	frm_tab[new_vaddr] = (fr_map_t) {FRM_UNMAPPED, -1, 4096, frm_tab[new_vaddr].fr_refcnt, FR_PAGE, 0, frm_tab[new_vaddr].fr_age, frm_tab[new_vaddr].fr_nextframe};
}
restore(intr);
return OK;
}

int remove_frame(int pg_replace) {
int current = 0, prev = -1, print_frame_number = 0;
for (current = 0, prev = -1; current > -1; prev = current, current = frm_tab[current].fr_nextframe) {
	unsigned long v_addr = frm_tab[current].fr_vpno;
	virt_addr_t *virt_addr = (virt_addr_t*)&v_addr;
	unsigned int virt_pt_offset = virt_addr->pt_offset;
	unsigned int virt_pd_offset = virt_addr->pd_offset;
	unsigned long pdbr = proctab[currpid].pdbr;
	pd_t *pd_entry = pdbr + virt_pd_offset * sizeof(pd_t);
	pt_t *pt_entry = (pt_t*)(pd_entry->pd_base*NBPG + virt_pt_offset*sizeof(pt_t));
	int frame_number = queue_head;

	if (pg_replace == SC) {
		if(pt_entry->pt_acc == 0){
			if(prev == -1){
				queue_head = frm_tab[current].fr_nextframe;
				frm_tab[current].fr_nextframe = -1;
				print_frame_number = frame_number;
			} else{
				frm_tab[prev].fr_nextframe = frm_tab[current].fr_nextframe;
				frm_tab[current].fr_nextframe = -1;
				print_frame_number = frame_number;
			}
		} else{
			pt_entry->pt_acc = 0;
			queue_head = frm_tab[current].fr_nextframe;
			frm_tab[current].fr_nextframe = -1;
			print_frame_number = frame_number;
		}
	} else {
		frm_tab[current].fr_age = frm_tab[current].fr_age/2;
		if(pt_entry->pt_acc == 1){
			int temp = frm_tab[current].fr_age + 128;
			if(temp < 255) {
				frm_tab[current].fr_age = temp;
			} else {
				frm_tab[current].fr_age = 255;
			}
		}
		if(frm_tab[current].fr_age < frm_tab[frame_number].fr_age) {
			frame_number = current;
			prev = current;
			current = frm_tab[current].fr_nextframe;		
		}
		print_frame_number = frame_number;
	}
}
if(debugging_mode == 1) {
	kprintf("Removing frame %d\n", print_frame_number );
}
if (print_frame_number >= 0) {
	free_frm (print_frame_number);
}
return print_frame_number;
}

void insert_frame(int frame_num)
{
int next = -1;
int current = -1;
if (queue_head == -1) {
	queue_head = frame_num;
	current = frame_num;
} else {
	current = queue_head;
	next = frm_tab[current].fr_nextframe;
	while (next != -1) {
		current = next;
		next = frm_tab[next].fr_nextframe;
	}
	frm_tab[current].fr_nextframe = frame_num;
	frm_tab[frame_num].fr_nextframe = -1;
}
}