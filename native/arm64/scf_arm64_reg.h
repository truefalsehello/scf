#ifndef SCF_ARM64_REG_H
#define SCF_ARM64_REG_H

#include"scf_native.h"
#include"scf_arm64_util.h"

#define ARM64_COLOR(type, id, mask)   ((type) << 24 | (id) << 16 | (mask))
#define ARM64_COLOR_TYPE(c)           ((c) >> 24)
#define ARM64_COLOR_ID(c)             (((c) >> 16) & 0xff)
#define ARM64_COLOR_MASK(c)           ((c) & 0xffff)
#define ARM64_COLOR_CONFLICT(c0, c1)  ( (c0) >> 16 == (c1) >> 16 && (c0) & (c1) & 0xffff )

#define ARM64_COLOR_BYTES(c) \
	({ \
	     int n = 0;\
	     intptr_t minor = (c) & 0xffff; \
	     while (minor) { \
	         minor &= minor - 1; \
	         n++;\
	     } \
	     n;\
	 })

#define ARM64_SELECT_REG_CHECK(pr, dn, c, f, load_flag) \
	do {\
		int ret = arm64_select_reg(pr, dn, c, f, load_flag); \
		if (ret < 0) { \
			scf_loge("\n"); \
			return ret; \
		} \
		assert(dn->color > 0); \
	} while (0)

// ABI: rdi rsi rdx rcx r8 r9
static uint32_t arm64_abi_regs[] =
{
	SCF_ARM64_REG_X0,
	SCF_ARM64_REG_X1,
	SCF_ARM64_REG_X2,
	SCF_ARM64_REG_X3,
	SCF_ARM64_REG_X4,
	SCF_ARM64_REG_X5,
	SCF_ARM64_REG_X6,
	SCF_ARM64_REG_X7,
};

static uint32_t arm64_abi_float_regs[] =
{
	SCF_ARM64_REG_D0,
	SCF_ARM64_REG_D1,
	SCF_ARM64_REG_D2,
	SCF_ARM64_REG_D3,
	SCF_ARM64_REG_D4,
	SCF_ARM64_REG_D5,
	SCF_ARM64_REG_D6,
	SCF_ARM64_REG_D7,
};
#define ARM64_ABI_NB (sizeof(arm64_abi_regs) / sizeof(arm64_abi_regs[0]))

static uint32_t arm64_abi_ret_regs[] =
{
	SCF_ARM64_REG_X0,
	SCF_ARM64_REG_X1,
	SCF_ARM64_REG_X2,
	SCF_ARM64_REG_X3,
};
#define ARM64_ABI_RET_NB (sizeof(arm64_abi_ret_regs) / sizeof(arm64_abi_ret_regs[0]))

static uint32_t arm64_abi_caller_saves[] =
{
	SCF_ARM64_REG_X0,
	SCF_ARM64_REG_X1,
	SCF_ARM64_REG_X2,
	SCF_ARM64_REG_X3,
	SCF_ARM64_REG_X4,
	SCF_ARM64_REG_X5,
	SCF_ARM64_REG_X6,
	SCF_ARM64_REG_X7,

	SCF_ARM64_REG_X9,
	SCF_ARM64_REG_X10,
	SCF_ARM64_REG_X11,
	SCF_ARM64_REG_X12,
	SCF_ARM64_REG_X13,
	SCF_ARM64_REG_X14,
	SCF_ARM64_REG_X15,
};
#define ARM64_ABI_CALLER_SAVES_NB (sizeof(arm64_abi_caller_saves) / sizeof(arm64_abi_caller_saves[0]))

static uint32_t arm64_abi_callee_saves[] =
{
	SCF_ARM64_REG_X19,
	SCF_ARM64_REG_X20,
	SCF_ARM64_REG_X21,
	SCF_ARM64_REG_X22,
	SCF_ARM64_REG_X23,
	SCF_ARM64_REG_X24,
	SCF_ARM64_REG_X25,
	SCF_ARM64_REG_X26,
	SCF_ARM64_REG_X27,
	SCF_ARM64_REG_X28,
	SCF_ARM64_REG_X29,
	SCF_ARM64_REG_X30,
};
#define ARM64_ABI_CALLEE_SAVES_NB (sizeof(arm64_abi_callee_saves) / sizeof(arm64_abi_callee_saves[0]))

