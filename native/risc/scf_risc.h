#ifndef SCF_RISC_H
#define SCF_RISC_H

#include"scf_native.h"
#include"scf_risc_util.h"
#include"scf_risc_reg.h"
#include"scf_risc_opcode.h"
#include"scf_graph.h"
#include"scf_elf.h"

#define RISC_INST_ADD_CHECK(vec, inst) \
			do { \
				if (!(inst)) { \
					scf_loge("\n"); \
					return -ENOMEM; \
				} \
				int ret = scf_vector_add((vec), (inst)); \
				if (ret < 0) { \
					scf_loge("\n"); \
					free(inst); \
					return ret; \
				} \
			} while (0)

#define RISC_RELA_ADD_CHECK(vec, rela, c, v, f) \
	do { \
		rela = calloc(1, sizeof(scf_rela_t)); \
		if (!rela) \
			return -ENOMEM; \
		\
		(rela)->code = (c); \
		(rela)->var  = (v); \
		(rela)->func = (f); \
		(rela)->inst = (c)->instructions->data[(c)->instructions->size - 1]; \
		(rela)->addend = 0; \
		\
		int ret = scf_vector_add((vec), (rela)); \
		if (ret < 0) { \
			free(rela); \
			rela = NULL; \
			return ret; \
		} \
	} while (0)

#define RISC_PEEPHOLE_DEL 1
#define RISC_PEEPHOLE_OK  0

typedef struct {

	scf_function_t*     f;

} scf_risc_context_t;

typedef struct {
	scf_dag_node_t*      dag_node;

	scf_register_t*  reg;

	scf_risc_OpCode_t*    OpCode;

} risc_rcg_node_t;

typedef struct {
	int 	type;
	int		(*func)(scf_native_t* ctx, scf_3ac_code_t* c, scf_graph_t* g);
} risc_rcg_handler_t;

typedef struct {
	int 	type;
	int		(*func)(scf_native_t* ctx, scf_3ac_code_t* c);
} risc_inst_handler_t;

risc_rcg_handler_t*  scf_risc_find_rcg_handler(const int op_type);
risc_inst_handler_t* scf_risc_find_inst_handler(const int op_type);

int  risc_rcg_find_node(scf_graph_node_t** pp, scf_graph_t* g, scf_dag_node_t* dn, scf_register_t* reg);
int _risc_rcg_make_node(scf_graph_node_t** pp, scf_graph_t* g, scf_dag_node_t* dn, scf_register_t* reg);

int scf_risc_open  (scf_native_t* ctx, const char* arch);
int scf_risc_close (scf_native_t* ctx);
int scf_risc_select(scf_native_t* ctx);

int risc_optimize_peephole(scf_native_t* ctx, scf_function_t* f);

int scf_risc_graph_kcolor(scf_graph_t* graph, int k, scf_vector_t* colors);


intptr_t risc_bb_find_color (scf_vector_t* dn_colors, scf_dag_node_t* dn);
int      risc_save_bb_colors(scf_vector_t* dn_colors, scf_bb_group_t* bbg, scf_basic_block_t* bb);

int risc_bb_load_dn (intptr_t color, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_basic_block_t* bb, scf_function_t* f);
int risc_bb_save_dn (intptr_t color, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_basic_block_t* bb, scf_function_t* f);
int risc_bb_load_dn2(intptr_t color, scf_dag_node_t* dn, scf_basic_block_t* bb, scf_function_t* f);
int risc_bb_save_dn2(intptr_t color, scf_dag_node_t* dn, scf_basic_block_t* bb, scf_function_t* f);

int  risc_fix_bb_colors  (scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
int  risc_load_bb_colors (scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
int  risc_load_bb_colors2(scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
void risc_init_bb_colors (scf_basic_block_t* bb, scf_function_t* f);


scf_instruction_t* risc_make_inst         (scf_3ac_code_t* c, uint32_t opcode);
scf_instruction_t* risc_make_inst_BL      (scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_BLR     (scf_3ac_code_t* c, scf_register_t* r);
scf_instruction_t* risc_make_inst_PUSH    (scf_3ac_code_t* c, scf_register_t* r);
scf_instruction_t* risc_make_inst_POP     (scf_3ac_code_t* c, scf_register_t* r);
scf_instruction_t* risc_make_inst_TEQ     (scf_3ac_code_t* c, scf_register_t* rs);
scf_instruction_t* risc_make_inst_NEG     (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_MSUB    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rm, scf_register_t* rn, scf_register_t* ra);
scf_instruction_t* risc_make_inst_MUL     (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_SDIV    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_DIV     (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_FDIV    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_FMUL    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_FSUB    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_FADD    (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_SUB_G   (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_ADD_G   (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_ADD_IMM (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm);
scf_instruction_t* risc_make_inst_SUB_IMM (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm);
scf_instruction_t* risc_make_inst_CMP_IMM(scf_3ac_code_t* c, scf_register_t* rs, uint64_t imm);
scf_instruction_t* risc_make_inst_CVTSS2SD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_MOVZX   (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size);
scf_instruction_t* risc_make_inst_MOVSX   (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size);
scf_instruction_t* risc_make_inst_FMOV_G  (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_MVN     (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_MOV_G   (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_MOV_SP  (scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_AND_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_OR_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_CMP_G(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_SHL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_SHR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);
scf_instruction_t* risc_make_inst_ASR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1);

scf_instruction_t* risc_make_inst_CVTSS2SD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_CVTSD2SS(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_CVTF2SI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_CVTF2UI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_CVTSI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_CVTUI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs);
scf_instruction_t* risc_make_inst_FCMP(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1);

scf_instruction_t* risc_make_inst_JA (scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JB (scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JZ (scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JNZ(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JGT(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JGE(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JLT(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JLE(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JMP(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JAE(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_JBE(scf_3ac_code_t* c);
scf_instruction_t* risc_make_inst_RET(scf_3ac_code_t* c);

scf_instruction_t* risc_make_inst_SETZ (scf_3ac_code_t* c, scf_register_t* rd);
scf_instruction_t* risc_make_inst_SETNZ(scf_3ac_code_t* c, scf_register_t* rd);
scf_instruction_t* risc_make_inst_SETGT(scf_3ac_code_t* c, scf_register_t* rd);
scf_instruction_t* risc_make_inst_SETGE(scf_3ac_code_t* c, scf_register_t* rd);
scf_instruction_t* risc_make_inst_SETLT(scf_3ac_code_t* c, scf_register_t* rd);
scf_instruction_t* risc_make_inst_SETLE(scf_3ac_code_t* c, scf_register_t* rd);

void risc_set_jmp_offset(scf_instruction_t* inst, int32_t bytes);

int risc_make_inst_I2G   (scf_3ac_code_t* c, scf_register_t* rd, uint64_t imm, int bytes);
int risc_make_inst_M2G   (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs);
int risc_make_inst_M2GF  (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs);
int risc_make_inst_G2M   (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, scf_variable_t* vs);
int risc_make_inst_G2P   (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, int32_t offset, int size);
int risc_make_inst_P2G   (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset, int size);
int risc_make_inst_ISTR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs);
int risc_make_inst_SIB2G (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);
int risc_make_inst_G2SIB (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);
int risc_make_inst_ADR2G (scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs);
int risc_make_inst_ADRP2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset);
int risc_make_inst_ADRSIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib);

int risc_rcg_make(scf_3ac_code_t* c, scf_graph_t* g, scf_dag_node_t* dn, scf_register_t* reg);

#endif

