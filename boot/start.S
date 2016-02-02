

/* invalidate the D-CACHE */
.macro inv_dcache
    mov     r0,#0
    mcr     p15,2,r0,c0,c0,0  /* 选择 dcache */
    mrc     p15,1,r0,c0,c0,0  /* 获取 cache size ID */
    mov     r0,r0,ASR #13
    ldr     r3,=0xfff
    and     r0,r0,r3
    cmp     r0,#0x7f			/* 0x7F 16KB */
    moveq   r6,#0x1000
    beq     size_done
    cmp     r0,#0xff			/* 0xFF 32KB */
    moveq   r6,#0x2000
    movne   r6,#0x4000			/* 0x1FF 64KB */
size_done:
    mov     r2,#0				/* 4-ways */
    mov     r3,#0x40000000
    mov     r4,#0x80000000
    mov     r5,#0xc0000000
d_inv_loop:
    mcr     p15,0,r2,c7,c6,2  	/* invalidate dcache by set / way */
    mcr     p15,0,r3,c7,c6,2
    mcr     p15,0,r4,c7,c6,2
    mcr     p15,0,r5,c7,c6,2
    add     r2,r2,#0x20			/* 遍历sets */
    add     r3,r3,#0x20
    add     r4,r4,#0x20
    add     r5,r5,#0x20

    cmp     r2,r6
    bne     d_inv_loop
.endm

/****************************************************/
.globl _start
_start:
		b	reset

.global _end_vect
_end_vect:
/****************************************************/

		.balignl 16,0xdeadbeef


_TEXT_BASE:
		.word	TEXT_BASE
/****************************************************/


	
reset:
		// 配置CPU工作在 SVC 模式下
		mrs	r0, cpsr
		bic	r0, r0, #0x1f
		orr	r0, r0, #0xd3
		msr	cpsr,r0
		// 初始化cpu和一些底层设置
		bl cpu_init_crit

		b	.

/*
 * CPU_init_critical - 初始化CPU的内存管理单元, Cache等
 */
cpu_init_crit:
		mov	r0, #0
		mcr	p15, 0, r0, c8, c7, 0	// invalidate TLBs, MMU的页转换cache(Translation Lookaside Buffer, TLB)
		mcr	p15, 0, r0, c7, c5, 0	// invalidate icache

		mrc	p15, 0, r0, c1, c0, 0
		bic	r0, r0, #0x00002000		// 异常向量映射到0x0000 0000
		bic	r0, r0, #0x00000007		// 关闭Data caching, 关闭MMU 
		orr	r0, r0, #0x00000002		// 开启Strict Alignment fault 检测
		orr	r0, r0, #0x00000800		// 使能Program flow prediction
		mcr	p15, 0, r0, c1, c0, 0

		inv_dcache

		mov	ip, lr
		bl	lowlevel_init			// 调用主板的配置函数, 主要为了初始化pll,mux,memory
		mov	lr, ip
		mov	pc, lr

		b	.
