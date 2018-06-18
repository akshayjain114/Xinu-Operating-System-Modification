/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	int f;
	for(f=0;f<NFRAMES;f++){
		(&frm_tab[f])->fr_status = FRM_UNMAPPED;
		(&frm_tab[f])->fr_pid = -1;
		(&frm_tab[f])->fr_vpno = -1;
		(&frm_tab[f])->fr_refcnt = 0;
		(&frm_tab[f])->fr_type = -1;
		(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
	}
	return OK;
}

/*----------------------------------------------------------------------
 * get_pagefrm - get a free page frame according page replacement policy
 *----------------------------------------------------------------------
 */
SYSCALL get_pagefrm(int* avail, int pid, int vpno)
{
	int f;
	for(f=0; f < NFRAMES ; f++){
		if( (&frm_tab[f])->fr_status == FRM_UNMAPPED ){
			*avail = f;
			(&frm_tab[f])->fr_status = FRM_MAPPED;
			(&frm_tab[f])->fr_pid = pid;
			(&frm_tab[f])->fr_vpno = vpno;
			(&frm_tab[f])->fr_type = FR_PAGE;
			(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
			(&frm_tab[f])->fr_refcnt++;
			//kprintf("Frame number = %d\n", f);
			if(grpolicy() == SC)
				insertForSC(f);
			return OK;
		}
	}
	
	if( getFrameFromPolicy(&f) == OK){
		if(policydebugging)
			kprintf("Frame to be replaced = %d\n", f + FRAME0);

		unsigned int a = (&frm_tab[f])->fr_vpno * NBPG;
		int p = (a & 0xFFC00000) >> 22;
		int q = (a & 0x003FF000) >> 12;
		int framepid = (&frm_tab[f])->fr_pid;
		unsigned int *pd = (&proctab[framepid])->pdbr;
		unsigned int pdentryvalue = *(pd + p);
		unsigned int *pt = pdentryvalue & 0xfffff000;
		int dirty = (*(pt + q) & 0x00000040) >> 6;
		unsigned int pageStartAddress = *(pt+q) & 0xfffff000;
		//if dirty bit was set
		if(dirty){
			int store;
			int page;
			bsm_lookup(framepid, a, &store, &page);
			write_bs(pageStartAddress, store, page);
		}
		*(pt + q) = 0x00000000;
	
		//TODO: invalidate TLB if page belongs to current process
		if(framepid == currpid){
			asm("invlpg (%0)" ::"r" (a) : "memory");
		}

		int pagetableframe = ((unsigned int)(pt))/NBPG - FRAME0;
		(&frm_tab[pagetableframe])->fr_refcnt--;
		if( (&frm_tab[pagetableframe])->fr_refcnt <= 0){
			*(pd+p) = 0x00000000;
			free_frm(pagetableframe);
			(&frm_tab[pagetableframe])->fr_refcnt = 0; //Just in case!
		}
	
		*avail = f;
		(&frm_tab[f])->fr_status = FRM_MAPPED;
		(&frm_tab[f])->fr_pid = pid;
		(&frm_tab[f])->fr_vpno = vpno;
		(&frm_tab[f])->fr_type = FR_PAGE;
		(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
		(&frm_tab[f])->fr_refcnt++;
		if(grpolicy() == SC)
			insertForSC(f);

		return OK;
	}
	return SYSERR;
}

/*-------------------------------------------------------
 * free_frames - free all frames for a particular process
 *-------------------------------------------------------
 */
SYSCALL free_frames(int pid){
	int f;
	for( f = 0 ; f<NFRAMES ;f++){
		if( (&frm_tab[f])->fr_pid == pid && (&frm_tab[f])->fr_status == FRM_MAPPED ){
			if ( (&frm_tab[f])->fr_type == FR_PAGE){ 
				unsigned int mstore, mpage;
				unsigned int *store = &mstore;				
				unsigned int *page = &mpage;
				bsm_lookup(pid, (&frm_tab[f])->fr_vpno * NBPG, store, page);

//to check that these are not garbage values. Ideally this should be replaced by 
//check if backing store is still mapped to this process		
				if( *store < NBACKINGSTORES && page < 128 && isdirty(pid, (&frm_tab[f])->fr_vpno))
					write_bs( (f+FRAME0)*NBPG, *store, *page );
			}
			free_frm(f);
		}
	}
}

/*----------------------------------------------------------------
 * free_pageframes - free all page frames for a particular process
 *----------------------------------------------------------------
 */
SYSCALL free_pageframes(int pid){
	int f;
	for( f = 0 ; f<NFRAMES ;f++){
		if( (&frm_tab[f])->fr_pid == pid && (&frm_tab[f])->fr_status == FRM_MAPPED ){
			if ( (&frm_tab[f])->fr_type == FR_PAGE){ 
				unsigned int mstore, mpage;
				unsigned int *store = &mstore;				
				unsigned int *page = &mpage;
				bsm_lookup(pid, (&frm_tab[f])->fr_vpno * NBPG, store, page);	
				if(isdirty(pid, (&frm_tab[f])->fr_vpno)){
					write_bs( (f+FRAME0)*NBPG, *store, *page );
				}
				free_frm(f);
			}
		}
	}
}

SYSCALL write_pageframes(int pid){
	STATWORD ps;
	disable(ps);
	int f;
	for( f = 0 ; f<NFRAMES ;f++){
		if( (&frm_tab[f])->fr_pid == pid && (&frm_tab[f])->fr_status == FRM_MAPPED ){
			if ( (&frm_tab[f])->fr_type == FR_PAGE){ 
				unsigned int mstore, mpage;
				unsigned int *store = &mstore;			
				unsigned int *page = &mpage;
				bsm_lookup(pid, (&frm_tab[f])->fr_vpno * NBPG, store, page);	
				if(isdirty(pid, (&frm_tab[f])->fr_vpno)){
					write_bs( (f+FRAME0)*NBPG, *store, *page );
				}
			}
		}
	}
	restore(ps);
}

SYSCALL read_pageframes(int pid){
	STATWORD ps;
	disable(ps);
	int f;
	for( f = 0 ; f<NFRAMES ;f++){
		if( (&frm_tab[f])->fr_pid == pid && (&frm_tab[f])->fr_status == FRM_MAPPED ){
			if ( (&frm_tab[f])->fr_type == FR_PAGE){ 
				unsigned int mstore, mpage;
				unsigned int *store = &mstore;			
				unsigned int *page = &mpage;
				bsm_lookup(pid, (&frm_tab[f])->fr_vpno * NBPG, store, page);	
					read_bs( (f+FRAME0)*NBPG, *store, *page );
			}
		}
	}
	restore(ps);
}


/*----------------------------------------------------------------------
 * get_pagetblfrm - get a free page table frame according page replacement policy
 *----------------------------------------------------------------------
 */
SYSCALL get_pagetblfrm(int* avail, int pid)
{
	int f;
	for(f=0; f <  NFRAMES ; f++){
		if( (&frm_tab[f])->fr_status == FRM_UNMAPPED ){
			*avail = f;
			(&frm_tab[f])->fr_status = FRM_MAPPED;
			(&frm_tab[f])->fr_pid = pid;
			(&frm_tab[f])->fr_vpno = -1;
			(&frm_tab[f])->fr_type = FR_TBL;
			(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
			//kprintf("Giving table frame = %d\n", f);
			return OK;
		}
	}
	if(getFrameFromPolicy(&f) == OK){
		if(policydebugging)
			kprintf("Frame to be replaced = %d\n", f + FRAME0);

		unsigned int a = (&frm_tab[f])->fr_vpno * NBPG;
		int p = (a & 0xFFC00000) >> 22;
		int q = (a & 0x003FF000) >> 12;
		int framepid = (&frm_tab[f])->fr_pid;
		unsigned int *pd = (&proctab[framepid])->pdbr;
		unsigned int pdentryvalue = *(pd + p);
		unsigned int *pt = pdentryvalue & 0xfffff000;
		int dirty = (*(pt + q) & 0x00000040) >> 6;
		unsigned int pageStartAddress = *(pt+q) & 0xfffff000;
		//if dirty bit was set
		if(dirty){
			int store;
			int page;
			bsm_lookup(framepid, a, &store, &page);
			write_bs(pageStartAddress, store, page);
		}
		*(pt + q) = 0x00000000;
	
		//TODO: invalidate TLB if page belongs to current process
		if(framepid == currpid){
			asm("invlpg (%0)" ::"r" (a) : "memory");
		}

		int pagetableframe = ((unsigned int)(pt))/NBPG - FRAME0;
		(&frm_tab[pagetableframe])->fr_refcnt--;
		if( (&frm_tab[pagetableframe])->fr_refcnt <= 0){
			*(pd+p) = 0x00000000;
			free_frm(pagetableframe);
			(&frm_tab[pagetableframe])->fr_refcnt = 0; //Just in case!
		}
	
		*avail = f;
		(&frm_tab[f])->fr_status = FRM_MAPPED;
		(&frm_tab[f])->fr_pid = pid;
		(&frm_tab[f])->fr_vpno = -1;
		(&frm_tab[f])->fr_type = FR_TBL;
		(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
		(&frm_tab[f])->fr_refcnt++;
		return OK;

	}

	

}

/*----------------------------------------------------------------------
 * get_pagedirfrm - get a free page table frame according page replacement policy
 *----------------------------------------------------------------------
 */
SYSCALL get_pagedirfrm(int* avail, int pid)
{
	int f;
	for(f=0; f <  NFRAMES ; f++){
		if( (&frm_tab[f])->fr_status == FRM_UNMAPPED ){
			*avail = f;
			(&frm_tab[f])->fr_status = FRM_MAPPED;
			(&frm_tab[f])->fr_pid = pid;
			(&frm_tab[f])->fr_vpno = -1;
			(&frm_tab[f])->fr_type = FR_DIR;
			(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
			return OK;
		}
	}
	if(getFrameFromPolicy(&f) == OK){
		if(policydebugging)
			kprintf("Frame to be replaced = %d\n", f + FRAME0);

		unsigned int a = (&frm_tab[f])->fr_vpno * NBPG;
		int p = (a & 0xFFC00000) >> 22;
		int q = (a & 0x003FF000) >> 12;
		int framepid = (&frm_tab[f])->fr_pid;
		unsigned int *pd = (&proctab[framepid])->pdbr;
		unsigned int pdentryvalue = *(pd + p);
		unsigned int *pt = pdentryvalue & 0xfffff000;
		int dirty = (*(pt + q) & 0x00000040) >> 6;
		unsigned int pageStartAddress = *(pt+q) & 0xfffff000;
		//if dirty bit was set
		if(dirty){
			int store;
			int page;
			bsm_lookup(framepid, a, &store, &page);
			write_bs(pageStartAddress, store, page);
		}
		*(pt + q) = 0x00000000;
	
		//TODO: invalidate TLB if page belongs to current process
		if(framepid == currpid){
			asm("invlpg (%0)" ::"r" (a) : "memory");
		}

		int pagetableframe = ((unsigned int)(pt))/NBPG - FRAME0;
		(&frm_tab[pagetableframe])->fr_refcnt--;
		if( (&frm_tab[pagetableframe])->fr_refcnt <= 0){
			*(pd+p) = 0x00000000;
			free_frm(pagetableframe);
			(&frm_tab[pagetableframe])->fr_refcnt = 0; //Just in case!
		}
	
		*avail = f;
		(&frm_tab[f])->fr_status = FRM_MAPPED;
		(&frm_tab[f])->fr_pid = pid;
		(&frm_tab[f])->fr_vpno = -1;
		(&frm_tab[f])->fr_type = FR_DIR;
		(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
		(&frm_tab[f])->fr_refcnt++;
		return OK;

	}
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int f)
{
	if( (&frm_tab[f])->fr_type == FR_PAGE && grpolicy() == SC)
		removeFromSCQueue(f);
  	(&frm_tab[f])->fr_status = FRM_UNMAPPED;
	(&frm_tab[f])->fr_pid = -1;
	(&frm_tab[f])->fr_vpno = -1;
	(&frm_tab[f])->fr_type = -1;
	(&frm_tab[f])->fr_dirty = FR_NOTDIRTY;
}