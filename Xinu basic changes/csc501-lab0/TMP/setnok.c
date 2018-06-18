/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL	setnok(int nok, int pid)
{

	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[setnoksyscall].name = "setnok";
		processCallsInfo[currpid].systemCallsInfo[setnoksyscall].count++;
		timerStart = ctr1000;
	}

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[setnoksyscall].totalTime += (ctr1000 - timerStart);
	}
	return(OK);
}
