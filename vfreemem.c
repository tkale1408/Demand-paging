/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{

STATWORD intr;
disable(intr);

struct mblock *head, *next;

head = proctab[currpid].vmemlist;
next = proctab[currpid].vmemlist->mnext;

if (block < 4096*NBPG || currpid < 0 || block < head->mlen || size <= 0 ) {
	kprintf ("Cannot freed the memory block with the size given");
}

while (next < block) {
if (next == NULL) {
	break;
}
head = next;
next = next->mnext;
}

unsigned long mblock_size = next->mlen + (unsigned)block;

if (mblock_size < next-> mlen && head != next) {
 	kprintf ("Block size is not appropriate");
} else if (mblock_size < next->mlen && head == NULL) {
	kprintf ("Cannot freed the memory block with the size given");
}

if (head == proctab[currpid].vmemlist && mblock_size != (unsigned)block) {
	head->mnext = block;
	head->mnext->mnext = next;
	head = block;
} else {
	head->mlen = head->mlen + size;
}
if (head->mlen + (unsigned)head == (unsigned)next) {
	head->mlen = head->mlen + next->mlen;
}

restore(intr);
return(OK);
}
