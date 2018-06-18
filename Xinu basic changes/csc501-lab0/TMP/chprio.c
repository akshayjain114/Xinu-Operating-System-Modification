/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL chprio(int pid, int newprio)
{
	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[chpriosyscall].name = "chprio";
		processCallsInfo[currpid].systemCallsInfo[chpriosyscall].count++;
		timerStart = ctr1000;
	}
	
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[chpriosyscall].totalTime += (ctr1000 - timerStart);
	}
	return(newprio);
}
