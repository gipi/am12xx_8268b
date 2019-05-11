	.file	1 "asm-offsets.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3

 # -G value = 0, Arch = mips32r2, ISA = 33
 # GNU C (Sourcery G++ Lite 4.3-154) version 4.3.3 (mips-linux-gnu)
 #	compiled by GNU C version 4.3.2, GMP version 4.2.4, MPFR version 2.3.2.
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc -Iinclude
 # -I/nas/garen/8268b_free/am12xx/sdk/linux/arch/mips/include
 # -I/nas/garen/8268b_free/am12xx/sdk/linux/../include
 # -Iinclude/asm-mips/mach-am7x -Iinclude/asm-mips/mach-generic -imultilib
 # soft-float/el -iprefix
 # /usr/local/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/mips-linux-gnu/4.3.3/
 # -isysroot
 # /usr/local/CodeSourcery/Sourcery_G++_Lite/bin/../mips-linux-gnu/libc
 # -D__KERNEL__ -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL
 # -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEL -D_MIPSEL -D__MIPSEL
 # -D__MIPSEL__ -DVMLINUX_LOAD_ADDRESS=0x80003000 -DKBUILD_STR(s)=#s
 # -DKBUILD_BASENAME=KBUILD_STR(asm_offsets)
 # -DKBUILD_MODNAME=KBUILD_STR(asm_offsets) -isystem
 # /usr/local/CodeSourcery/Sourcery_G++_Lite/bin/../lib/gcc/mips-linux-gnu/4.3.3/include
 # -include include/linux/autoconf.h -MD arch/mips/kernel/.asm-offsets.s.d
 # arch/mips/kernel/asm-offsets.c -G 0 -mel -mabi=32 -mno-abicalls
 # -msoft-float -march=mips32r2 -mllsc -mno-shared -auxbase-strip
 # arch/mips/kernel/asm-offsets.s -Os -Wall -Wundef -Wstrict-prototypes
 # -Wno-trigraphs -Werror-implicit-function-declaration
 # -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-aliasing
 # -fno-common -fno-delete-null-pointer-checks -ffunction-sections -fno-pic
 # -ffreestanding -fno-stack-protector -fomit-frame-pointer
 # -fno-strict-overflow -fverbose-asm
 # options enabled:  -falign-loops -fargument-alias -fauto-inc-dec
 # -fbranch-count-reg -fcaller-saves -fcprop-registers -fcrossjumping
 # -fcse-follow-jumps -fdefer-pop -fearly-inlining
 # -feliminate-unused-debug-types -fexpensive-optimizations
 # -fforward-propagate -ffunction-cse -ffunction-sections -fgcse -fgcse-lm
 # -fguess-branch-probability -fident -fif-conversion -fif-conversion2
 # -finline-functions -finline-functions-called-once
 # -finline-small-functions -fipa-pure-const -fipa-reference -fivopts
 # -fkeep-static-consts -fleading-underscore -fmath-errno -fmerge-constants
 # -fmerge-debug-strings -fmove-loop-invariants -fomit-frame-pointer
 # -foptimize-register-move -foptimize-sibling-calls -fpcc-struct-return
 # -fpeephole -fpeephole2 -fregmove -freorder-functions
 # -frerun-cse-after-loop -fsched-interblock -fsched-spec
 # -fsched-stalled-insns-dep -fschedule-insns -fschedule-insns2
 # -fsigned-zeros -fsplit-ivs-in-unroller -fsplit-wide-types -fthread-jumps
 # -ftoplevel-reorder -ftrapping-math -ftree-ccp -ftree-copy-prop
 # -ftree-copyrename -ftree-cselim -ftree-dce -ftree-dominator-opts
 # -ftree-dse -ftree-fre -ftree-loop-im -ftree-loop-ivcanon
 # -ftree-loop-optimize -ftree-parallelize-loops= -ftree-reassoc
 # -ftree-salias -ftree-scev-cprop -ftree-sink -ftree-sra -ftree-store-ccp
 # -ftree-ter -ftree-vect-loop-version -ftree-vrp -funit-at-a-time
 # -fverbose-asm -fzero-initialized-in-bss -mbranch-likely
 # -mcheck-zero-division -mdivide-traps -mdouble-float -mel
 # -mexplicit-relocs -mextern-sdata -mfp-exceptions -mfp32 -mfused-madd
 # -mglibc -mgp32 -mgpopt -mllsc -mlocal-sdata -mlong32 -mmemcpy
 # -mno-mips16 -mno-mips3d -msoft-float -msplit-addresses

 # Compiler executable checksum: 0cc46800581c1e27d643dece23bfe4da

