#include <mx6.h>

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

		ldr r0, =GPIO1_BASE_ADDR
		ldr r1, =0x00000004
		str r1, [r0, #0x0]

		b	.


