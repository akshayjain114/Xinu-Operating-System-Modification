#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	4

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}
	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 1024; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

void procA(char *msg, int lck){
	char *x; 
            char temp; 
            get_bs(8, 100); 
            kprintf("xmmap = %d\n", xmmap(7000, 8, 100));    /* This call simply creates an entry in the backing store mapping */ 
            x = 7000*4096;
            *x = 'Y';                            /* write into virtual memory, will create a fault and system should proceed as in the prev example */ 
            temp = *x;                        /* read back and check */ 
	    kprintf("Process A: %c\n", temp);
	    sleep(20);
            xmunmap(7000);
}

void procB(char *msg, int lck){
	char *x; 
            char temp_b; 
            kprintf("xmmap = %d\n", xmmap(6000, 8, 50)); 
            x = 6000 * 4096;
            temp_b = *x;
	    kprintf("Page fault should have occured\n");
	    kprintf("Process B: %c\n", temp_b);
	    xmunmap(6000);
}

void procC(char * msg, int lck){
	int *x, *y, *z,  *a;
	int temp1, temp2;
	x = vgetmem(1000);
	*(x) = 10;
	*(x+1) = 10;
	kprintf("x=%u\n", x);
	y = vgetmem(1000);
	*(y) = 10;
	*(y+1) = 10;

	kprintf("y=%u\n", y);
	z = vgetmem(1000);
	int b;
	for(b=0;b<1000/4;b++){
		*(z+b) = 10;
	}

	kprintf("z=%u\n", z);
	a = vgetmem(1000);
	*(a) = 10;
	*(a+1) = 10;

	kprintf("a=%u\n", a);
	kprintf("Current next block 1 = %u\n", (&proctab[currpid])->vmemlist->mnext);
	vfreemem(z, 1000);
	kprintf("Current next block 2 = %u\n", (&proctab[currpid])->vmemlist->mnext);
	kprintf("Current next block 2 len = %u\n", (&proctab[currpid])->vmemlist->mnext->mlen);

	kprintf("Current next next block 2 = %u\n", (&proctab[currpid])->vmemlist->mnext->mnext);
	kprintf("Current next next block 2 len = %u\n", (&proctab[currpid])->vmemlist->mnext->mnext->mlen);
	//vfreemem(y, 1000);
	//vfreemem(x, 1000);
	z = vgetmem(1000);
	kprintf("z=%u\n", z);
	kprintf("Ended process C\n");
}

int main() {
	srpolicy(SC);
	int pid1;
	int pid2;

	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);

	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);

	//kprintf("\n3: Frame test\n");
	//pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	//resume(pid1);
	//sleep(3);

	kprintf("\n3. Write backing store check\n");

	pid1 = create(procA, 20000, 20, "process A", 0, NULL);
	resume(pid1);
	sleep(10);

	pid1 = create(procB, 20000, 20, "process B", 0, NULL);
	resume(pid1);
	sleep(10);
	
	kprintf("\n vgetmem check\n");
	pid1 = vcreate(procC, 2000, 100, 20, "proc C", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(10);

}