#APP
	.macro _ssnop; sll $0, $0, 1; .endm
	.macro _ehb; sll $0, $0, 3; .endm
	.macro mtc0_tlbw_hazard; _ehb; .endm
	.macro tlbw_use_hazard; _ehb; .endm
	.macro tlb_probe_hazard; _ehb; .endm
	.macro irq_enable_hazard; _ehb; .endm
	.macro irq_disable_hazard; _ehb; .endm
	.macro back_to_back_c0_hazard; _ehb; .endm
	.macro enable_fpu_hazard; _ehb; .endm
	.macro disable_fpu_hazard; _ehb; .endm
		.macro	raw_local_irq_enable				
	.set	push						
	.set	reorder						
	.set	noat						
	ei							
	irq_enable_hazard					
	.set	pop						
	.endm
		.macro	raw_local_irq_disable
	.set	push						
	.set	noat						
	di							
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_save_flags flags			
	.set	push						
	.set	reorder						
	mfc0	\flags, $12					
	.set	pop						
	.endm							

		.macro	raw_local_irq_save result			
	.set	push						
	.set	reorder						
	.set	noat						
	di	\result					
	andi	\result, 1					
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_irq_restore flags			
	.set	push						
	.set	noreorder					
	.set	noat						
	beqz	\flags, 1f					
	 di							
	ei							
1:								
	irq_disable_hazard					
	.set	pop						
	.endm							

#NO_APP
	.section	.text.output_ptreg_defines,"ax",@progbits
	.align	2
	.globl	output_ptreg_defines
	.ent	output_ptreg_defines
	.type	output_ptreg_defines, @function
output_ptreg_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 22 "arch/mips/kernel/asm-offsets.c" 1
	
