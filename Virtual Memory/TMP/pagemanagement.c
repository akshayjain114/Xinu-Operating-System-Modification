/* pagemanagement.c - manage page directories */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * initialize_directory - initialize a directory
 *-------------------------------------------------------------------------
 */
SYSCALL initialize_directory(unsigned int *pdbr, int pid)
{	
	int mf;
	int *f = &mf;
	get_pagedirfrm(f, pid);

	unsigned int *startAddress = (*f+FRAME0) * NBPG;
	unsigned int *endAddress = (*f+FRAME0) * NBPG + NBPG;
	*pdbr = startAddress;
	
	while(startAddress < endAddress ){
		*startAddress = 0x00000000;
		startAddress++;
	}

	return OK;
}

/*-------------------------------------------------------------------------
 * initialize_pagetable - initialize a page table
 *-------------------------------------------------------------------------
 */
SYSCALL initialize_pagetable(unsigned int pdbrValue, int pageTableIndex, int pid, int vpno)
{	
	int mf;
	int *f = &mf;
	get_pagetblfrm(f, pid, vpno);

	unsigned int *pageDirectoryEntryAddress = pdbrValue + pageTableIndex*sizeof(unsigned int);
	*pageDirectoryEntryAddress = (*f+FRAME0) * NBPG;
	*pageDirectoryEntryAddress = *pageDirectoryEntryAddress | 3;

	unsigned int *startAddress = (*f+FRAME0) * NBPG;
	unsigned int *endAddress = (*f+FRAME0) * NBPG + NBPG;
	
	while(startAddress < endAddress ){
		*startAddress = 0x00000000;
		startAddress++;
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * initialize_page - initialize a page
 *-------------------------------------------------------------------------
 */
SYSCALL initialize_page(unsigned int *pageTableStart, int pageIndex, int pid, int vpno)
{	
	int mf;
	int *f = &mf;	
	get_pagefrm(f, pid, vpno);

	unsigned int *pageTableEntryAddress = pageTableStart + pageIndex;
	*pageTableEntryAddress = (*f+FRAME0) * NBPG;
	*pageTableEntryAddress = *pageTableEntryAddress | 3;
	
	return OK;
}

/*-------------------------------------------------------------------------
 * initialize_page - initialize a page
 *-------------------------------------------------------------------------
 */
SYSCALL copy_page(unsigned int *pageStartAddress, int pid, int vpno)
{
	//kprintf("Copy page: pid = %d, vpno = %d\n", pid, vpno);
	int mstore, mpage;
	unsigned int *store = &mstore;
	unsigned int *page = &mpage;
	bsm_lookup(pid, read_cr2(), store, page);
	read_bs(pageStartAddress, *store, *page);
}

/*------------------------------------------------------------------------
 *  initialize_global_page_tables --  initialize the 4 global page tables
 *------------------------------------------------------------------------
 */
initialize_global_page_tables(unsigned int *directoryStart)
{
	unsigned int *startPageAddress = 0;
	int i;
	for(i=0;i<4;i++){
		unsigned int mf;
		unsigned int *f = &mf;
		get_pagetblfrm(f);
		unsigned int *startFrameAddress = (*f+FRAME0) * NBPG;
		unsigned int *endFrameAddress = (*f+FRAME0) * NBPG + NBPG;

		*(directoryStart+ i) = (int)startFrameAddress | 0x00000003;

		while(startFrameAddress < endFrameAddress){
			*startFrameAddress = ((int)startPageAddress)|0x00000003;
			startFrameAddress++;
			startPageAddress += (NBPG / sizeof(unsigned int) );
		}
	}
	
}

/*---------------------------------------------------------------------------------------
 *  copy_global_page_tables --  copy the 4 global page tables addresses from null process
 *---------------------------------------------------------------------------------------
 */
SYSCALL copy_global_page_tables(unsigned int *directoryStart)
{
	unsigned int *nullProcessDirectoryStart = (&proctab[0])->pdbr;
	int i;
	for(i=0;i<4;i++)
		*(directoryStart+ i) = *(nullProcessDirectoryStart + i);
}

int isdirty(int pid, unsigned int vpno){
	unsigned int a = vpno * NBPG;
	int p = (a & 0xFFC00000) >> 22;
	int q = (a & 0x003FF000) >> 12;
	unsigned int *pd = (&proctab[pid])->pdbr;
	unsigned int pdentryvalue = *(pd+p);
	unsigned int *pt = pdentryvalue & 0xfffff000;
	//kprintf("address = %u , value = %u\n", pt+q, *(pt+q));
	int dirty = (*(pt + q) & 0x00000040) >> 6;
	//kprintf("dirty inside function %d\n", dirty);
	return dirty;
}


