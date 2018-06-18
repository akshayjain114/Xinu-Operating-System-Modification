#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

unsigned long *esp;

void printprocstks(int priority){
	int i = 0;
	for(;i<NPROC;i++){
		struct pentry *proc = &proctab[i];
		if(proc->pstate != PRFREE && proc->pprio > priority){
			
			if(i==currpid){
				asm("movl %esp,esp");
			}
			else {
				esp = (unsigned long *)proc->pesp;
			}
			kprintf("\nProcess [proc %s]\n\tpid: %d\n\tpriority: %d\n\tbase: 0x%08x\n\tlimit: 0x%08x\n\tlen: %d\n\tpointer: 0x %08x",
	 		proc->pname, i, proc->pprio,
			proc->pbase, proc->plimit, proc->pstklen, esp);
		}
	}
}