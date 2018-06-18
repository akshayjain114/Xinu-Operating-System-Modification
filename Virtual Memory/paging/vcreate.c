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
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD		PS;
	if(hsize == NULL || hsize == 0){
		return SYSERR;
	}
	int newpid = create(procaddr, ssize, priority, name, nargs, args);
	disable(PS);
	int mbs_id;
	int *bs_id = &mbs_id;
	if( get_bsm(bs_id) == SYSERR){
		kill(newpid);
		return SYSERR;
	}
	if( get_bs(*bs_id, hsize) == SYSERR){
		kill(newpid);
		return SYSERR;
	}
	if( bsm_map(newpid, VIRTUALPAGE0, *bs_id, hsize, BSM_PRIVATE) == SYSERR ){
		kill(newpid);
		return SYSERR;
	}
	
	(&proctab[newpid])->vmemlist = &(&proctab[newpid])->memorylist;
	//initializing virtual memory list
	(&proctab[newpid])->vhpno = VIRTUALPAGE0;
	(&proctab[newpid])->vhpnpages = hsize;
	(&proctab[currpid])->vmemlist->mnext == NULL;
	(&proctab[currpid])->virtualMemoryCreated == 0;

	restore(PS);
	return(newpid);
}