->#MIPS pt_regs offsets.
 # 0 "" 2
 # 23 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R0 24 offsetof(struct pt_regs, regs[0])	 #
 # 0 "" 2
 # 24 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R1 28 offsetof(struct pt_regs, regs[1])	 #
 # 0 "" 2
 # 25 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R2 32 offsetof(struct pt_regs, regs[2])	 #
 # 0 "" 2
 # 26 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R3 36 offsetof(struct pt_regs, regs[3])	 #
 # 0 "" 2
 # 27 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R4 40 offsetof(struct pt_regs, regs[4])	 #
 # 0 "" 2
 # 28 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R5 44 offsetof(struct pt_regs, regs[5])	 #
 # 0 "" 2
 # 29 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R6 48 offsetof(struct pt_regs, regs[6])	 #
 # 0 "" 2
 # 30 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R7 52 offsetof(struct pt_regs, regs[7])	 #
 # 0 "" 2
 # 31 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R8 56 offsetof(struct pt_regs, regs[8])	 #
 # 0 "" 2
 # 32 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R9 60 offsetof(struct pt_regs, regs[9])	 #
 # 0 "" 2
 # 33 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R10 64 offsetof(struct pt_regs, regs[10])	 #
 # 0 "" 2
 # 34 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R11 68 offsetof(struct pt_regs, regs[11])	 #
 # 0 "" 2
 # 35 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R12 72 offsetof(struct pt_regs, regs[12])	 #
 # 0 "" 2
 # 36 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R13 76 offsetof(struct pt_regs, regs[13])	 #
 # 0 "" 2
 # 37 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R14 80 offsetof(struct pt_regs, regs[14])	 #
 # 0 "" 2
 # 38 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R15 84 offsetof(struct pt_regs, regs[15])	 #
 # 0 "" 2
 # 39 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R16 88 offsetof(struct pt_regs, regs[16])	 #
 # 0 "" 2
 # 40 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R17 92 offsetof(struct pt_regs, regs[17])	 #
 # 0 "" 2
 # 41 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R18 96 offsetof(struct pt_regs, regs[18])	 #
 # 0 "" 2
 # 42 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R19 100 offsetof(struct pt_regs, regs[19])	 #
 # 0 "" 2
 # 43 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R20 104 offsetof(struct pt_regs, regs[20])	 #
 # 0 "" 2
 # 44 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R21 108 offsetof(struct pt_regs, regs[21])	 #
 # 0 "" 2
 # 45 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R22 112 offsetof(struct pt_regs, regs[22])	 #
 # 0 "" 2
 # 46 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R23 116 offsetof(struct pt_regs, regs[23])	 #
 # 0 "" 2
 # 47 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R24 120 offsetof(struct pt_regs, regs[24])	 #
 # 0 "" 2
 # 48 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R25 124 offsetof(struct pt_regs, regs[25])	 #
 # 0 "" 2
 # 49 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R26 128 offsetof(struct pt_regs, regs[26])	 #
 # 0 "" 2
 # 50 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R27 132 offsetof(struct pt_regs, regs[27])	 #
 # 0 "" 2
 # 51 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R28 136 offsetof(struct pt_regs, regs[28])	 #
 # 0 "" 2
 # 52 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R29 140 offsetof(struct pt_regs, regs[29])	 #
 # 0 "" 2
 # 53 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R30 144 offsetof(struct pt_regs, regs[30])	 #
 # 0 "" 2
 # 54 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_R31 148 offsetof(struct pt_regs, regs[31])	 #
 # 0 "" 2
 # 55 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_LO 160 offsetof(struct pt_regs, lo)	 #
 # 0 "" 2
 # 56 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_HI 156 offsetof(struct pt_regs, hi)	 #
 # 0 "" 2
 # 60 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_EPC 172 offsetof(struct pt_regs, cp0_epc)	 #
 # 0 "" 2
 # 61 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_BVADDR 164 offsetof(struct pt_regs, cp0_badvaddr)	 #
 # 0 "" 2
 # 62 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_STATUS 152 offsetof(struct pt_regs, cp0_status)	 #
 # 0 "" 2
 # 63 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_CAUSE 168 offsetof(struct pt_regs, cp0_cause)	 #
 # 0 "" 2
 # 67 "arch/mips/kernel/asm-offsets.c" 1
	
->PT_SIZE 176 sizeof(struct pt_regs)	 #
 # 0 "" 2
 # 68 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_ptreg_defines
	.section	.text.output_task_defines,"ax",@progbits
	.align	2
	.globl	output_task_defines
	.ent	output_task_defines
	.type	output_task_defines, @function
output_task_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 73 "arch/mips/kernel/asm-offsets.c" 1
	
->#MIPS task_struct offsets.
 # 0 "" 2
 # 74 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_STATE 0 offsetof(struct task_struct, state)	 #
 # 0 "" 2
 # 75 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_THREAD_INFO 4 offsetof(struct task_struct, stack)	 #
 # 0 "" 2
 # 76 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_FLAGS 12 offsetof(struct task_struct, flags)	 #
 # 0 "" 2
 # 77 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_MM 172 offsetof(struct task_struct, mm)	 #
 # 0 "" 2
 # 78 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_PID 208 offsetof(struct task_struct, pid)	 #
 # 0 "" 2
 # 79 "arch/mips/kernel/asm-offsets.c" 1
	
