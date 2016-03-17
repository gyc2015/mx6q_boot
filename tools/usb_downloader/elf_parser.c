#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include <elf_parser.h>

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
 * @fname: 文件名称
 */
struct elf_img *load_elf_img(char *fname) {
    assert(NULL != fname);
    FILE *file = fopen(fname, "rb");

    struct elf_img *img = malloc(sizeof(struct elf_img));
    assert(0 != img);
    memset(img, 0, sizeof(struct elf_img));

    img->fname = fname;
    img->file = file;

    // header
    if (sizeof(img->header) != fread(&img->header, 1, sizeof(img->header), file))
        return 0;

    // program header
    int phnum = img->header.e_phnum;
    if (0 != phnum) {
        img->pheader = malloc(phnum * sizeof(Elf32_Phdr));
        memset(img->pheader, 0, phnum * sizeof(Elf32_Phdr));
        for (int i = 0; i < phnum; i++) {
            if (sizeof(Elf32_Phdr) != fread(&img->pheader[i], 1, sizeof(Elf32_Phdr), file))
                return 0;
        }
    }

    img->sections = malloc(img->header.e_shnum * sizeof(struct elf_section));
    assert(0 != img->sections);
    memset(img->sections, 0, img->header.e_shnum * sizeof(struct elf_section));

    // 段名称段
    int shstrndx = img->header.e_shstrndx;
    if (0 != load_sec_header(img, shstrndx))
        return 0;
    if (0 != load_sec_data(img, shstrndx))
        return 0;
    img->shstrtab = img->sections[shstrndx].data;
    if (0 != load_sec_name(img, shstrndx))
        return 0;

    for (int i = 0; i < img->header.e_shnum; i++) {
        if (i != shstrndx) {
            if (0 != load_section(img, i))
                return 0;
        }
        if (0 == strcmp(".symtab", img->sections[i].name))
            img->symboltab = img->sections[i].data;
        else if (0 == strcmp(".text", img->sections[i].name))
            img->text = img->sections[i].data;
    }

    return img;
}

void print_elf_header(const Elf32_Ehdr *h) {
    fprintf(stdout, "h.entry = 0x%x\n", h->e_entry);
    fprintf(stdout, "h.phentsize = 0x%x\n", h->e_phentsize);
    fprintf(stdout, "h.phnum = 0x%x\n", h->e_phnum);
    fprintf(stdout, "h.shnum = 0x%x\n", h->e_shnum);
    fprintf(stdout, "h.shoff = 0x%x\n", h->e_shoff);
    fprintf(stdout, "h.shstrndx = 0x%x\n", h->e_shstrndx);
}

void print_sec_header(const struct elf_section *s) {
    fprintf(stdout, "s->name = %s\n", s->name);
    fprintf(stdout, "s->size = 0x%x\n", s->header.sh_size);
    fprintf(stdout, "s->addr = 0x%x\n", s->header.sh_addr);
    fprintf(stdout, "s->offset = 0x%x\n", s->header.sh_offset);
    fprintf(stdout, "s->type = 0x%x\n", s->header.sh_type);
}

void print_pro_header(const Elf32_Phdr *ph) {
    fprintf(stdout, "ph.type = 0x%x\n", ph->p_type);
    fprintf(stdout, "ph.off = 0x%x\n", ph->p_offset);
    fprintf(stdout, "ph.vaddr = 0x%x\n", ph->p_vaddr);
    fprintf(stdout, "ph.paddr = 0x%x\n", ph->p_paddr);
    fprintf(stdout, "ph.filesz = 0x%x\n", ph->p_filesz);
    fprintf(stdout, "ph.memsz = 0x%x\n", ph->p_memsz);
    fprintf(stdout, "ph.flags = 0x%x\n", ph->p_flags);
    fprintf(stdout, "ph.align = 0x%x\n", ph->p_align);
}

unsigned load_text(char *buff, unsigned start, unsigned size, const struct elf_img *img) {
    for (int i = 0; i < size; i++) {
        buff[i] = img->text[start + i];
    }

    return size;
}
/*
int main(int argc, char *argv[]) {
    FILE *file = fopen("imx6q", "rb");
    struct elf_img img;

    load_elf_img(&img, file);

    return 0;
}
*/
