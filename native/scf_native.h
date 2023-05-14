#ifndef SCF_NATIVE_H
#define SCF_NATIVE_H

#include"scf_3ac.h"
#include"scf_parse.h"

typedef struct scf_native_ops_s scf_native_ops_t;

struct scf_register_s {
	uint32_t		id;
	int				bytes;
	char*			name;

	intptr_t        color;

	scf_vector_t*	dag_nodes;

	uint32_t        updated;
	uint32_t        used;
};

struct scf_OpCode_s {
	int				type;
	char*			name;
};

typedef struct {
	scf_register_t* base;
	scf_register_t* index;

	int32_t         scale;
	int32_t         disp;
	int32_t         size;
} scf_sib_t;

typedef struct {

	scf_register_t* base;
	scf_register_t* index;
	int             scale;
	int             disp;

	uint64_t        imm;
	int             imm_size;

	uint8_t         flag;
} scf_inst_data_t;

typedef struct {

	scf_3ac_code_t* c;

	scf_OpCode_t*	OpCode;

	scf_inst_data_t src;
	scf_inst_data_t dst;

	uint8_t			code[32];
	int				len;

	int             nb_used;

} scf_instruction_t;

typedef struct {
	scf_3ac_code_t*     code;        // related 3ac code
	scf_function_t*     func;
	scf_variable_t*     var;
	scf_string_t*       name;

	scf_instruction_t*  inst;
	int                 inst_offset; // byte offset in instruction
	int64_t             text_offset; // byte offset in .text segment
	uint64_t            type;
	int                 addend;
} scf_rela_t;

typedef struct {
	scf_native_ops_t*	ops;

	scf_inst_ops_t*     iops;
	scf_regs_ops_t*     rops;

	void*				priv;

} scf_native_t;

struct scf_native_ops_s
{
	const char*         name;

	int               (*open )(scf_native_t* ctx, const char* arch);
	int               (*close)(scf_native_t* ctx);

	int               (*select_inst)(scf_native_t* ctx, scf_function_t* f);
};

struct scf_regs_ops_s
{
	const char*         name;

	uint32_t*           abi_regs;
	uint32_t*           abi_float_regs;
	uint32_t*           abi_ret_regs;
	uint32_t*           abi_caller_saves;
	uint32_t*           abi_callee_saves;

	const int           ABI_NB;
	const int           ABI_RET_NB;
	const int           ABI_CALLER_SAVES_NB;
	const int           ABI_CALLEE_SAVES_NB;

	const int           MAX_BYTES;

	int               (*registers_init  )();
	int               (*registers_reset )();
	void              (*registers_clear )();
	scf_vector_t*     (*register_colors )();

	int               (*reg_used        )(scf_register_t* r, scf_dag_node_t* dn);
	int               (*reg_cached_vars )(scf_register_t* r);

	int               (*variable_size   )(scf_variable_t* v);

