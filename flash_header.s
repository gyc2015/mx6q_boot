# 1 "flash_header.S"
# 1 "/home/gyc/Development/imx/uboot-art/board/freescale/omni//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "flash_header.S"

# 1 "/home/gyc/Development/imx/uboot-art/include/config.h" 1
# 3 "flash_header.S" 2





.section ".text.flasheader", "x"
 b _start
.org 0x400

ivt_header: .word 0x402000D1
app_code_jump_v: .word _start
reserv1: .word 0x0
dcd_ptr: .word dcd_hdr
boot_data_ptr: .word boot_data
self_ptr: .word ivt_header



app_code_csf: .word 0

reserv2: .word 0x0
