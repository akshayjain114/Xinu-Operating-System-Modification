/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL init_bsm()
{
	int i;
	for(i=0;i<NBACKINGSTORES;i++){
		(&bsm_tab[i])->bs_no_of_processes_mapped = 0;
		(&bsm_tab[i])->bs_created = BSM_NOTCREATED;
		(&bsm_tab[i])->bs_access_modifier = BSM_PUBLIC;
		//(&bsm_tab[i])->bs_sem	 = -1;
		(&bsm_tab[i])->bs_npages = 0;

		int p;
		for(p=0;p<NPROC;p++){
			free_bsm(i, p);
		}
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int i;
	for(i=0;i<NBACKINGSTORES;i++){
		if( (&bsm_tab[i])->bs_no_of_processes_mapped == 0){
			*avail = i;
			return OK;
		}
	}
	*avail = -1;
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i, int pid)
{
	(&bsm_tab[i])->bs_statuses[pid] = BSM_UNMAPPED;
	(&bsm_tab[i])->bs_vpnos[pid]	 = -1;
	(&bsm_tab[i])->bs_npages_per_process[pid] = 0;

	if( (&bsm_tab[i])->bs_no_of_processes_mapped > 0)
		(&bsm_tab[i])->bs_no_of_processes_mapped--;

	if( (&bsm_tab[i])->bs_no_of_processes_mapped == 0){
		//Doubt!
		(&bsm_tab[i])->bs_access_modifier = BSM_PUBLIC;
		//(&bsm_tab[i])->bs_sem	 = -1;
	}
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
	int vpno = (vaddr & 0xfffff000)>>12;
	int i;
	for(i=0;i<NBACKINGSTORES;i++){
		if( (&bsm_tab[i])->bs_statuses[pid] == BSM_MAPPED){
			int base_vpno = (&bsm_tab[i])->bs_vpnos[pid];	
			int npages = (&bsm_tab[i])->bs_npages_per_process[pid];
			if(vpno >= base_vpno && vpno < (base_vpno+npages) ){
			 	//kprintf("In if: vpno = %d, base_vpno = %d, npages = %d \n", vpno, base_vpno, npages);
				*store = i;
				//kprintf("Came in bsm lookup: store = %d\n", *store);
				*pageth = (vpno - base_vpno);
				return OK;
			}
			else{
			 	//kprintf("In else: vpno = %d, base_vpno = %d, npages = %d \n", vpno, base_vpno, npages);
				*store = -1;
				*pageth = -1;
				return SYSERR;
			}
		}
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages, int access_modifier)
{
	if( (&bsm_tab[source])->bs_created == BSM_CREATED 
		&& 
	    (&bsm_tab[source])->bs_statuses[pid] == BSM_UNMAPPED 
		&&
	    (&bsm_tab[source])->bs_npages >= npages
	  ){
		(&bsm_tab[source])->bs_statuses[pid] = BSM_MAPPED;
		(&bsm_tab[source])->bs_vpnos[pid] = vpno;
		(&bsm_tab[source])->bs_npages_per_process[pid] = npages;
		(&bsm_tab[source])->bs_access_modifier = access_modifier;
		(&bsm_tab[source])->bs_no_of_processes_mapped++;
		
		(&proctab[pid])->store = source;
		(&proctab[pid])->vhpno = vpno;
		(&proctab[pid])->vhpnpages = npages;
		
		return OK;
	}
	else{
		//Already mapped!
		return SYSERR;
	}
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete a mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	int i;
	for(i=0;i<NBACKINGSTORES;i++){
		if( (&bsm_tab[i])->bs_statuses[pid] == BSM_MAPPED && (&bsm_tab[i])->bs_vpnos[pid] == vpno){
			free_bsm(i, pid);
			return OK;
		}
	}
	return SYSERR;
}

int mapped(unsigned long vaddr, int pid){
	int vpno = (vaddr & 0xfffff000)>>12;
	int i;
	for(i=0;i<NBACKINGSTORES;i++){
		if( (&bsm_tab[i])->bs_statuses[pid] == BSM_MAPPED){
			int base_vpno = (&bsm_tab[i])->bs_vpnos[pid];	
			int npages = (&bsm_tab[i])->bs_npages_per_process[pid];
			if(vpno >= base_vpno && vpno < (base_vpno+npages) )
				return 1;
		}
	}
	return 0;
}