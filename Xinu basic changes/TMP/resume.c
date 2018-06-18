/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL resume(int pid)
{
	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[resumesyscall].name = "resume";
		processCallsInfo[currpid].systemCallsInfo[resumesyscall].count++;
		timerStart = ctr1000;
	}	

	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		return(SYSERR);
	}

	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[resumesyscall].totalTime += (ctr1000 - timerStart);
	}
	return(prio);
}
