#include <stdio.h>
#include <utils.h>


void dump_bytes(unsigned char *src, unsigned cnt, unsigned addr) {
	unsigned char *p = src;
	int i;
	while (cnt >= 16) {
		printf("%08x: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n", addr,
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		p += 16;
		cnt -= 16;
		addr += 16;
	}
	if (cnt) {
		printf("%08x:", addr);
		i = 0;
		while (cnt) {
			printf(" %02x", p[0]);
			p++;
			cnt--;
			i++;
			if (cnt) if (i == 4) {
				i = 0;
				printf(" ");
			}
		}
		printf("\n");
	}
}

/*
 * get_file_size - 获取文件大小
 *
 * @file: 文件
 */
long get_file_size(FILE *file) {
    long tmp = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, tmp, SEEK_SET);
    tmp = ftell(file);
    return size;
}
