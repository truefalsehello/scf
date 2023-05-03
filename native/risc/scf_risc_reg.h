#ifndef SCF_RISC_REG_H
#define SCF_RISC_REG_H

#include"scf_native.h"
#include"scf_risc_util.h"

#define RISC_COLOR(type, id, mask)   ((type) << 24 | (id) << 16 | (mask))
#define RISC_COLOR_TYPE(c)           ((c) >> 24)
#define RISC_COLOR_ID(c)             (((c) >> 16) & 0xff)
#define RISC_COLOR_MASK(c)           ((c) & 0xffff)
#define RISC_COLOR_CONFLICT(c0, c1)  ( (c0) >> 16 == (c1) >> 16 && (c0) & (c1) & 0xffff )

#define RISC_COLOR_BYTES(c) \
	({ \
	     int n = 0;\
	     intptr_t minor = (c) & 0xffff; \
	     while (minor) { \
	         minor &= minor - 1; \
	         n++;\
	     } \
	     n;\
	 })

#define RISC_SELECT_REG_CHECK(pr, dn, c, f, load_flag) \
	do {\
		int ret = risc_select_reg(pr, dn, c, f, load_flag); \
		if (ret < 0) { \
			scf_loge("\n"); \
			return ret; \
		} \
		assert(dn->color > 0); \
	} while (0)

static uint32_t risc_abi_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
	SCF_RISC_REG_X4,
	SCF_RISC_REG_X5,
	SCF_RISC_REG_X6,
	SCF_RISC_REG_X7,
};

static uint32_t risc_abi_float_regs[] =
{
	SCF_RISC_REG_D0,
	SCF_RISC_REG_D1,
	SCF_RISC_REG_D2,
	SCF_RISC_REG_D3,
	SCF_RISC_REG_D4,
	SCF_RISC_REG_D5,
	SCF_RISC_REG_D6,
	SCF_RISC_REG_D7,
};
#define RISC_ABI_NB (sizeof(risc_abi_regs) / sizeof(risc_abi_regs[0]))

static uint32_t risc_abi_ret_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
};
#define RISC_ABI_RET_NB (sizeof(risc_abi_ret_regs) / sizeof(risc_abi_ret_regs[0]))

static uint32_t risc_abi_caller_saves[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
	SCF_RISC_REG_X4,
	SCF_RISC_REG_X5,
	SCF_RISC_REG_X6,
	SCF_RISC_REG_X7,

	SCF_RISC_REG_X9,
	SCF_RISC_REG_X10,
	SCF_RISC_REG_X11,
	SCF_RISC_REG_X12,
	SCF_RISC_REG_X13,
	SCF_RISC_REG_X14,
	SCF_RISC_REG_X15,
};
#define RISC_ABI_CALLER_SAVES_NB (sizeof(risc_abi_caller_saves) / sizeof(risc_abi_caller_saves[0]))

static uint32_t risc_abi_callee_saves[] =
{
	SCF_RISC_REG_X19,
	SCF_RISC_REG_X20,
	SCF_RISC_REG_X21,
	SCF_RISC_REG_X22,
	SCF_RISC_REG_X23,
	SCF_RISC_REG_X24,
	SCF_RISC_REG_X25,
	SCF_RISC_REG_X26,
	SCF_RISC_REG_X27,
	SCF_RISC_REG_X28,
	SCF_RISC_REG_X29,
	SCF_RISC_REG_X30,
};
#define RISC_ABI_CALLEE_SAVES_NB (sizeof(risc_abi_callee_saves) / sizeof(risc_abi_callee_saves[0]))

static inline int risc_variable_size(scf_variable_t* v)
{
	if (v->nb_dimentions > 0)
		return 8;

	if (v->type >= SCF_STRUCT && 0 == v->nb_pointers)
		return 8;

	return v->size < 4 ? 4 : v->size;
}

typedef int         (*risc_sib_fill_pt)(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_registers_init();
int                 risc_registers_reset();
void                risc_registers_clear();
scf_vector_t*       risc_register_colors();

scf_register_t*     risc_find_register(const char* name);

scf_register_t*     risc_find_register_type_id_bytes(uint32_t type, uint32_t id, int bytes);

scf_register_t*     risc_find_register_color(intptr_t color);

scf_register_t*     risc_find_register_color_bytes(intptr_t color, int bytes);

scf_register_t*	    risc_find_abi_register(int index, int bytes);

scf_register_t*     risc_select_overflowed_reg(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float);

int                 risc_reg_cached_vars(scf_register_t* r);

int                 risc_save_var(scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_save_var2(scf_dag_node_t* dn, scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_pop_regs(scf_3ac_code_t* c, scf_function_t* f, scf_register_t** regs, int nb_regs, scf_register_t** updated_regs, int nb_updated);

int                 risc_caller_save_regs(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_t** saved_regs);

int                 risc_save_reg(scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_load_const(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
int                 risc_load_reg  (scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
int                 risc_reg_used  (scf_register_t* r, scf_dag_node_t* dn);

int                 risc_overflow_reg (scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f);
int                 risc_overflow_reg2(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
int                 risc_overflow_reg3(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_select_reg(scf_register_t** preg, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f, int load_flag);

int                 risc_select_free_reg(scf_register_t** preg, scf_3ac_code_t* c, scf_function_t* f, int is_float);

int                 risc_dereference_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_pointer_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_array_index_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_dag_node_t* scale, scf_3ac_code_t* c, scf_function_t* f);

void                risc_call_rabi(int* p_nints, int* p_nfloats, scf_3ac_code_t* c, scf_function_t* f);

static inline int   risc_inst_data_is_reg(scf_inst_data_t* id)
{
	scf_register_t* fp = (scf_register_t*)risc_find_register("fp");
	scf_register_t* sp = (scf_register_t*)risc_find_register("sp");

	if (!id->flag && id->base && id->base != sp && id->base != fp && 0 == id->imm_size)
		return 1;
	return 0;
}

static inline int   risc_inst_data_is_local(scf_inst_data_t* id)
{
	scf_register_t* fp = (scf_register_t*)risc_find_register("fp");
	scf_register_t* sp = (scf_register_t*)risc_find_register("sp");

	if (id->flag && (id->base == fp || id->base == sp))
		return 1;
	return 0;
}

static inline int   risc_inst_data_is_global(scf_inst_data_t* id)
{
	if (id->flag && !id->base)
		return 1;
	return 0;
}

static inline int   risc_inst_data_is_const(scf_inst_data_t* id)
{
	if (!id->flag && id->imm_size > 0)
		return 1;
	return 0;
}

#endif

