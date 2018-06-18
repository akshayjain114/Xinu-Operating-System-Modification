/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);
	unsigned long a = read_cr2();

	//Check that virtual page number is mapped
	if( !mapped(a, currpid) ){
		kprintf("Not a valid virtual address. Killing the process.\n");
		kill(currpid);
		restore(ps);
		return SYSERR;
	}

	unsigned int vp = (a & 0xfffff000) >> 12;

	unsigned int *pd = (&proctab[currpid])->pdbr;
	
	//Check that a is a legal address
	
	int pagetableindex = (a & 0xFFC00000) >> 22;
	int pageindex = (a & 0x003FF000) >> 12;
	int offset = (a & 0x00000fff);

	//kprintf("a = %u\n", a);
	//kprintf("vp = %u\n", vp);
	//kprintf("pd = %u\n", pd);
	//kprintf("pagetableindex = %u\n", pagetableindex);
	//kprintf("pageindex = %u\n", pageindex);
	//kprintf("offset = %u\n", offset);

	int pdentryvalue = *(pd + pagetableindex);
	if( (pdentryvalue & 1) != 0){
		//kprintf("Page table is valid\n");
		unsigned int *pagetablestart = pdentryvalue & 0xfffff000;
		int ptentryvalue = *(pagetablestart + pageindex);
		//kprintf("ptentryvalue2 = %d\n", ptentryvalue);
		if( (ptentryvalue & 1) != 0){
			//kprintf("In if\n");
			//shutdown();
		}
		else{
			//kprintf("In else\n");
			//shutdown();
			initialize_page(pagetablestart, pageindex, currpid, vp);
			ptentryvalue = *(pagetablestart + pageindex);

			//Increasing reference count for page table
			int frame_no = ((unsigned int)(pagetablestart)) / NBPG - FRAME0;
			(&frm_tab[frame_no])->fr_refcnt++;

			unsigned int *pagestart = ptentryvalue & 0xfffff000;
			copy_page(pagestart, currpid, vp);
		}
	}
	else{ //page table is not valid
		//kprintf("Page table not valid\n");
		//kprintf("currpid = %d\n", currpid);
		initialize_pagetable((&proctab[currpid])->pdbr, pagetableindex, currpid);
		pdentryvalue = *(pd + pagetableindex);
		//kprintf("pdentryvalue = %u\n", pdentryvalue);
		unsigned int *pagetablestart = pdentryvalue & 0xfffff000;
		initialize_page(pagetablestart, pageindex, currpid, vp);
		int ptentryvalue = *(pagetablestart + pageindex);
		
		//Increasing reference count for page table
		int frame_no = ((unsigned int)(pagetablestart)) / NBPG - FRAME0;
		(&frm_tab[frame_no])->fr_refcnt++;

		//kprintf("ptentryvalue = %u\n", ptentryvalue);
		unsigned int *pagestart = ptentryvalue & 0xfffff000;
		copy_page(pagestart, currpid, vp);
	}
	restore(ps);
	return OK;
}