->TASK_STRUCT_SIZE 1120 sizeof(struct task_struct)	 #
 # 0 "" 2
 # 80 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_task_defines
	.section	.text.output_thread_info_defines,"ax",@progbits
	.align	2
	.globl	output_thread_info_defines
	.ent	output_thread_info_defines
	.type	output_thread_info_defines, @function
output_thread_info_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 85 "arch/mips/kernel/asm-offsets.c" 1
	
->#MIPS thread_info offsets.
 # 0 "" 2
 # 86 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_TASK 0 offsetof(struct thread_info, task)	 #
 # 0 "" 2
 # 87 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_EXEC_DOMAIN 4 offsetof(struct thread_info, exec_domain)	 #
 # 0 "" 2
 # 88 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_FLAGS 8 offsetof(struct thread_info, flags)	 #
 # 0 "" 2
 # 89 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_TP_VALUE 12 offsetof(struct thread_info, tp_value)	 #
 # 0 "" 2
 # 90 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_CPU 16 offsetof(struct thread_info, cpu)	 #
 # 0 "" 2
 # 91 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_PRE_COUNT 20 offsetof(struct thread_info, preempt_count)	 #
 # 0 "" 2
 # 92 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_ADDR_LIMIT 24 offsetof(struct thread_info, addr_limit)	 #
 # 0 "" 2
 # 93 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_RESTART_BLOCK 32 offsetof(struct thread_info, restart_block)	 #
 # 0 "" 2
 # 94 "arch/mips/kernel/asm-offsets.c" 1
	
->TI_REGS 64 offsetof(struct thread_info, regs)	 #
 # 0 "" 2
 # 95 "arch/mips/kernel/asm-offsets.c" 1
	
->_THREAD_SIZE 8192 THREAD_SIZE	 #
 # 0 "" 2
 # 96 "arch/mips/kernel/asm-offsets.c" 1
	
->_THREAD_MASK 8191 THREAD_MASK	 #
 # 0 "" 2
 # 97 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_thread_info_defines
	.section	.text.output_thread_defines,"ax",@progbits
	.align	2
	.globl	output_thread_defines
	.ent	output_thread_defines
	.type	output_thread_defines, @function
output_thread_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 102 "arch/mips/kernel/asm-offsets.c" 1
	
->#MIPS specific thread_struct offsets.
 # 0 "" 2
 # 103 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG16 520 offsetof(struct task_struct, thread.reg16)	 #
 # 0 "" 2
 # 104 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG17 524 offsetof(struct task_struct, thread.reg17)	 #
 # 0 "" 2
 # 105 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG18 528 offsetof(struct task_struct, thread.reg18)	 #
 # 0 "" 2
 # 106 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG19 532 offsetof(struct task_struct, thread.reg19)	 #
 # 0 "" 2
 # 107 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG20 536 offsetof(struct task_struct, thread.reg20)	 #
 # 0 "" 2
 # 108 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG21 540 offsetof(struct task_struct, thread.reg21)	 #
 # 0 "" 2
 # 109 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG22 544 offsetof(struct task_struct, thread.reg22)	 #
 # 0 "" 2
 # 110 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG23 548 offsetof(struct task_struct, thread.reg23)	 #
 # 0 "" 2
 # 111 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG29 552 offsetof(struct task_struct, thread.reg29)	 #
 # 0 "" 2
 # 112 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG30 556 offsetof(struct task_struct, thread.reg30)	 #
 # 0 "" 2
 # 113 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_REG31 560 offsetof(struct task_struct, thread.reg31)	 #
 # 0 "" 2
 # 114 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_STATUS 564 offsetof(struct task_struct, thread.cp0_status)	 #
 # 0 "" 2
 # 116 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPU 568 offsetof(struct task_struct, thread.fpu)	 #
 # 0 "" 2
 # 118 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_BVADDR 860 offsetof(struct task_struct, thread.cp0_badvaddr)	 #
 # 0 "" 2
 # 120 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_BUADDR 864 offsetof(struct task_struct, thread.cp0_baduaddr)	 #
 # 0 "" 2
 # 122 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_ECODE 868 offsetof(struct task_struct, thread.error_code)	 #
 # 0 "" 2
 # 124 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_TRAPNO 872 offsetof(struct task_struct, thread.trap_no)	 #
 # 0 "" 2
 # 125 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_TRAMP 876 offsetof(struct task_struct, thread.irix_trampoline)	 #
 # 0 "" 2
 # 127 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_OLDCTX 880 offsetof(struct task_struct, thread.irix_oldctx)	 #
 # 0 "" 2
 # 129 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_thread_defines
	.section	.text.output_thread_fpu_defines,"ax",@progbits
	.align	2
	.globl	output_thread_fpu_defines
	.ent	output_thread_fpu_defines
	.type	output_thread_fpu_defines, @function
