/* wait.c - wait */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lock  --  make current process wait on a lock
 *------------------------------------------------------------------------
 */
void incrementReaderOrWriter();
extern unsigned long ctr1000;
SYSCALL	lock(int lockdescriptor, int locktype, int lockpriority)
{
	STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr = &proctab[currpid];
	
	disable(ps);
	if (isbadlock(lockdescriptor) || (lptr= &locks[lockdescriptor])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}

	/* If process is trying to wait on lock it earlier waited/associated on but the lock was deleted between the 2 waits*/
	if(pptr->pwaitret == DELETED && (pptr->plock == lockdescriptor || pptr->lockid == lockdescriptor || pptr->associatedlocks[lockdescriptor] == -1)){
		restore(ps);
		return(SYSERR);
	}

	pptr->plocktype = locktype;
	pptr->plockpriority = lockpriority;
	
	//No one is holding the lock | Or | A reader is holding the lock and the lock requested by is also a reader and reader is worth readying, so grant the lock immediately
	//A writer will only obtain the lock if there are no readers as well as writers holding the lock
	if( (lptr->lreadercnt == 0 && lptr->lwritercnt == 0) ||
	    	(lptr->currentprocesstype == READ && locktype == READ && isReaderWorthReadying(currpid, highestWaitingWriter(lockdescriptor)) ) 
            )
	{
		pptr->plocksheld[lockdescriptor/32] |= 1<<(lockdescriptor%32);
		lptr->currentprocesstype = locktype;
		pptr->lockid = -1;
		incrementReaderOrWriter(lockdescriptor, locktype);
		restore(ps);
		return(OK);
	}
	else{	//lock should not be granted
		if(locks[lockdescriptor].lprio < getprio(currpid))
			locks[lockdescriptor].lprio = getprio(currpid); //Changing to max priority
		updateAllPriorities(lockdescriptor, locks[lockdescriptor].lprio);
		int i;

		for(i=0; i<NPROC; i++){

			if( ( ( (&proctab[i])->plocksheld[lockdescriptor/32] & lockdescriptor%32) != 0) 
				&& ((&proctab[i])->pinh < locks[lockdescriptor].lprio ) 
			)
				(&proctab[i])->pinh = locks[lockdescriptor].lprio;

		}

		pptr->plock = lockdescriptor;
		#ifdef RTCLOCK
		pptr->plockwaittime = ctr1000;
		#endif
		pptr->pstate = PRWAIT;
		pptr->lockid = lockdescriptor;
		insert(currpid, lptr->lqhead, lockpriority);
		pptr->pwaitret = OK;
		resched();
		restore(ps);
		return pptr->pwaitret;
	}
}

void incrementReaderOrWriter(int lockdescriptor, int locktype){	
	struct lentry *lptr = &locks[lockdescriptor];
	if(locktype == READ)
		lptr->lreadercnt++;
	else
		lptr->lwritercnt++;
}