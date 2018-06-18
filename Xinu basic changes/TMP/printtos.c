#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

unsigned long *ebp;
unsigned long *esp;

void printtos(){
	asm("movl %esp, esp");	
	int a=1,b=2,c=3;
	asm("movl %ebp, ebp");
	kprintf("\nBefore[0x%08x]: 0x%08x\nAfter [0x%08x]: 0x%08x",
	ebp+2, *(ebp+2), ebp, *ebp);
	int numberOfContents = 0;
	int i = 0;
	for(;i<100;i++);
	while(esp < ebp && numberOfContents < 4){
		kprintf("\n\telement[0x%08x]: 0x%08x", esp, *esp);	
		esp = esp + 1;
		numberOfContents++;
	}	
}