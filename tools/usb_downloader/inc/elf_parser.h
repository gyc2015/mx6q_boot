#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <elf.h>

struct elf_section {
    char *name;
    void *data;
    Elf32_Shdr header;
};

struct elf_img {
    char *fname;
    FILE *file;

    Elf32_Ehdr header;    // elf文件头
    Elf32_Phdr *pheader;   // program header
    struct elf_section *sections;

    char *shstrtab;       // 段名称列表
    char *symboltab;      // 符号表
    char *text;           // .text段内容
};

struct elf_img *load_elf_img(char *fname);
void print_elf_header(const Elf32_Ehdr *h);
void print_sec_header(const struct elf_section *s);
void print_pro_header(const Elf32_Phdr *ph);
unsigned load_text(char *buff, unsigned start, unsigned size, const struct elf_img *img);

#endif
