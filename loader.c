#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

typedef void (*func_t)();


#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#define Elf_Shdr Elf64_Shdr

#define SHIFT 0x000000
// #define SHIFT 0x00000
// readelfした結果
#define DEFAULT_START_POINT 0x400000

// https://smallkirby.hatenablog.com/?page=1560403658
#define IS_ELF(ehdr)\
    ((ehdr).e_ident[EI_MAG0] == ELFMAG0 &&\
     (ehdr).e_ident[EI_MAG1] == ELFMAG1 &&\
     (ehdr).e_ident[EI_MAG2] == ELFMAG2 &&\
     (ehdr).e_ident[EI_MAG3] == ELFMAG3)

// セクションヘッダ(39頁)
static Elf_Shdr* get_shdr(Elf_Ehdr *ehdr, int num){
    Elf_Shdr* shdr;
    if((num < 0) || (num >= ehdr->e_shnum)){
        fprintf(stderr, "cannot find section header (%d)\n", num);
        return NULL;
    }
    shdr = (Elf_Shdr*)((char*)ehdr + ehdr->e_shoff + ehdr->e_shentsize*num);
    return shdr;
}

// セクションをセクション名で検索
static Elf_Shdr* search_shdr(Elf_Ehdr* ehdr, char* name){
    int i;
    Elf_Shdr* shdr;  // セクションヘッダ
    Elf_Shdr* nhdr;  // セクション名格納セクション

    // shstrndx: セクション名格納セクションの番号
    nhdr = get_shdr(ehdr, ehdr->e_shstrndx);

    // セクション数まわす
    for(i=0; i < ehdr->e_shnum; i++){
        shdr = get_shdr(ehdr, i);
        // sh_name: 名前の格納位置
        if(!strcmp((char*)ehdr + nhdr->sh_offset + shdr->sh_name, name)){
            fprintf(stderr, "found %s\n", name);
            return shdr;
        }
    }

    fprintf(stderr, "cannot find shdr %s\n", name);

    return NULL;
}

static func_t load_file(char* head){
    int i;
    Elf_Ehdr* ehdr;
    Elf_Phdr* phdr;
    Elf_Shdr* shdr;
    func_t f;

    ehdr = (Elf_Ehdr*)head;
    if(!IS_ELF(*ehdr)){
        fprintf(stderr, "This is not ELF file.\n");
        return NULL;
    }

    if(ehdr->e_ident[EI_CLASS] != ELFCLASS64){
        fprintf(stderr, "unknown class. (%d)\n", (int)ehdr->e_ident[EI_CLASS]);
        return NULL;
    }

    if(ehdr->e_ident[EI_DATA] != ELFDATA2LSB){
        fprintf(stderr, "unknown endian.  (%d)\n", (int)ehdr->e_ident[EI_DATA]);
        return NULL;
    }

    if(ehdr->e_type != ET_EXEC){
        fprintf(stderr, "unknown type.  (%d)\n", (int)ehdr->e_type);
    }

    // プログラムヘッダ
    for(i=0; i < ehdr->e_phnum; i++){
        fprintf(stderr, "Program Header %d:", i);
        phdr = (Elf_Phdr*)(head + ehdr->e_phoff + ehdr->e_phentsize * i);
        
        switch(phdr->p_type){
            case PT_LOAD:
            fprintf(stderr, " Type:LOAD(%p@%p, %p=%p+%lx, %lx)", SHIFT + (char*)phdr->p_vaddr, &(phdr->p_vaddr),  head +  phdr->p_offset, 
            head ,  phdr->p_offset, phdr->p_filesz);
            memcpy(SHIFT + (char*)phdr->p_vaddr, head +  phdr->p_offset, phdr->p_filesz);
            fprintf(stderr, " (loaded)\n");
            break;
        default:
            fprintf(stderr, " Type:OTHER\n");
        }
    }

    fprintf(stderr, "reading sections.\n");
    // セクションの処理
    // BSSをクリア
    shdr = search_shdr(ehdr, ".bss");
    if(shdr){
        fprintf(stderr, "clear BSS: 0x%08lx, 0x%08lx\n", shdr->sh_addr, shdr->sh_size);
        memset((char*)shdr->sh_addr  + SHIFT, 0, shdr->sh_size);
    }

    f = (func_t)(ehdr->e_entry + SHIFT);
    fprintf(stderr, "Entry Point: 0x%08lx\n", (long)f);

    return f;
}

#define USE_LOADER_ARGV

int main(int argc, char* argv[]){
    int fd;
    struct stat sb;
    char* head;
    func_t f;
    static char filename[128];

#ifdef USE_LOADER_ARGV
    // static char **stackp;
#endif 

    strcpy(filename, argv[1]);
    fprintf(stderr, "open file. (%s)\n", filename);
    fd = open(filename, O_RDONLY);
    if(fd < 0){
        fprintf(stderr, "cannot open file. (%s)\n", filename);
        exit(1);
    }
    fstat(fd, &sb);
    fprintf(stderr, "loading %lx\n", sb.st_size);
    head = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    fprintf(stderr, "mmapped to %p\n", head);
    f = load_file(head);
    if(f == NULL){
        fprintf(stderr, "fail to load file\n");
        exit(1);
    }
    close(fd);
    fprintf(stderr, "jump to entry point.\n");

#ifdef USE_LOADER_ARGV
    int* x = &argc;
    char** y = &argv[1];
    // ediにargc-1を積む
    argc--;
    asm volatile ("movl %0,%%edi" :: "m"(x));
    // esiにargv[1]の番地を積む
    asm volatile ("movl %0,%%esi" :: "m"(y));
    // argvの実体ってどこに住んでるんだ？→このmainのスタック中のはず
#endif
    f();
    // asm volatile ("jmp *%0" :: "m"(f));
    exit(0);
}