output_thread_fpu_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 134 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR0 568 offsetof(struct task_struct, thread.fpu.fpr[0])	 #
 # 0 "" 2
 # 135 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR1 576 offsetof(struct task_struct, thread.fpu.fpr[1])	 #
 # 0 "" 2
 # 136 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR2 584 offsetof(struct task_struct, thread.fpu.fpr[2])	 #
 # 0 "" 2
 # 137 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR3 592 offsetof(struct task_struct, thread.fpu.fpr[3])	 #
 # 0 "" 2
 # 138 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR4 600 offsetof(struct task_struct, thread.fpu.fpr[4])	 #
 # 0 "" 2
 # 139 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR5 608 offsetof(struct task_struct, thread.fpu.fpr[5])	 #
 # 0 "" 2
 # 140 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR6 616 offsetof(struct task_struct, thread.fpu.fpr[6])	 #
 # 0 "" 2
 # 141 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR7 624 offsetof(struct task_struct, thread.fpu.fpr[7])	 #
 # 0 "" 2
 # 142 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR8 632 offsetof(struct task_struct, thread.fpu.fpr[8])	 #
 # 0 "" 2
 # 143 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR9 640 offsetof(struct task_struct, thread.fpu.fpr[9])	 #
 # 0 "" 2
 # 144 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR10 648 offsetof(struct task_struct, thread.fpu.fpr[10])	 #
 # 0 "" 2
 # 145 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR11 656 offsetof(struct task_struct, thread.fpu.fpr[11])	 #
 # 0 "" 2
 # 146 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR12 664 offsetof(struct task_struct, thread.fpu.fpr[12])	 #
 # 0 "" 2
 # 147 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR13 672 offsetof(struct task_struct, thread.fpu.fpr[13])	 #
 # 0 "" 2
 # 148 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR14 680 offsetof(struct task_struct, thread.fpu.fpr[14])	 #
 # 0 "" 2
 # 149 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR15 688 offsetof(struct task_struct, thread.fpu.fpr[15])	 #
 # 0 "" 2
 # 150 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR16 696 offsetof(struct task_struct, thread.fpu.fpr[16])	 #
 # 0 "" 2
 # 151 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR17 704 offsetof(struct task_struct, thread.fpu.fpr[17])	 #
 # 0 "" 2
 # 152 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR18 712 offsetof(struct task_struct, thread.fpu.fpr[18])	 #
 # 0 "" 2
 # 153 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR19 720 offsetof(struct task_struct, thread.fpu.fpr[19])	 #
 # 0 "" 2
 # 154 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR20 728 offsetof(struct task_struct, thread.fpu.fpr[20])	 #
 # 0 "" 2
 # 155 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR21 736 offsetof(struct task_struct, thread.fpu.fpr[21])	 #
 # 0 "" 2
 # 156 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR22 744 offsetof(struct task_struct, thread.fpu.fpr[22])	 #
 # 0 "" 2
 # 157 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR23 752 offsetof(struct task_struct, thread.fpu.fpr[23])	 #
 # 0 "" 2
 # 158 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR24 760 offsetof(struct task_struct, thread.fpu.fpr[24])	 #
 # 0 "" 2
 # 159 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR25 768 offsetof(struct task_struct, thread.fpu.fpr[25])	 #
 # 0 "" 2
 # 160 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR26 776 offsetof(struct task_struct, thread.fpu.fpr[26])	 #
 # 0 "" 2
 # 161 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR27 784 offsetof(struct task_struct, thread.fpu.fpr[27])	 #
 # 0 "" 2
 # 162 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR28 792 offsetof(struct task_struct, thread.fpu.fpr[28])	 #
 # 0 "" 2
 # 163 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR29 800 offsetof(struct task_struct, thread.fpu.fpr[29])	 #
 # 0 "" 2
 # 164 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR30 808 offsetof(struct task_struct, thread.fpu.fpr[30])	 #
 # 0 "" 2
 # 165 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FPR31 816 offsetof(struct task_struct, thread.fpu.fpr[31])	 #
 # 0 "" 2
 # 167 "arch/mips/kernel/asm-offsets.c" 1
	
