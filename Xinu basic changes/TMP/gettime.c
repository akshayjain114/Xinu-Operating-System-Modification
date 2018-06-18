/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <date.h>
#include <lab0.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL	gettime(long *timvar)
{

	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[gettimesyscall].name = "gettime";
		processCallsInfo[currpid].systemCallsInfo[gettimesyscall].count++;
		timerStart = ctr1000;
	}

    /* long	now; */

	/* FIXME -- no getutim */
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[gettimesyscall].totalTime += (ctr1000 - timerStart);
	}
    return OK;
}
