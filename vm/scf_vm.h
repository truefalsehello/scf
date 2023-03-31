#ifndef SCF_VM_H
#define SCF_VM_H

#include"scf_elf.h"
#include<dlfcn.h>

#if 1
#define NAJA_PRINTF   printf
#else
#define NAJA_PRINTF
#endif

#define NAJA_REG_FP   29
#define NAJA_REG_LR   30
#define NAJA_REG_SP   31

typedef struct scf_vm_s       scf_vm_t;
typedef struct scf_vm_ops_s   scf_vm_ops_t;

struct  scf_vm_s
{
	scf_elf_context_t*        elf;

	scf_vector_t*             sofiles;
	scf_vector_t*             phdrs;

	scf_elf_phdr_t*           text;
	scf_elf_phdr_t*           rodata;
	scf_elf_phdr_t*           data;

	scf_elf_phdr_t*           dynamic;
	Elf64_Rela*               jmprel;
	uint64_t                  jmprel_addr;
	uint64_t                  jmprel_size;
	Elf64_Sym*                dynsym;
	uint64_t*                 pltgot;
	uint8_t*                  dynstr;

	scf_vm_ops_t*             ops;
	void*                     priv;
};

struct scf_vm_ops_s
{
	const char* name;

	int  (*open )(scf_vm_t* vm);
	int  (*close)(scf_vm_t* vm);

	int  (*run  )(scf_vm_t* vm, const char* path, const char* sys);
};

#define  SCF_VM_Z   0
#define  SCF_VM_NZ  1
#define  SCF_VM_GE  2
#define  SCF_VM_GT  3
#define  SCF_VM_LE  4
#define  SCF_VM_LT  5

typedef union {
	uint8_t  b[32];
	uint16_t w[16];
	uint32_t l[8];
	uint64_t q[4];
	float    f[8];
	double   d[4];
} fv256_t;

typedef struct {
	uint64_t  regs[32];
	fv256_t   fvec[32];

	uint64_t  ip;
	uint64_t  flags;

#define STACK_INC 16
	uint8_t*  stack;
	int64_t   size;

	uint64_t  _start;

} scf_vm_naja_t;

#define NAJA_ADD        0
#define NAJA_SUB        1
#define NAJA_MUL        2
#define NAJA_DIV        3
#define NAJA_LDR_DISP   4
#define NAJA_STR_DISP   5
#define NAJA_AND        6
#define NAJA_OR         7
#define NAJA_JMP_DISP   8
#define NAJA_CMP        9
#define NAJA_JMP_REG   10
#define NAJA_SETCC     11
#define NAJA_LDR_SIB   12
#define NAJA_STR_SIB   13
#define NAJA_TEQ       14
#define NAJA_MOV       15

#define NAJA_CALL_DISP 24
#define NAJA_CALL_REG  26
#define NAJA_ADRP      42
#define NAJA_RET       56

typedef int (*naja_opcode_pt)(scf_vm_t* vm, uint32_t inst);

int scf_vm_open (scf_vm_t** pvm, const char* arch);
int scf_vm_close(scf_vm_t*   vm);
int scf_vm_clear(scf_vm_t*   vm);

int scf_vm_run  (scf_vm_t*   vm, const char* path, const char* sys);

int naja_vm_open(scf_vm_t* vm);
int naja_vm_close(scf_vm_t* vm);
int naja_vm_init(scf_vm_t* vm, const char* path, const char* sys);

#endif