->THREAD_FCR31 824 offsetof(struct task_struct, thread.fpu.fcr31)	 #
 # 0 "" 2
 # 168 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_thread_fpu_defines
	.section	.text.output_mm_defines,"ax",@progbits
	.align	2
	.globl	output_mm_defines
	.ent	output_mm_defines
	.type	output_mm_defines, @function
output_mm_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 173 "arch/mips/kernel/asm-offsets.c" 1
	
->#Size of struct page
 # 0 "" 2
 # 174 "arch/mips/kernel/asm-offsets.c" 1
	
->STRUCT_PAGE_SIZE 32 sizeof(struct page)	 #
 # 0 "" 2
 # 175 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 176 "arch/mips/kernel/asm-offsets.c" 1
	
->#Linux mm_struct offsets.
 # 0 "" 2
 # 177 "arch/mips/kernel/asm-offsets.c" 1
	
->MM_USERS 40 offsetof(struct mm_struct, mm_users)	 #
 # 0 "" 2
 # 178 "arch/mips/kernel/asm-offsets.c" 1
	
->MM_PGD 36 offsetof(struct mm_struct, pgd)	 #
 # 0 "" 2
 # 179 "arch/mips/kernel/asm-offsets.c" 1
	
->MM_CONTEXT 320 offsetof(struct mm_struct, context)	 #
 # 0 "" 2
 # 180 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 181 "arch/mips/kernel/asm-offsets.c" 1
	
->_PAGE_SIZE 4096 PAGE_SIZE	 #
 # 0 "" 2
 # 182 "arch/mips/kernel/asm-offsets.c" 1
	
->_PAGE_SHIFT 12 PAGE_SHIFT	 #
 # 0 "" 2
 # 183 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 184 "arch/mips/kernel/asm-offsets.c" 1
	
->_PGD_T_SIZE 4 sizeof(pgd_t)	 #
 # 0 "" 2
 # 185 "arch/mips/kernel/asm-offsets.c" 1
	
->_PMD_T_SIZE 4 sizeof(pmd_t)	 #
 # 0 "" 2
 # 186 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTE_T_SIZE 4 sizeof(pte_t)	 #
 # 0 "" 2
 # 187 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 188 "arch/mips/kernel/asm-offsets.c" 1
	
->_PGD_T_LOG2 2 PGD_T_LOG2	 #
 # 0 "" 2
 # 189 "arch/mips/kernel/asm-offsets.c" 1
	
