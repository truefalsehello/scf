#ifndef SCF_ELF_NAJA_H
#define SCF_ELF_NAJA_H

#include"scf_elf.h"
#include"scf_elf_native.h"
#include"scf_vector.h"
#include"scf_string.h"

int __naja_elf_add_dyn (elf_native_t* naja);
int __naja_elf_post_dyn(elf_native_t* naja, uint64_t rx_base, uint64_t rw_base, elf_section_t* cs);

int __naja_elf_write_phdr   (scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint32_t nb_phdrs);
int __naja_elf_write_interp (scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint64_t len);
int __naja_elf_write_text   (scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint64_t len);
int __naja_elf_write_rodata (scf_elf_context_t* elf, uint64_t r_base,  uint64_t offset, uint64_t len);
int __naja_elf_write_data   (scf_elf_context_t* elf, uint64_t rw_base, uint64_t offset, uint64_t len);
int __naja_elf_write_dynamic(scf_elf_context_t* elf, uint64_t rw_base, uint64_t offset, uint64_t len);

#endif

