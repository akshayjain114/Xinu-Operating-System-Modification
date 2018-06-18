/* signal.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------------------------
 * signal  --  release a lock, unblocking the relevant process with highest priority on lock
 *------------------------------------------------------------------------------------------
 */
void decrementReaderOrWriter(int lockdescriptor);
SYSCALL releaseall(numlocks, lockdescriptors)
	int numlocks;		/*number of locks to be released	*/
	int lockdescriptors;	/*arguments all lock descriptors	*/
{
	STATWORD ps;    
	disable(ps);
	register struct	lentry	*lptr;

	int	*a;		/* points to list of args	*/
	a = (int*)(&lockdescriptors) + (numlocks-1);
	int lockdescs[numlocks];
	int i;
	for(i = numlocks-1;i>=0;i--){
		lockdescs[i] = *(a--);
	}

	int badlock = 0;

	for(i = 0; i < numlocks; i++){
		int lock = lockdescs[i];
		if(isbadlock(lock))
			badlock = 1;
		lptr = &locks[lock];
		if(lptr->lstate==LFREE || ((&proctab[currpid])->plocksheld[lock/32] & (1<<(lock%32))) == 0){
			badlock = 1;
		}
		else{
			(&proctab[currpid])->plocksheld[lock/32] &= ~(1<<(lock%32));
			int l;
			(&proctab[currpid])->pinh = 0;
			for(l=0; l<NLOCKS;l++){
				if( ((&proctab[currpid])->plocksheld[l/32] & (1<<(l%32))) != 0 ){
					if(locks[l].lprio > getprio(currpid))
						(&proctab[currpid])->pinh = locks[l].lprio;
				}
			}			

			decrementReaderOrWriter(lock);

			//Default lock type
			lptr->currentprocesstype = READ;
			
			if(lptr->lreadercnt == 0 && lptr->lwritercnt == 0 && nonempty(lptr->lqhead) ){
				if(checkWriteOrReadNext(lock) == READ){
					//Ready all readers with higher priority than highest writer

					lptr->currentprocesstype = READ;
					int previous = lptr->lqhead;
					int waitingprocess = q[lptr->lqhead].qnext;
					register struct pentry *pptr;
					
					int highestWriterPid = highestWaitingWriter(lock);				

					while(waitingprocess > 0 && waitingprocess < NPROC){
						pptr = &proctab[waitingprocess];
						int prev = -1;
						if(pptr->plocktype == READ && isReaderWorthReadying(waitingprocess, highestWriterPid)){
							q[previous].qnext = q[waitingprocess].qnext;
							pptr->plocksheld[lock/32] |= 1<<(lock%32);
							lptr->lreadercnt++;
							(&proctab[waitingprocess])->lockid = -1;
							(&proctab[currpid])->pinh = 0;
							ready(waitingprocess, RESCHNO);
						}
						else{
							previous = waitingprocess;
						}
						waitingprocess = q[waitingprocess].qnext;
					}
				}
				else if (checkWriteOrReadNext(lock) == WRITE){
					int nextpid = highestWaitingWriter(lock);
					lptr->currentprocesstype = WRITE;
					(&proctab[nextpid])->plocksheld[lock/32] |= 1<<(lock%32);
					lptr->lwritercnt++;

					//dequeue
					int first = lptr->lqhead;
					while(q[first].qnext != nextpid)
						first = q[first].qnext;
					q[first].qnext = q[q[first].qnext].qnext;
					(&proctab[currpid])->lockid = -1;
					(&proctab[currpid])->pinh = 0;
					ready(nextpid, RESCHNO);
				}
			}

		}
	}
	resched();
	if (badlock) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(OK);
}

void decrementReaderOrWriter(int lockdescriptor){	
	struct lentry *lptr = &locks[lockdescriptor];
	if(lptr->currentprocesstype == READ)
		lptr->lreadercnt--;
	else
		lptr->lwritercnt--;
}