->_PMD_T_LOG2 2 PMD_T_LOG2	 #
 # 0 "" 2
 # 190 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTE_T_LOG2 2 PTE_T_LOG2	 #
 # 0 "" 2
 # 191 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 192 "arch/mips/kernel/asm-offsets.c" 1
	
->_PGD_ORDER 0 PGD_ORDER	 #
 # 0 "" 2
 # 193 "arch/mips/kernel/asm-offsets.c" 1
	
->_PMD_ORDER 1 PMD_ORDER	 #
 # 0 "" 2
 # 194 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTE_ORDER 0 PTE_ORDER	 #
 # 0 "" 2
 # 195 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 196 "arch/mips/kernel/asm-offsets.c" 1
	
->_PMD_SHIFT 22 PMD_SHIFT	 #
 # 0 "" 2
 # 197 "arch/mips/kernel/asm-offsets.c" 1
	
->_PGDIR_SHIFT 22 PGDIR_SHIFT	 #
 # 0 "" 2
 # 198 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
 # 199 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PGD 1024 PTRS_PER_PGD	 #
 # 0 "" 2
 # 200 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PMD 1 PTRS_PER_PMD	 #
 # 0 "" 2
 # 201 "arch/mips/kernel/asm-offsets.c" 1
	
->_PTRS_PER_PTE 1024 PTRS_PER_PTE	 #
 # 0 "" 2
 # 202 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_mm_defines
	.section	.text.output_sc_defines,"ax",@progbits
	.align	2
	.globl	output_sc_defines
	.ent	output_sc_defines
	.type	output_sc_defines, @function
output_sc_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 208 "arch/mips/kernel/asm-offsets.c" 1
	
->#Linux sigcontext offsets.
 # 0 "" 2
 # 209 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_REGS 16 offsetof(struct sigcontext, sc_regs)	 #
 # 0 "" 2
 # 210 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_FPREGS 272 offsetof(struct sigcontext, sc_fpregs)	 #
 # 0 "" 2
 # 211 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_ACX 528 offsetof(struct sigcontext, sc_acx)	 #
 # 0 "" 2
 # 212 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_MDHI 552 offsetof(struct sigcontext, sc_mdhi)	 #
 # 0 "" 2
 # 213 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_MDLO 560 offsetof(struct sigcontext, sc_mdlo)	 #
 # 0 "" 2
 # 214 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_PC 8 offsetof(struct sigcontext, sc_pc)	 #
 # 0 "" 2
 # 215 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_FPC_CSR 532 offsetof(struct sigcontext, sc_fpc_csr)	 #
 # 0 "" 2
 # 216 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_FPC_EIR 536 offsetof(struct sigcontext, sc_fpc_eir)	 #
 # 0 "" 2
 # 217 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_HI1 568 offsetof(struct sigcontext, sc_hi1)	 #
 # 0 "" 2
 # 218 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_LO1 572 offsetof(struct sigcontext, sc_lo1)	 #
 # 0 "" 2
 # 219 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_HI2 576 offsetof(struct sigcontext, sc_hi2)	 #
 # 0 "" 2
 # 220 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_LO2 580 offsetof(struct sigcontext, sc_lo2)	 #
 # 0 "" 2
 # 221 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_HI3 584 offsetof(struct sigcontext, sc_hi3)	 #
 # 0 "" 2
 # 222 "arch/mips/kernel/asm-offsets.c" 1
	
->SC_LO3 588 offsetof(struct sigcontext, sc_lo3)	 #
 # 0 "" 2
 # 223 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_sc_defines
	.section	.text.output_signal_defined,"ax",@progbits
	.align	2
	.globl	output_signal_defined
	.ent	output_signal_defined
	.type	output_signal_defined, @function
output_signal_defined:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 254 "arch/mips/kernel/asm-offsets.c" 1
	