	int               (*caller_save_regs)(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_t** saved_regs);
	int               (*pop_regs        )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t** regs, int nb_regs, scf_register_t** updated_regs, int nb_updated);

	scf_register_t*   (*find_register              )(const char* name);
	scf_register_t*   (*find_register_color        )(intptr_t color);
	scf_register_t*   (*find_register_color_bytes  )(intptr_t color, int bytes);
	scf_register_t*   (*find_register_type_id_bytes)(uint32_t type, uint32_t id, int bytes);

	scf_register_t*   (*select_overflowed_reg      )(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float);

	int               (*overflow_reg )(scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f);
	int               (*overflow_reg2)(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
	int               (*overflow_reg3)(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
};

struct scf_inst_ops_s
{
	const char*         name;

	int                (*BL      )(scf_3ac_code_t* c, scf_function_t* f, scf_function_t* pf);
	scf_instruction_t* (*BLR     )(scf_3ac_code_t* c, scf_register_t* r);
	scf_instruction_t* (*PUSH    )(scf_3ac_code_t* c, scf_register_t* r);
	scf_instruction_t* (*POP     )(scf_3ac_code_t* c, scf_register_t* r);
	scf_instruction_t* (*TEQ     )(scf_3ac_code_t* c, scf_register_t* rs);
	scf_instruction_t* (*NEG     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);

	scf_instruction_t* (*MOVZX   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size);
	scf_instruction_t* (*MOVSX   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size);
	scf_instruction_t* (*MVN     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*MOV_G   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*MOV_SP  )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);

	scf_instruction_t* (*ADD_G   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*ADD_IMM )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm);
	scf_instruction_t* (*SUB_G   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*SUB_IMM )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm);
	scf_instruction_t* (*CMP_G   )(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*CMP_IMM )(scf_3ac_code_t* c, scf_register_t* rs, uint64_t imm);
	scf_instruction_t* (*AND_G   )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*OR_G    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);

	scf_instruction_t* (*MUL     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*DIV     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*SDIV    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*MSUB    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rm, scf_register_t* rn, scf_register_t* ra);

	scf_instruction_t* (*SHL     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*SHR     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*ASR     )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);

	scf_instruction_t* (*CVTSS2SD)(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*CVTSD2SS)(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*CVTF2SI )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*CVTF2UI )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*CVTSI2F )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
	scf_instruction_t* (*CVTUI2F )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);

	scf_instruction_t* (*FCMP    )(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*FADD    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*FSUB    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*FMUL    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*FDIV    )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
	scf_instruction_t* (*FMOV_G  )(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);

	scf_instruction_t* (*JA      )(scf_3ac_code_t* c);
	scf_instruction_t* (*JB      )(scf_3ac_code_t* c);
	scf_instruction_t* (*JZ      )(scf_3ac_code_t* c);
	scf_instruction_t* (*JNZ     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JGT     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JGE     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JLT     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JLE     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JMP     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JAE     )(scf_3ac_code_t* c);
	scf_instruction_t* (*JBE     )(scf_3ac_code_t* c);
	scf_instruction_t* (*RET     )(scf_3ac_code_t* c);

	scf_instruction_t* (*SETZ    )(scf_3ac_code_t* c, scf_register_t* rd);
	scf_instruction_t* (*SETNZ   )(scf_3ac_code_t* c, scf_register_t* rd);
	scf_instruction_t* (*SETGT   )(scf_3ac_code_t* c, scf_register_t* rd);
	scf_instruction_t* (*SETGE   )(scf_3ac_code_t* c, scf_register_t* rd);
	scf_instruction_t* (*SETLT   )(scf_3ac_code_t* c, scf_register_t* rd);
	scf_instruction_t* (*SETLE   )(scf_3ac_code_t* c, scf_register_t* rd);

	int  (*I2G           )(scf_3ac_code_t* c, scf_register_t* rd, uint64_t imm, int bytes);
	int  (*M2G           )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs);
	int  (*M2GF          )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs);
	int  (*G2M           )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, scf_variable_t* vs);
	int  (*G2P           )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, int32_t offset, int size);
	int  (*P2G           )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset, int size);
	int  (*ISTR2G        )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs);
	int  (*SIB2G         )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);
	int  (*G2SIB         )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);
	int  (*ADR2G         )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs);
	int  (*ADRP2G        )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset);
	int  (*ADRSIB2G      )(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);

	int  (*cmp_update    )(scf_3ac_code_t* c, scf_function_t* f, scf_instruction_t* inst);
	int  (*set_rel_veneer)(scf_function_t* f);

	void (*set_jmp_offset)(scf_instruction_t* inst, int32_t bytes);
};

static inline int scf_inst_data_same(scf_inst_data_t* id0, scf_inst_data_t* id1)
{
	// global var, are considered as different.
	if ((id0->flag && !id0->base) || (id1->flag && !id1->base))
		return 0;

	if (id0->base == id1->base
			&& id0->scale == id1->scale
			&& id0->index == id1->index
			&& id0->disp  == id1->disp
			&& id0->flag  == id1->flag
			&& id0->imm   == id1->imm
			&& id0->imm_size == id1->imm_size)
		return 1;
	return 0;
}

void scf_instruction_print(scf_instruction_t* inst);

int scf_native_open(scf_native_t** pctx, const char* name);
int scf_native_close(scf_native_t* ctx);

int scf_native_select_inst(scf_native_t* ctx, scf_function_t* f);

int scf_native_write_elf(scf_native_t* ctx, const char* path, scf_function_t* f);

#endif

