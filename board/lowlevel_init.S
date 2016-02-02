
/* 关闭L2缓存 */
.macro init_l2cc
    ldr     r1, =0xa02000
    ldr     r0, =0x0
    str     r0, [r1, #0x100]
.endm

/* AIPS setup - Only setup MPROTx registers.
 * The PACR default values are good.*/
.macro init_aips
	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	ldr r0, =AIPS1_ON_BASE_ADDR
	ldr r1, =0x77777777
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	ldr r1, =0x0
	str r1, [r0, #0x40]
	str r1, [r0, #0x44]
	str r1, [r0, #0x48]
	str r1, [r0, #0x4C]
	str r1, [r0, #0x50]

	ldr r0, =AIPS2_ON_BASE_ADDR
	ldr r1, =0x77777777
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	ldr r1, =0x0
	str r1, [r0, #0x40]
	str r1, [r0, #0x44]
	str r1, [r0, #0x48]
	str r1, [r0, #0x4C]
	str r1, [r0, #0x50]
.endm /* init_aips */


.globl lowlevel_init
lowlevel_init:
        init_l2cc /* init_l2cc */
        init_aips
        b	.
