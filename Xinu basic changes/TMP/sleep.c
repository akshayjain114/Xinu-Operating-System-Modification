/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL	sleep(int n)
{
	
	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[sleepsyscall].name = "sleep";
		processCallsInfo[currpid].systemCallsInfo[sleepsyscall].count++;
		timerStart = ctr1000;
	}
	
	STATWORD ps;    
	if (n<0 || clkruns==0)
		return(SYSERR);
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[sleepsyscall].totalTime += (ctr1000 - timerStart);
	}
	return(OK);
}
