/* screate.c - screate, newsem */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate()
{
	STATWORD ps;    
	int	lock;

	disable(ps);
	if ( (lock=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	locks[lock].lreadercnt = 0;
	locks[lock].lwritercnt = 0;
	locks[lock].currentprocesstype = READ;
	locks[lock].lstate = LUSED;
	locks[lock].lprio = 0;
	locks[lock].processes[0] = 0;
	locks[lock].processes[1] = 0;
	restore(ps);
	return(lock);
}

/*---------------------------------------------------------
 * newsem  --  allocate an unused lock and return its index
 *---------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lock;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locks[lock].lstate==LFREE) {
			locks[lock].lstate = LUSED;
			return(lock);
		}
	}
	return(SYSERR);
}
