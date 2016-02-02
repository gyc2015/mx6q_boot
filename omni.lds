
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;
 	. = ALIGN(4);
 	.text :
 	{
		boot/start.o
	 	*(.text)
 	}

	. = ALIGN(4);
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = .;
	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	__u_boot_cmd_end = .;

	/* Original Size is 0x27..., enlarge to 0x2F000 */
	. = TEXT_BASE + 0x2F000;
	__hab_data = .;
	. = . + 0x2000;
	__hab_data_end = .;
	/* End of Hab Data, Place it before BSS section */


	. = ALIGN(4);
	_end_of_copy = .; /* end_of ROM copy code here */
	__bss_start = .;
	.bss : { *(.bss) }
	_end = .;

}
