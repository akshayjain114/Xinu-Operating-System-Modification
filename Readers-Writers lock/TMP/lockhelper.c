#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include<proc.h>
#include <q.h>

extern unsigned long ctr1000;

int highestWaitingWriter(int lockdescriptor){
	struct	lentry	*lptr = &locks[lockdescriptor];
	struct	pentry *pptr;
	int max = -1000;
	int first = q[lptr->lqhead].qnext;
	int highestWriter = -1;
	while(first != EMPTY && first<NPROC){
		pptr = &proctab[first];
		if(pptr->plocktype == WRITE && pptr->plockpriority >= max && (pptr->pwaitret == OK)){
			max = pptr->plockpriority;
			highestWriter = first;
		}
		first = q[first].qnext;
	}
	return highestWriter;
}

int isReaderWorthReadying(int pid, int highestWriterPid){
	
	if(highestWriterPid == -1){ //No writer is waiting on the lock. So ready the reader.
		return 1;
	}
	
	struct pentry *reader = &proctab[pid];
	struct	pentry	*writer = &proctab[highestWriterPid];
	unsigned long currentTime = ctr1000;
	unsigned long readerWaitTime = currentTime - reader->plockwaittime;
	unsigned long writerWaitTime = currentTime - writer->plockwaittime;

	if( reader->plockpriority > writer->plockpriority || 
            (reader->plockpriority == writer->plockpriority && readerWaitTime < (writerWaitTime + 500)) 
          ){
		return 1;
	}
	else{
		return 0;
	}

}

int checkWriteOrReadNext(int lockdescriptor){
	int highestWriterPid = highestWaitingWriter(lockdescriptor);
	if(highestWriterPid == -1){
		int waitingprocess = q[locks[lockdescriptor].lqhead].qnext;
		return READ;
	}
	
	struct lentry *lptr = &locks[lockdescriptor];
	struct pentry *pptr;

	int first = q[lptr->lqhead].qnext;
	
	while( first > 0 && first < NPROC){	
		pptr = &proctab[first];
		if(pptr->plocktype == READ && isReaderWorthReadying(first, highestWriterPid))
			return READ;
		first = q[first].qnext;
	}

	return WRITE;
		
}

void updateAllPriorities(int lockdescriptor, int lprio){
	int i;
	for(i=0; i<NPROC; i++){
		if( ((proctab[i].plocksheld[lockdescriptor/32] & (1<<(lockdescriptor%32)) ) != 0)
			&& ( getprio(i) < locks[lockdescriptor].lprio)){
			proctab[i].pinh = lprio;
			locks[lockdescriptor].lprio = lprio;
			if(proctab[i].lockid != -1){
				forceUpdateAllPriorities(proctab[i].lockid, lprio);
			}
		}
	}
}

void forceUpdateAllPriorities(int lockdescriptor, int lprio){
	int i;
	for(i=0; i<NPROC; i++){
		if( ((proctab[i].plocksheld[lockdescriptor/32] & (1<<(lockdescriptor%32)) ) != 0) ){
			proctab[i].pinh = 0;
			if(proctab[i].pprio < lprio)
				proctab[i].pinh = lprio;
			locks[lockdescriptor].lprio = lprio;
			if(proctab[i].lockid != -1){
				forceUpdateAllPriorities(proctab[i].lockid, lprio);
			}
		}
	}
}


void pupdateAllPriorities(int pid, int prioritytobechecked, int isalive){
	if((&proctab[pid])->lockid != -1 ){
		int lock = (&proctab[pid])->lockid;
		if( isalive || locks[lock].lprio == prioritytobechecked ){
			locks[lock].lprio = 0;
			int first = q[locks[lock].lqhead].qnext;
			while(first>0 && first<NPROC){
				if( (isalive || first != pid) && getprio(first) > locks[lock].lprio ){
					locks[lock].lprio = getprio(first);
				}
				first = q[first].qnext;
			}
			int p;
			for(p=0; p < NPROC;p++){
				if ( (proctab[p].plocksheld[lock/32] & (1<<(lock%32))) != 0 ){
					if(proctab[p].pprio < locks[lock].lprio){
						proctab[p].pinh = locks[lock].lprio;
					}
					else{
						proctab[p].pinh = 0;
					}
					pupdateAllPriorities(p, prioritytobechecked, isalive);
				}
			}
		}
		
	}
}
