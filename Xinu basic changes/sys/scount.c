/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */

extern unsigned long ctr1000;
extern int traceSystemCalls;

SYSCALL scount(int sem)
{

	unsigned long timerStart;
	if(traceSystemCalls){
		processCallsInfo[currpid].processCalledWhileTracing = 1;
		processCallsInfo[currpid].systemCallsInfo[scountsyscall].name = "scount";
		processCallsInfo[currpid].systemCallsInfo[scountsyscall].count++;
		timerStart = ctr1000;
	}

extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);
	if(traceSystemCalls){
		processCallsInfo[currpid].systemCallsInfo[scountsyscall].totalTime += (ctr1000 - timerStart);
	}
	return(semaph[sem].semcnt);
}
