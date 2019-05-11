
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <actions_regs.h>
#include <am7x_pm.h>

 .set noreorder
#if 0
 .global mips24kec_cache_close
LEAF(mips24kec_cache_close)	
 	mfc0 t0, CP0_CONFIG	
 	li   a0, ~7	
 	and  t0, a0	
 	ori  t0,  2	
 	mtc0 t0, CP0_CONFIG		
 	jr ra	
 	nop
 END(mips24kec_cache_close)

 .global mips24kec_cache_init
LEAF(mips24kec_cache_init) 	
 	mtc0 zero, $28, 0    
 	mtc0 zero, $28, 2	
 	li a0, 0x80000000	
 	la a2, 0x80000000 + 0x8000

1:	cache 8,  0(a0)	
  	cache 9,  0(a0)	
  	bne   a0, a2, 1b	
  	addiu a0, 32	
  	mfc0 t0, CP0_CONFIG	
  	li   a0, ~7	
  	and  t0, a0	
  	ori  t0,  3	
  	mtc0 t0, CP0_CONFIG	

	jr ra	
	nop
END(mips24kec_cache_init)
#endif
 .global switch_to_uncache
LEAF(switch_to_uncache)
	li v1, 0xa0000000
	
	move v0, sp
	or v0, v1
	move sp, v0

	move v0, ra
	or v0, v1
	move ra, v0

	jr ra
	nop
END(switch_to_uncache)

 .global save_sp
LEAF(save_sp)
	move v0, sp

   	li  sp, SP_IN_SRAM
	nop
	
	jr ra
    	nop
END(save_sp)

 .global restore_sp
LEAF(restore_sp)
      	move sp, a0
	
      	jr ra
     	nop
END(restore_sp)

/**
*@brief change ddr clock when system running
*
*@a0	: ddr_clk
*@a1	: clk_dly
*
*@note: This function can not access memory as it is trying to change it
*/
 .global change_ddr_clk
LEAF(change_ddr_clk)
	la t0, CHANGE_START
	la t1, CHANGE_END
PREF_LOOP:
	cache 0x1c, 0(t0)
	bltu t0, t1, PREF_LOOP
	addiu t0, 32


         li t1, DDR_LOW_CLOCK
	sub t1, a0
	bnez t1,HIGH_CLOCK
	nop
LOW_CLOCK:
	li  t1, DDR_LOW_TIMING
	li  t2, DDR_LOW_RFC
	b CHANGE_START
	nop
HIGH_CLOCK:
	li  t1, DDR_HIGH_TIMING
	li  t2, DDR_HIGH_RFC

.align 5
CHANGE_START:
	li t0, CMU_DDRPLL
	sw a0, 0(t0)

	li t0, SDR_CTL
	sw t1, 0x30(t0)
	sw t2, 0x14(t0)
	sw a1,0x24(t0)

RESET_PLL:
	lw t1, 0x18(t0)
	ori t1, 0x100
	sw t1, 0x18(t0)
	li t1, 0xa2
	sw t1, 0xc(t0)
	li t1, 0x8ffff
1:
	bnez t1, 1b
	sub t1, 1
	
	lw t1, 0x18(t0)
	li t2,~0x100
	and t1, t2
	sw t1, 0x18(t0)
	li t1, 0xa2
	sw t1, 0xc(t0)
	li t1, 0x8ffff
2:
	bnez t1, 2b
	sub t1, 1

.align  5
CHANGE_END:
	la t0, CHANGE_START
	la t1, CHANGE_END
INV_LOOP:
	cache 0x10, 0(t0)
	bltu t0, t1, INV_LOOP
	addiu t0, 32
	
      	jr ra
     	nop
END(change_ddr_clk)

