/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL getpid()
{
	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[getpidsyscall].name = "getpid";
		processCallsInfo[currpid].systemCallsInfo[getpidsyscall].count++;
		timerStart = ctr1000;
	}
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[getpidsyscall].totalTime += (ctr1000 - timerStart);
	}
	return(currpid);
}
