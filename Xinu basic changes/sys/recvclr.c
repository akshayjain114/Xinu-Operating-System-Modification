/* recvclr.c - recvclr */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  recvclr  --  clear messages, returning waiting message (if any)
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL	recvclr()
{

	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[recvclrsyscall].name = "recvclr";
		processCallsInfo[currpid].systemCallsInfo[recvclrsyscall].count++;
		timerStart = ctr1000;
	}

	STATWORD ps;    
	WORD	msg;

	disable(ps);
	if (proctab[currpid].phasmsg) {
		proctab[currpid].phasmsg = 0;
		msg = proctab[currpid].pmsg;
	} else
		msg = OK;
	restore(ps);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[recvclrsyscall].totalTime += (ctr1000 - timerStart);
	}
	return(msg);
}
