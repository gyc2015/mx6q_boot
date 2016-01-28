
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

		mov	ip, lr
		bl	lowlevel_init			// 调用主板的配置函数, 主要为了初始化pll,mux,memory
		mov	lr, ip
		mov	pc, lr

		b	.
