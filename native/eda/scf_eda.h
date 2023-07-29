#ifndef SCF_EDA_H
#define SCF_EDA_H

#include"scf_native.h"
#include"scf_eda_pb.h"

#define EDA_INST_ADD_COMPONENT(_f, _c, _type) \
	do { \
		_c = scf_ecomponent__alloc(_type); \
		if (!_c) \
			return -ENOMEM; \
		\
		(_c)->id = (_f)->ef->n_components; \
		\
		int ret = scf_efunction__add_component((_f)->ef, _c); \
		if (ret < 0) { \
			scf_ecomponent__free(_c); \
			_c = NULL; \
			return ret; \
		} \
		\
		for (size_t i = 0;  i < (_c)->n_pins; i++) \
			(_c)->pins[i]->cid = (_c)->id; \
	} while (0)

#define EDA_PIN_ADD_COMPONENT(_pin, _cid, _pid) \
	do { \
		int ret = scf_epin__add_component((_pin), _cid, _pid); \
		if (ret < 0) \
			return ret; \
	} while (0)

#define EDA_PIN_ADD_PIN(_c0, _pid0, _c1, _pid1) \
	do { \
		int ret = scf_epin__add_component((_c0)->pins[_pid0], (_c1)->id, (_pid1)); \
		if (ret < 0) \
			return ret; \
		\
		ret = scf_epin__add_component((_c1)->pins[_pid1], (_c0)->id, (_pid0)); \
		if (ret < 0) \
			return ret; \
	} while (0)

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
