#ifndef SCF_RISC_REG_H
#define SCF_RISC_REG_H

#include"scf_native.h"
#include"scf_risc_util.h"

#define RISC_COLOR(type, id, mask)   ((type) << 24 | (id) << 16 | (mask))
#define RISC_COLOR_TYPE(c)           ((c) >> 24)
#define RISC_COLOR_ID(c)             (((c) >> 16) & 0xff)
#define RISC_COLOR_MASK(c)           ((c) & 0xffff)
//#define RISC_COLOR_CONFLICT(c0, c1)  ( (c0) >> 16 == (c1) >> 16 && (c0) & (c1) & 0xffff )

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

#define RISC_ABI_CALLER_SAVES_MAX 32
#define RISC_ABI_RET_MAX          8

int                 risc_save_var (scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_save_var2(scf_dag_node_t* dn, scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_save_reg (scf_register_t* r,  scf_3ac_code_t* c, scf_function_t* f);

int                 risc_load_const(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);
int                 risc_load_reg  (scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_select_reg(scf_register_t** preg, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f, int load_flag);

int                 risc_select_free_reg(scf_register_t** preg, scf_3ac_code_t* c, scf_function_t* f, int is_float);

int                 risc_dereference_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_pointer_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f);

int                 risc_array_index_reg(scf_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_dag_node_t* scale, scf_3ac_code_t* c, scf_function_t* f);

void                risc_call_rabi(int* p_nints, int* p_nfloats, scf_3ac_code_t* c, scf_function_t* f);

#endif

