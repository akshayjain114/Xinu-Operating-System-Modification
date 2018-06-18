/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL	setdev(int pid, int dev1, int dev2)
{

	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[setdevsyscall].name = "setdev";
		processCallsInfo[currpid].systemCallsInfo[setdevsyscall].count++;
		timerStart = ctr1000;
	}


	short	*nxtdev;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[setdevsyscall].totalTime += (ctr1000 - timerStart);
	}
	return(OK);
}
