#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include <elf.h>

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

struct elf_section {
    char *name;
    void *data;
    Elf32_Shdr header;
};

struct elf_img {
    FILE *file;

    Elf32_Ehdr header;    // elf文件头
    struct elf_section *sections;

    char *shstrtab;       // 段名称列表
    char *symboltab;      // 符号表
};

/*
 * load_sec_header - 读取段头
 *
 * @img: 目标镜像
 * @n: 段索引
 */
static int load_sec_header(struct elf_img *img, int n) {
    assert(0 != img);
    assert(0 != img->sections);
    
    Elf32_Shdr *sh = &img->sections[n].header;
    off_t offset = img->header.e_shoff + n * sizeof(Elf32_Shdr);

    if (0 != fseek(img->file, offset, SEEK_SET))
        return -1;
    if (sizeof(Elf32_Shdr) != fread(sh, 1, sizeof(Elf32_Shdr), img->file))
        return -1;
    return 0;
}

/*
 * load_sec_data - 读取段数据
 *
 * @img: 目标镜像
 * @n: 段索引
 */
static int load_sec_data(struct elf_img *img, int n) {
    assert(0 != img);
    assert(0 != img->sections);
    
    struct elf_section *s = &img->sections[n];
    
    if (!s->header.sh_size) {
        fprintf(stdout, "no data for section %d:\n", n);
        return 0;
    }


    s->data = memalign(s->header.sh_addralign, s->header.sh_size);
    assert(s->data);
    if (0 != fseek(img->file, s->header.sh_offset, SEEK_SET))
        return -1;
    if (s->header.sh_size != fread(s->data, 1, s->header.sh_size, img->file))
        return -1;

    return 0;
}

/*
 * load_sec_name - 读取段名称
 *
 * @img: 目标镜像
 * @n: 段索引
 */
static int load_sec_name(struct elf_img *img, int n) {
    assert(0 != img);
    assert(0 != img->sections);
    assert(0 != img->shstrtab);

    struct elf_section *s = &img->sections[n];
    s->name = img->shstrtab + s->header.sh_name;

    return 0;
}

/*
 * load_section - 读取段
 *
 * @img: 目标镜像
 * @n: 段索引
 */
static int load_section(struct elf_img *img, int n) {
    assert(0 != img);
    assert(0 != img->sections);
    
    if (0 != load_sec_header(img, n))
        return -1;
    if (0 != load_sec_data(img, n))
        return -1;
    if (0 != load_sec_name(img, n))
        return -1;
    return 0;
}

/*
 * load_elf_img - 装载镜像文件
 *
 * @img: 目标镜像
 * @file: 文件指针
 */
static int load_elf_img(struct elf_img *img, FILE *file) {
    assert(NULL != img);
    assert(NULL != file);
    memset(img, 0, sizeof(struct elf_img));

    img->file = file;

    if (sizeof(img->header) != fread(&img->header, 1, sizeof(img->header), file))
        return -1;

    fprintf(stdout, "h.entry = 0x%x\n", img->header.e_entry);
    fprintf(stdout, "h.shnum = 0x%x\n", img->header.e_shnum);
    fprintf(stdout, "h.shoff = 0x%x\n", img->header.e_shoff);
    fprintf(stdout, "h.shstrndx = 0x%x\n", img->header.e_shstrndx);
    int shstrndx = img->header.e_shstrndx;

    img->sections = malloc(img->header.e_shnum * sizeof(struct elf_section));
    assert(0 != img->sections);
    memset(img->sections, 0, img->header.e_shnum * sizeof(struct elf_section));

    if (0 != load_sec_header(img, shstrndx))
        return -1;
    if (0 != load_sec_data(img, shstrndx))
        return -1;
    img->shstrtab = img->sections[shstrndx].data;
    if (0 != load_sec_name(img, shstrndx))
        return -1;

    for (int i = 0; i < img->header.e_shnum; i++) {
        if (i != shstrndx)
            load_section(img, i);
        if (0 == strcmp(".symtab", img->sections[i].name))
            img->symboltab = img->sections[i].data;
            
        fprintf(stdout, "\n>>>>>>>>>>>>>>>>>>>>>>>\n");
        fprintf(stdout, "section_%d->name = %s\n", i, img->sections[i].name);
        fprintf(stdout, "section_%d->size = 0x%x\n", i, img->sections[i].header.sh_size);
        fprintf(stdout, "section_%d->addr = 0x%x\n", i, img->sections[i].header.sh_addr);
        fprintf(stdout, "section_%d->offset = 0x%x\n", i, img->sections[i].header.sh_offset);
        fprintf(stdout, "section_%d->type = 0x%x\n", i, img->sections[i].header.sh_type);
        fprintf(stdout, "\n>>>>>>>>>>>>>>>>>>>>>>>\n");
    }

    return 0;
}



int main(int argc, char *argv[]) {
    FILE *file = fopen("u-boot", "rb");
    struct elf_img img;

    load_elf_img(&img, file);

    return 0;
}
