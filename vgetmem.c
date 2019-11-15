/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

STATWORD intr;
disable(intr);
struct	mblock *head, *next, *extra;

if (proctab[currpid].vmemlist->mnext == NULL || nbytes <= 0) {
	kprintf("Cannot return the requested memory block");
}
nbytes = (unsigned int) roundmb(nbytes);
head = proctab[currpid].vmemlist;
next = proctab[currpid].vmemlist->mnext;

while (next != NULL) {
	if (head->mlen == nbytes) {
		next->mnext = head->mnext;
		restore(intr);
		return ((WORD*) next);
	} else if (next->mlen > nbytes) {
		extra = (unsigned)next + nbytes;
		head->mnext = extra;
		extra->mnext = next->mnext;
		extra->mlen = next->mlen - nbytes;
		restore(intr);
		return ((WORD*) next);
	} 
head = next;
next = next->mnext;

}
restore(intr);
return((WORD *)SYSERR);
}



