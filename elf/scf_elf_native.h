#ifndef SCF_ELF_NATIVE_H
#define SCF_ELF_NATIVE_H

#include"scf_elf.h"
#include"scf_vector.h"
#include"scf_string.h"

typedef struct elf_section_s  elf_section_t;

struct elf_section_s
{
	elf_section_t*  link;
	elf_section_t*  info;

	scf_string_t*   name;

	Elf64_Shdr      sh;

	uint64_t        offset;

	uint16_t        index;
	uint8_t*        data;
	int             data_len;
};

typedef struct {
	elf_section_t*  section;

	scf_string_t*   name;

	Elf64_Sym       sym;

	int             index;
	uint8_t         dyn_flag:1;

} elf_sym_t;

typedef struct {
	Elf64_Ehdr      eh;

	Elf64_Shdr      sh_null;

	scf_vector_t*   sections;
	scf_vector_t*   phdrs;

	Elf64_Shdr      sh_symtab;
	scf_vector_t*   symbols;

	Elf64_Shdr      sh_strtab;

	Elf64_Shdr      sh_shstrtab;
	scf_string_t*   sh_shstrtab_data;

	scf_vector_t*   dynsyms;
	scf_vector_t*   dyn_needs;
	scf_vector_t*   dyn_relas;

	elf_section_t*  interp;
	elf_section_t*  dynsym;
	elf_section_t*  dynstr;
	elf_section_t*  gnu_version;
	elf_section_t*  gnu_version_r;
	elf_section_t*  rela_plt;
	elf_section_t*  plt;
	elf_section_t*  dynamic;
	elf_section_t*  got_plt;

} elf_native_t;


int  elf_open (scf_elf_context_t* elf);
int  elf_close(scf_elf_context_t* elf);

int  elf_add_sym  (scf_elf_context_t* elf, const scf_elf_sym_t* sym, const char* sh_name);

int  elf_add_section (scf_elf_context_t* elf, const scf_elf_section_t* section);

int  elf_add_rela_section(scf_elf_context_t* elf, const scf_elf_section_t* section, scf_vector_t* relas);

int  elf_read_shstrtab(scf_elf_context_t* elf);
int  elf_read_section (scf_elf_context_t* elf, scf_elf_section_t** psection, const char* name);

int  elf_add_dyn_need(scf_elf_context_t* elf, const char* soname);
int  elf_add_dyn_rela(scf_elf_context_t* elf, const scf_elf_rela_t* rela);

int  elf_read_phdrs  (scf_elf_context_t* elf, scf_vector_t* phdrs);
int  elf_read_relas  (scf_elf_context_t* elf, scf_vector_t* relas, const char* sh_name);
int  elf_read_syms   (scf_elf_context_t* elf, scf_vector_t* syms,  const char* sh_name);

int  elf_find_sym    (elf_sym_t**   psym, Elf64_Rela* rela, scf_vector_t* symbols);
void elf_process_syms(elf_native_t* native, uint32_t cs_index);

int  elf32_find_sym    (elf_sym_t**   psym, Elf32_Rela* rela, scf_vector_t* symbols);
void elf32_process_syms(elf_native_t* native, uint32_t cs_index);

int  elf_write_sections(scf_elf_context_t* elf);
int  elf_write_shstrtab(scf_elf_context_t* elf);
int  elf_write_symtab  (scf_elf_context_t* elf);
int  elf_write_strtab  (scf_elf_context_t* elf);
int  elf_write_rel     (scf_elf_context_t* elf, uint16_t e_machine);

int  elf_sym_cmp(const void* v0, const void* v1);

static inline void elf_header(Elf64_Ehdr* eh,
		uint16_t    e_type,
		uint16_t    e_machine,
		Elf64_Addr  e_entry,
		Elf64_Off   e_phoff,
		uint16_t    e_phnum,
		uint16_t    e_shnum,
		uint16_t    e_shstrndx)
{
	eh->e_ident[EI_MAG0]    = ELFMAG0;
	eh->e_ident[EI_MAG1]    = ELFMAG1;
	eh->e_ident[EI_MAG2]    = ELFMAG2;
	eh->e_ident[EI_MAG3]    = ELFMAG3;
	eh->e_ident[EI_CLASS]   = ELFCLASS64;
	eh->e_ident[EI_DATA]    = ELFDATA2LSB;
	eh->e_ident[EI_VERSION] = EV_CURRENT;
	eh->e_ident[EI_OSABI]   = ELFOSABI_SYSV;

	eh->e_type      = e_type;
	eh->e_machine   = e_machine;
	eh->e_version   = EV_CURRENT;
	eh->e_entry     = e_entry;
	eh->e_ehsize    = sizeof(Elf64_Ehdr);

	eh->e_phoff     = e_phoff;
	eh->e_phentsize = sizeof(Elf64_Phdr);
	eh->e_phnum     = e_phnum;

	eh->e_shoff     = sizeof(Elf64_Ehdr);
	eh->e_shentsize = sizeof(Elf64_Shdr);
	eh->e_shnum     = e_shnum;
	eh->e_shstrndx  = e_shstrndx;
}

static inline void section_header(Elf64_Shdr* sh,
		uint32_t    sh_name,
        Elf64_Addr  sh_addr,
        Elf64_Off   sh_offset,
        uint64_t    sh_size,
        uint32_t    sh_link,
        uint32_t    sh_info,
        uint64_t    sh_entsize)
{
	sh->sh_name		= sh_name;
	sh->sh_addr		= sh_addr;
	sh->sh_offset	= sh_offset;
	sh->sh_size		= sh_size;
	sh->sh_link		= sh_link;
	sh->sh_info		= sh_info;
	sh->sh_entsize	= sh_entsize;
}

#endif

