#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <paging.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {

  /* fetch page page from map map_id
     and write beginning at dst.
  */
   void * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
   //kprintf("bs_id = %d, page = %d, physical address = %u, destination adrress = %u\n", bs_id, page, phy_addr, dst);
   bcopy(phy_addr, (void*)dst, NBPG);
   //kprintf("Block copy done\n");
}