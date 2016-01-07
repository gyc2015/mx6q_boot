#include <config.h>
#include <asm/arch/mx6.h>

#ifndef CONFIG_FLASH_HEADER_OFFSET
# error "Must define the offset of flash header"
#endif

#define CPU_2_BE_32(l) \
       ((((l) & 0x000000FF) << 24) | \
	(((l) & 0x0000FF00) << 8)  | \
	(((l) & 0x00FF0000) >> 8)  | \
	(((l) & 0xFF000000) >> 24))

#define MXC_DCD_ITEM(i, addr, val)   \
dcd_node_##i:                        \
        .word CPU_2_BE_32(addr) ;     \
        .word CPU_2_BE_32(val)  ;     \


.section ".text.flasheader", "x"
	b	_start
/*************************************************/
/*           Image Vector Table, IVT             */		
/*************************************************/
.org	CONFIG_FLASH_HEADER_OFFSET
ivt_header:       .word 0x402000D1		/* Tag=0xD1, Len=0x0020, Ver=0x40 */
app_code_jump_v:  .word _start			/* 程序的入口,定义在cpu/${CPU}/start.S中 */
reserv1:          .word 0x0
dcd_ptr:          .word dcd_hdr
boot_data_ptr:	  .word boot_data
self_ptr:         .word ivt_header
app_code_csf:     .word 0
reserv2:          .word 0x0
/*************************************************/
/*                  BOOT DATA                    */		
/*************************************************/
boot_data:        .word TEXT_BASE
image_len:        .word 0xDEADBEAF	/* 需要具体计算镜像的大小 */
plugin:           .word 0x0
/*************************************************/
/*            DCD, DDR3配置                      */		
/*************************************************/
dcd_hdr:          .word 0x40AAAAD2		/* Tag=0xD2, Len=AAAA, Ver=0x40 */
write_dcd_cmd:    .word 0x04AAAACC		/* Tag=0xCC, Len=AAAA, Param=0x04 */

# IOMUXC_BASE_ADDR  = 0x20e0000
# DDR IO TYPE
MXC_DCD_ITEM(1, IOMUXC_BASE_ADDR + 0x798, 0x000c0000)

