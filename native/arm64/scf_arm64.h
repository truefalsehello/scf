#ifndef SCF_ARM64_H
#define SCF_ARM64_H

#include"scf_native.h"
#include"scf_arm64_util.h"
#include"scf_arm64_reg.h"
#include"scf_arm64_opcode.h"
#include"scf_graph.h"
#include"scf_elf.h"

#define ARM64_INST_ADD_CHECK(vec, inst) \
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

#define ARM64_RELA_ADD_CHECK(vec, rela, c, v, f) \
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

#define ARM64_PEEPHOLE_DEL 1
#define ARM64_PEEPHOLE_OK  0

typedef struct {

	scf_function_t*     f;

} scf_arm64_context_t;

typedef struct {
	scf_dag_node_t*      dag_node;

	scf_register_arm64_t*  reg;

	scf_arm64_OpCode_t*    OpCode;

} arm64_rcg_node_t;

typedef struct {
	int 	type;
	int		(*func)(scf_native_t* ctx, scf_3ac_code_t* c, scf_graph_t* g);
} arm64_rcg_handler_t;

typedef struct {
	int 	type;
	int		(*func)(scf_native_t* ctx, scf_3ac_code_t* c);
} arm64_inst_handler_t;

arm64_rcg_handler_t*  scf_arm64_find_rcg_handler(const int op_type);
arm64_inst_handler_t* scf_arm64_find_inst_handler(const int op_type);

int arm64_rcg_find_node(scf_graph_node_t** pp, scf_graph_t* g, scf_dag_node_t* dn, scf_register_arm64_t* reg);
int _arm64_rcg_make_node(scf_graph_node_t** pp, scf_graph_t* g, scf_dag_node_t* dn, scf_register_arm64_t* reg, scf_arm64_OpCode_t* OpCode);

int scf_arm64_open(scf_native_t* ctx);
int scf_arm64_close(scf_native_t* ctx);
int scf_arm64_select(scf_native_t* ctx);

int arm64_optimize_peephole(scf_native_t* ctx, scf_function_t* f);

int scf_arm64_graph_kcolor(scf_graph_t* graph, int k, scf_vector_t* colors);


intptr_t arm64_bb_find_color (scf_vector_t* dn_colors, scf_dag_node_t* dn);
int      arm64_save_bb_colors(scf_vector_t* dn_colors, scf_bb_group_t* bbg, scf_basic_block_t* bb);

int arm64_bb_load_dn (intptr_t color, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_basic_block_t* bb, scf_function_t* f);
int arm64_bb_save_dn (intptr_t color, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_basic_block_t* bb, scf_function_t* f);
int arm64_bb_load_dn2(intptr_t color, scf_dag_node_t* dn, scf_basic_block_t* bb, scf_function_t* f);
int arm64_bb_save_dn2(intptr_t color, scf_dag_node_t* dn, scf_basic_block_t* bb, scf_function_t* f);

int  arm64_fix_bb_colors  (scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
int  arm64_load_bb_colors (scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
int  arm64_load_bb_colors2(scf_basic_block_t* bb, scf_bb_group_t* bbg, scf_function_t* f);
void arm64_init_bb_colors (scf_basic_block_t* bb);


scf_instruction_t* arm64_make_inst_G(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r);
scf_instruction_t* arm64_make_inst_E(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r);
scf_instruction_t* arm64_make_inst_I(scf_arm64_OpCode_t* OpCode, uint8_t* imm, int size);
void               arm64_make_inst_I2(scf_instruction_t* inst, scf_arm64_OpCode_t* OpCode, uint8_t* imm, int size);

scf_instruction_t* arm64_make_inst_I2E(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_dst, uint8_t* imm, int size);

scf_instruction_t* arm64_make_inst_M ( scf_rela_t** prela, scf_arm64_OpCode_t* OpCode, scf_variable_t* v,         scf_register_arm64_t* r_base);
scf_instruction_t* arm64_make_inst_I2M(scf_rela_t** prela, scf_arm64_OpCode_t* OpCode, scf_variable_t* v_dst,     scf_register_arm64_t* r_base, uint8_t* imm, int32_t size);

scf_instruction_t* arm64_make_inst_G2E(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_dst, scf_register_arm64_t* r_src);
scf_instruction_t* arm64_make_inst_E2G(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_dst, scf_register_arm64_t* r_src);

scf_instruction_t* arm64_make_inst_I2P(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_base, int32_t offset, uint8_t* imm, int size);

scf_instruction_t* arm64_make_inst_I2SIB(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_base, scf_register_arm64_t* r_index, int32_t scale, int32_t disp, uint8_t* imm, int32_t size);

scf_instruction_t* arm64_make_inst_SIB(scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_base,  scf_register_arm64_t* r_index, int32_t scale, int32_t disp, int size);
scf_instruction_t* arm64_make_inst_P(  scf_arm64_OpCode_t* OpCode, scf_register_arm64_t* r_base, int32_t offset, int size);

int arm64_float_OpCode_type(int OpCode_type, int var_type);


int arm64_shift(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_shift_assign(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);


int arm64_binary_assign(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_binary_assign_dereference(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_binary_assign_pointer(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_binary_assign_array_index(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_unary_assign_dereference(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_unary_assign_pointer(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_unary_assign_array_index(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);


int arm64_inst_int_mul(scf_dag_node_t* dst, scf_dag_node_t* src, scf_3ac_code_t* c, scf_function_t* f);
int arm64_inst_int_div(scf_dag_node_t* dst, scf_dag_node_t* src, scf_3ac_code_t* c, scf_function_t* f, int mod_flag);

int arm64_inst_pointer(scf_native_t* ctx, scf_3ac_code_t* c, int lea_flag);
int arm64_inst_dereference(scf_native_t* ctx, scf_3ac_code_t* c);

int arm64_inst_float_cast(scf_dag_node_t* dst, scf_dag_node_t* src, scf_3ac_code_t* c, scf_function_t* f);

int arm64_inst_movx(scf_dag_node_t* dst, scf_dag_node_t* src, scf_3ac_code_t* c, scf_function_t* f);

int arm64_inst_op2(int OpCode_type, scf_dag_node_t* dst, scf_dag_node_t* src, scf_3ac_code_t* c, scf_function_t* f);

int arm64_inst_jmp(scf_native_t* ctx, scf_3ac_code_t* c, int OpCode_type);

int arm64_inst_teq(scf_native_t* ctx, scf_3ac_code_t* c);
int arm64_inst_cmp(scf_native_t* ctx, scf_3ac_code_t* c);
int arm64_inst_set(scf_native_t* ctx, scf_3ac_code_t* c, int setcc_type);

int arm64_inst_cmp_set(scf_native_t* ctx, scf_3ac_code_t* c, int setcc_type);


scf_instruction_t* arm64_make_inst(scf_3ac_code_t* c, uint32_t opcode);

int arm64_make_inst_I2G   (scf_3ac_code_t* c, scf_register_arm64_t* rd, uint64_t imm, int bytes);
int arm64_make_inst_M2G   (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, scf_variable_t* vs);
int arm64_make_inst_G2M   (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rs, scf_register_arm64_t* rb, scf_variable_t* vs);
int arm64_make_inst_G2P   (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rs, scf_register_arm64_t* rb, int32_t offset, int size);
int arm64_make_inst_P2G   (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, int32_t offset, int size);
int arm64_make_inst_ISTR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_variable_t* vs);
int arm64_make_inst_SIB2G (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, arm64_sib_t* sib);
int arm64_make_inst_G2SIB (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, arm64_sib_t* sib);
int arm64_make_inst_ADR2G (scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_variable_t* vs);

#endif