typedef struct {
	uint32_t		id;
	int				bytes;
	char*			name;

	intptr_t        color;

	scf_vector_t*	dag_nodes;

	uint32_t        updated;
	uint32_t        used;
} scf_register_arm64_t;

typedef struct {
	scf_register_arm64_t* base;
	scf_register_arm64_t* index;

	int32_t             scale;
	int32_t             disp;
	int32_t             size;
} arm64_sib_t;

static inline int arm64_variable_size(scf_variable_t* v)
{
	if (v->nb_dimentions > 0)
		return 8;

	if (v->type >= SCF_STRUCT && 0 == v->nb_pointers)
		return 8;

	return v->size < 4 ? 4 : v->size;
}

typedef int         (*arm64_sib_fill_pt)(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_registers_init();
int                 arm64_registers_reset();
void                arm64_registers_clear();
scf_vector_t*       arm64_register_colors();

scf_register_arm64_t*	arm64_find_register(const char* name);

scf_register_arm64_t* arm64_find_register_type_id_bytes(uint32_t type, uint32_t id, int bytes);

scf_register_arm64_t* arm64_find_register_color(intptr_t color);

scf_register_arm64_t* arm64_find_register_color_bytes(intptr_t color, int bytes);

scf_register_arm64_t*	arm64_find_abi_register(int index, int bytes);

scf_register_arm64_t* arm64_select_overflowed_reg(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float);

int                 arm64_reg_cached_vars(scf_register_arm64_t* r);

int                 arm64_save_var(scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_save_var2(scf_dag_node_t* dn, scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_push_regs(scf_vector_t* instructions, uint32_t* regs, int nb_regs);
int                 arm64_pop_regs(scf_3ac_code_t* c, scf_register_arm64_t** regs, int nb_regs, scf_register_arm64_t** updated_regs, int nb_updated);

int                 arm64_caller_save_regs(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_arm64_t** saved_regs);

int                 arm64_save_reg(scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_load_reg(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
int                 arm64_reg_used(scf_register_arm64_t* r, scf_dag_node_t* dn);

int                 arm64_overflow_reg (scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f);
int                 arm64_overflow_reg2(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_select_reg(scf_register_arm64_t** preg, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f, int load_flag);

int                 arm64_select_free_reg(scf_register_arm64_t** preg, scf_3ac_code_t* c, scf_function_t* f, int is_float);

int                 arm64_dereference_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_pointer_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 arm64_array_index_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_dag_node_t* scale, scf_3ac_code_t* c, scf_function_t* f);

void                arm64_call_rabi(int* p_nints, int* p_nfloats, scf_3ac_code_t* c);


static inline int   arm64_inst_data_is_reg(scf_inst_data_t* id)
{
	scf_register_t* fp = (scf_register_t*)arm64_find_register("fp");
	scf_register_t* sp = (scf_register_t*)arm64_find_register("sp");

	if (!id->flag && id->base && id->base != sp && id->base != fp && 0 == id->imm_size)
		return 1;
	return 0;
}

static inline int   arm64_inst_data_is_local(scf_inst_data_t* id)
{
	scf_register_t* fp = (scf_register_t*)arm64_find_register("fp");
	scf_register_t* sp = (scf_register_t*)arm64_find_register("sp");

	if (id->flag && (id->base == fp || id->base == sp))
		return 1;
	return 0;
}

static inline int   arm64_inst_data_is_global(scf_inst_data_t* id)
{
	if (id->flag && !id->base)
		return 1;
	return 0;
}

static inline int   arm64_inst_data_is_const(scf_inst_data_t* id)
{
	if (!id->flag && id->imm_size > 0)
		return 1;
	return 0;
}

#endif

