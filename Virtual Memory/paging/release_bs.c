#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
	if( (&bsm_tab[bs_id])->bs_no_of_processes_mapped == 0 && (&bsm_tab[bs_id])->bs_created == BSM_CREATED ){
		(&bsm_tab[bs_id])->bs_created = BSM_NOTCREATED;
		(&bsm_tab[bs_id])->bs_npages = 0;
		(&bsm_tab[bs_id])->bs_access_modifier = BSM_PUBLIC;
		//sem
	}
	else{
		return SYSERR;
	}
}