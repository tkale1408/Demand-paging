/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;		/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;		/* arguments (treated like an array in the code)*/
{

STATWORD intr;
disable(intr);

int proc_id, store;
proc_id = create(procaddr, ssize, priority, name, nargs, args);

get_bsm(&store);
bsm_map(proc_id, 4096, store, hsize);

proctab[proc_id].store = store;
proctab[proc_id].vhpno = 4096;
proctab[proc_id].vhpnpages = hsize;
proctab[proc_id].vmemlist->mnext = 4096 * NBPG;
struct mblock *base = (store * BACKING_STORE_UNIT_SIZE) + BACKING_STORE_BASE;
(*base).mlen = hsize * NBPG; (*base).mnext = NULL;

restore(intr);
return proc_id;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
