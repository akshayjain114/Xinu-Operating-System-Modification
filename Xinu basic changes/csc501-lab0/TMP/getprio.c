/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL getprio(int pid)
{	
	
	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[getpriosyscall].name = "getprio";
		processCallsInfo[currpid].systemCallsInfo[getpriosyscall].count++;
		timerStart = ctr1000;
	}	

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[getpriosyscall].totalTime += (ctr1000 - timerStart);
	}
	return(pptr->pprio);
}
