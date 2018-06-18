/* sdelete.c - sdelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
SYSCALL ldelete(int lock)
{
	STATWORD ps;    
	int	pid;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadlock(lock) || locks[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locks[lock];
	lptr->lstate = LFREE;
	if (nonempty(lptr->lqhead)) {
		while( (pid=getfirst(lptr->lqhead)) != EMPTY) /*dequeue and change pwaitret to DELETED*/
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	int i;
	for(i=0;i<NPROC;i++){
		if((&proctab[i])->associatedlocks[lock] == 1){
			(&proctab[i])->pwaitret = DELETED;
			(&proctab[i])->associatedlocks[lock] = -1;
		}
	}
	restore(ps);
	return(OK);
}
