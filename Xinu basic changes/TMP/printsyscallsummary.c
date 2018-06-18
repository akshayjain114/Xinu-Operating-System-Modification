#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

extern int traceSystemCalls;
extern struct ProcessCall processCallsInfo[NPROC];

const char * systemCallNames[] = {
    "freemem",
    "chprio",
    "getpid",
    "getprio",
    "gettime",
    "kill",
    "receive",
    "recvclr",
    "recvtim",
    "resume",
    "scount",
    "sdelete",
    "send",
    "setdev",
    "setnok",
    "screate",
    "signal",
    "signaln",
    "sleep",
    "sleep10",
    "sleep100",
    "sleep1000",
    "sreset",
    "stacktrace",
    "suspend",
    "unsleep",
    "wait"
};

void syscallsummary_start(){
	traceSystemCalls = 1;
}

void syscallsummary_stop(){
	traceSystemCalls = 0;
}

void printsyscallsummary(){
	int i;
	for(i=0;i<NPROC;i++){
		if(processCallsInfo[i].processCalledWhileTracing){
			kprintf("\nProcess [pid:%d]", i);
			int j;
			for(j=0;j<27;j++){
				if(processCallsInfo[i].systemCallsInfo[j].count){
					kprintf("\n\tSyscall: sys_%s, count: %d, average execution time: %d (ms)",
					systemCallNames[j], processCallsInfo[i].systemCallsInfo[j].count, 
					processCallsInfo[i].systemCallsInfo[j].totalTime/processCallsInfo[i].systemCallsInfo[j].count);
				}	
			}
		}		
	}
}