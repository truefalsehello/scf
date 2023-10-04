#ifndef SCF_EDA_H
#define SCF_EDA_H

#include"scf_native.h"
#include"scf_eda_pb.h"

typedef struct {

	scf_function_t*     f;

} scf_eda_context_t;

typedef struct {
	int 	type;
	int		(*func)(scf_native_t* ctx, scf_3ac_code_t* c);
} eda_inst_handler_t;

eda_inst_handler_t* scf_eda_find_inst_handler(const int op_type);

int scf_eda_open  (scf_native_t* ctx, const char* arch);
int scf_eda_close (scf_native_t* ctx);
int scf_eda_select(scf_native_t* ctx);

static inline int eda_variable_size(scf_variable_t* v)
{
	if (v->nb_dimentions + v->nb_pointers > 0)
		return 64;

	if (v->type >= SCF_STRUCT)
		return 64;

	if (SCF_VAR_BIT == v->type)
		return 1;

	return v->size << 3;
}

#endif