->#Linux signal numbers.
 # 0 "" 2
 # 255 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGHUP 1 SIGHUP	 #
 # 0 "" 2
 # 256 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGINT 2 SIGINT	 #
 # 0 "" 2
 # 257 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGQUIT 3 SIGQUIT	 #
 # 0 "" 2
 # 258 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGILL 4 SIGILL	 #
 # 0 "" 2
 # 259 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGTRAP 5 SIGTRAP	 #
 # 0 "" 2
 # 260 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGIOT 6 SIGIOT	 #
 # 0 "" 2
 # 261 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGABRT 6 SIGABRT	 #
 # 0 "" 2
 # 262 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGEMT 7 SIGEMT	 #
 # 0 "" 2
 # 263 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGFPE 8 SIGFPE	 #
 # 0 "" 2
 # 264 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGKILL 9 SIGKILL	 #
 # 0 "" 2
 # 265 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGBUS 10 SIGBUS	 #
 # 0 "" 2
 # 266 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGSEGV 11 SIGSEGV	 #
 # 0 "" 2
 # 267 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGSYS 12 SIGSYS	 #
 # 0 "" 2
 # 268 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGPIPE 13 SIGPIPE	 #
 # 0 "" 2
 # 269 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGALRM 14 SIGALRM	 #
 # 0 "" 2
 # 270 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGTERM 15 SIGTERM	 #
 # 0 "" 2
 # 271 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGUSR1 16 SIGUSR1	 #
 # 0 "" 2
 # 272 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGUSR2 17 SIGUSR2	 #
 # 0 "" 2
 # 273 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGCHLD 18 SIGCHLD	 #
 # 0 "" 2
 # 274 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGPWR 19 SIGPWR	 #
 # 0 "" 2
 # 275 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGWINCH 20 SIGWINCH	 #
 # 0 "" 2
 # 276 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGURG 21 SIGURG	 #
 # 0 "" 2
 # 277 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGIO 22 SIGIO	 #
 # 0 "" 2
 # 278 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGSTOP 23 SIGSTOP	 #
 # 0 "" 2
 # 279 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGTSTP 24 SIGTSTP	 #
 # 0 "" 2
 # 280 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGCONT 25 SIGCONT	 #
 # 0 "" 2
 # 281 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGTTIN 26 SIGTTIN	 #
 # 0 "" 2
 # 282 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGTTOU 27 SIGTTOU	 #
 # 0 "" 2
 # 283 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGVTALRM 28 SIGVTALRM	 #
 # 0 "" 2
 # 284 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGPROF 29 SIGPROF	 #
 # 0 "" 2
 # 285 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGXCPU 30 SIGXCPU	 #
 # 0 "" 2
 # 286 "arch/mips/kernel/asm-offsets.c" 1
	
->_SIGXFSZ 31 SIGXFSZ	 #
 # 0 "" 2
 # 287 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_signal_defined
	.section	.text.output_irq_cpustat_t_defines,"ax",@progbits
	.align	2
	.globl	output_irq_cpustat_t_defines
	.ent	output_irq_cpustat_t_defines
	.type	output_irq_cpustat_t_defines, @function
output_irq_cpustat_t_defines:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 292 "arch/mips/kernel/asm-offsets.c" 1
	
->#Linux irq_cpustat_t offsets.
 # 0 "" 2
 # 293 "arch/mips/kernel/asm-offsets.c" 1
	
->IC_SOFTIRQ_PENDING 0 offsetof(irq_cpustat_t, __softirq_pending)	 #
 # 0 "" 2
 # 295 "arch/mips/kernel/asm-offsets.c" 1
	
->IC_IRQ_CPUSTAT_T 32 sizeof(irq_cpustat_t)	 #
 # 0 "" 2
 # 296 "arch/mips/kernel/asm-offsets.c" 1
	
->
 # 0 "" 2
#NO_APP
	j	$31
	.end	output_irq_cpustat_t_defines
	.ident	"GCC: (Sourcery G++ Lite 4.3-154) 4.3.3"
