/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <lab1.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
extern int epoch;
extern int rrtimer;
static int epochCounter = 0;
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	if(getschedclass() == RANDOMSCHED){
		register struct	pentry	*optr;	/* pointer to old process entry */
		register struct	pentry	*nptr;	/* pointer to new process entry */
		
		optr= &proctab[currpid];
		/* force context switch */
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/*Implementing Random scheduler*/
		int sum = 0;
		int first = q[q[rdyhead].qnext].qnext;
		int counter = 0;
		while(first < NPROC && first != EMPTY){
			sum += q[first].qkey;
			first = q[first].qnext;
		}

		int repid = 0; //Default null process should run
		if(sum > 0){
			int randPrio = rand() % sum;
			int last = q[rdytail].qprev;
			while(randPrio > q[last].qkey){
				randPrio -= q[last].qkey;
				last = q[last].qprev;
			}
			if(proctab[last].pstate == PRREADY){
				repid = last;
				dequeue(repid);
			}
		}
		currpid = repid;
		nptr = &proctab[currpid];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
		
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		
		/* The OLD process returns here when resumed. */
		return OK;
	}
	else if(getschedclass() == LINUXSCHED){

		//If epoch is running, decrement quantum for current process and total epoch value
		//Don't do this for the first time we are working with epoch, or when epoch is over.
		if(epoch > 0){
			linuxprocesses[currpid].quantum-= (epochCounter - preempt);
			epoch-= (epochCounter - preempt);
		}
		
		register struct	pentry	*optr;	/* pointer to old process entry */
		register struct	pentry	*nptr;	/* pointer to new process entry */
		
		/* force context switch */
		optr= &proctab[currpid];
		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/*Check if any runnable process has quantum left*/
		int first = q[q[rdyhead].qnext].qnext;
		int allQuantumZero = 1;
		while(first < NPROC && first != EMPTY){
			if(linuxprocesses[first].quantum != 0){
				allQuantumZero = 0;
				break;
			}
			first = q[first].qnext;
		}

		//Start epoch if total quantum was used up or if no runnable process had non-zero quantum
		if(epoch <= 0 || allQuantumZero){

			//Quantum and priority calculations for ready processes
			first = q[q[rdyhead].qnext].qnext; //Skipping the null process
			while(first < NPROC && first != EMPTY){
				linuxprocesses[first].priority = proctab[first].pprio;
				linuxprocesses[first].quantum = linuxprocesses[first].quantum/2 + linuxprocesses[first].priority;
				epoch += linuxprocesses[first].quantum;
				first = q[first].qnext;
			}

			//Quantum and priority calculations for blocked processes
			first = q[clockq].qnext;
			while(first < NPROC && first != EMPTY){
				linuxprocesses[first].priority = proctab[first].pprio;
				linuxprocesses[first].quantum = linuxprocesses[first].quantum/2 + linuxprocesses[first].priority;
				epoch += linuxprocesses[first].quantum;
				first = q[first].qnext;
			}		
		}

		int maxGoodness = 0;
		int leastTimer = 1000;
		int pidWithMaxGoodness = 0; //Default NULL process' id

		//choose a runnable process with max goodness
		first = q[q[rdyhead].qnext].qnext;
		while(first < NPROC && first != EMPTY){
			if(linuxprocesses[first].priority > -1 && linuxprocesses[first].quantum > 0){
				int processGoodness = linuxprocesses[first].quantum + linuxprocesses[first].priority;
				int processTimer = linuxprocesses[first].lastRRTimer;
				if(processGoodness > maxGoodness){
					maxGoodness = processGoodness;
					pidWithMaxGoodness = first;
					leastTimer = processTimer;
				}
				else if(processGoodness == maxGoodness && processTimer < leastTimer){
					//Handling round-robin here
					maxGoodness = processGoodness;
					pidWithMaxGoodness = first;
					leastTimer = processTimer;
				}
			}
			first = q[first].qnext;
		}

		dequeue(pidWithMaxGoodness);
		
		//Once process is chosen, switch context
		currpid = pidWithMaxGoodness;
		linuxprocesses[currpid].lastRRTimer = rrtimer;
		rrtimer++;
		nptr = &proctab[currpid];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		
		//If null process is chosen, set preempt as QUANTUM
		//Ideally this can be set to inifinity since resched will be called when a blocked process becomes ready
		//but being safe.
		if(currpid == 0){
			#ifdef	RTCLOCK
				preempt = QUANTUM;		/* reset preemption counter	*/
			#endif
		}
		else{
			preempt = linuxprocesses[currpid].quantum;
		}
		epochCounter = preempt;

		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

		/* The OLD process returns here when resumed. */
		return OK;
	}
	else{
		register struct	pentry	*optr;	/* pointer to old process entry */
		register struct	pentry	*nptr;	/* pointer to new process entry */

		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
			  (lastkey(rdytail)<optr->pprio)) {
				return(OK);
		}
			
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
			
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		epochCounter = preempt;	
	
		/* The OLD process returns here when resumed. */
		return OK;
	}
}