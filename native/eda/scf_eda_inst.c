#include"scf_eda.h"

#define EDA_INST_OP2_CHECK() \
	if (!c->dsts || c->dsts->size != 1) \
		return -EINVAL; \
	\
	if (!c->srcs || c->srcs->size != 1) \
		return -EINVAL; \
	\
	scf_eda_context_t* eda  = ctx->priv; \
	scf_function_t*    f    = eda->f; \
	\
	scf_3ac_operand_t* dst  = c->dsts->data[0]; \
	scf_3ac_operand_t* src  = c->srcs->data[0]; \
	\
	if (!src || !src->dag_node) \
		return -EINVAL; \
	\
	if (!dst || !dst->dag_node) \
		return -EINVAL; \
	\
	if (src->dag_node->var->size != dst->dag_node->var->size) {\
		scf_loge("size: %d, %d\n", src->dag_node->var->size, dst->dag_node->var->size); \
		return -EINVAL; \
	}

#define EDA_INST_OP3_CHECK() \
	if (!c->dsts || c->dsts->size != 1) \
		return -EINVAL; \
	\
	if (!c->srcs || c->srcs->size != 2) \
		return -EINVAL; \
	\
	scf_eda_context_t* eda  = ctx->priv; \
	scf_function_t*    f    = eda->f; \
	\
	scf_3ac_operand_t* dst  = c->dsts->data[0]; \
	scf_3ac_operand_t* src0 = c->srcs->data[0]; \
	scf_3ac_operand_t* src1 = c->srcs->data[1]; \
	\
	if (!src0 || !src0->dag_node) \
		return -EINVAL; \
	\
	if (!src1 || !src1->dag_node) \
		return -EINVAL; \
	\
	if (!dst || !dst->dag_node) \
		return -EINVAL; \
	\
	if (src0->dag_node->var->size != src1->dag_node->var->size) {\
		scf_loge("size: %d, %d\n", src0->dag_node->var->size, src1->dag_node->var->size); \
		return -EINVAL; \
	}

#define EDA_INST_IN_CHECK(_in, _N) \
	do { \
		int i; \
		if (!(_in)->var->arg_flag) { \
			if ((_in)->n_pins != _N) \
				return -EINVAL; \
			\
			for (i = 0; i < N; i++) { \
				if (!(_in)->pins[i]) \
					return -EINVAL; \
			} \
		} \
	} while (0)

#define EDA_PIN_ADD_INPUT(_in, _i, _c, _pid) \
		do { \
			if (!(_in)->pins[_i]) \
				 (_in)->pins[_i] = (_c)->pins[_pid]; \
			else { \
				EDA_PIN_ADD_COMPONENT((_in)->pins[_i],   (_c)->id, _pid); \
				EDA_PIN_ADD_COMPONENT((_c )->pins[_pid], (_in)->pins[_i]->cid, (_in)->pins[_i]->id); \
			} \
		} while (0)


static int _eda_inst_bit_not_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP2_CHECK()

	scf_dag_node_t* in  = src->dag_node;
	scf_dag_node_t* out = dst->dag_node;

	ScfEcomponent*  T   = NULL;
	ScfEcomponent*  R   = NULL;
	ScfEcomponent*  B   = f->ef->components[0];

	int i;
	int N = eda_variable_size(in->var);

	EDA_INST_IN_CHECK(in, N);

	in ->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		EDA_INST_ADD_COMPONENT(f, T, SCF_EDA_Transistor);
		EDA_INST_ADD_COMPONENT(f, R, SCF_EDA_Resistor);

		EDA_PIN_ADD_INPUT(in, i, T, SCF_EDA_Transistor_B);

		EDA_PIN_ADD_PIN(T, SCF_EDA_Transistor_C, R, 1);
		EDA_PIN_ADD_PIN(T, SCF_EDA_Transistor_E, B, SCF_EDA_Battery_NEG);

		EDA_PIN_ADD_PIN(R, 0, B, SCF_EDA_Battery_POS);

		out->pins[i] = T->pins[SCF_EDA_Transistor_C];
	}

	return 0;
}

static int _eda_inst_bit_and_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	ScfEcomponent*  D0  = NULL;
	ScfEcomponent*  D1  = NULL;
	ScfEcomponent*  R   = NULL;
	ScfEcomponent*  B   = f->ef->components[0];

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		EDA_INST_ADD_COMPONENT(f, D0, SCF_EDA_Diode);
		EDA_INST_ADD_COMPONENT(f, D1, SCF_EDA_Diode);
		EDA_INST_ADD_COMPONENT(f, R,  SCF_EDA_Resistor);

		EDA_PIN_ADD_INPUT(in0, i, D0, SCF_EDA_Diode_NEG);
		EDA_PIN_ADD_INPUT(in1, i, D1, SCF_EDA_Diode_NEG);

		EDA_PIN_ADD_PIN(D0, SCF_EDA_Diode_POS, D1, SCF_EDA_Diode_POS);
		EDA_PIN_ADD_PIN(D0, SCF_EDA_Diode_POS, R,  1);
		EDA_PIN_ADD_PIN(D1, SCF_EDA_Diode_POS, R,  1);

		EDA_PIN_ADD_PIN(R,  0, B, SCF_EDA_Battery_POS);

		out->pins[i] = R ->pins[1];
	}

	return 0;
}

static int _eda_inst_bit_or_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	ScfEcomponent*  D0  = NULL;
	ScfEcomponent*  D1  = NULL;
	ScfEcomponent*  R   = NULL;
	ScfEcomponent*  B   = f->ef->components[0];

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		EDA_INST_ADD_COMPONENT(f, D0, SCF_EDA_Diode);
		EDA_INST_ADD_COMPONENT(f, D1, SCF_EDA_Diode);
		EDA_INST_ADD_COMPONENT(f, R,  SCF_EDA_Resistor);

		EDA_PIN_ADD_INPUT(in0, i, D0, SCF_EDA_Diode_POS);
		EDA_PIN_ADD_INPUT(in1, i, D1, SCF_EDA_Diode_POS);

		EDA_PIN_ADD_PIN(D0, SCF_EDA_Diode_NEG, D1, SCF_EDA_Diode_NEG);
		EDA_PIN_ADD_PIN(D0, SCF_EDA_Diode_NEG, R,  0);
		EDA_PIN_ADD_PIN(D1, SCF_EDA_Diode_NEG, R,  0);

		EDA_PIN_ADD_PIN(R, 1, B, SCF_EDA_Battery_NEG);

		out->pins[i] = R ->pins[0];
	}

	return 0;
}

static int _eda_inst_inc_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_inc_post_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_dec_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_dec_post_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 4)
		return -EINVAL;

	scf_eda_context_t*  eda    = ctx->priv;
	scf_function_t*     f      = eda->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];
	scf_3ac_operand_t*  src    = c->srcs->data[3];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	return -EINVAL;
}

static int _eda_inst_and_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 4)
		return -EINVAL;

	scf_eda_context_t*  eda    = ctx->priv;
	scf_function_t*     f      = eda->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];
	scf_3ac_operand_t*  src    = c->srcs->data[3];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	return -EINVAL;
}

static int _eda_inst_or_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 4)
		return -EINVAL;

	scf_eda_context_t*  eda    = ctx->priv;
	scf_function_t*     f      = eda->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];
	scf_3ac_operand_t*  src    = c->srcs->data[3];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	return -EINVAL;
}

static int _eda_inst_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_eda_context_t*  eda    = ctx->priv;
	scf_function_t*     f      = eda->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*     vd  = dst  ->dag_node->var;
	scf_variable_t*     vb  = base ->dag_node->var;
	scf_variable_t*     vi  = index->dag_node->var;
	scf_variable_t*     vs  = scale->dag_node->var;

	return -EINVAL;
}

static int _eda_inst_teq_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_logic_not_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_cmp_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setz_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setnz_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setgt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setge_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setlt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_setle_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_eq_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_ne_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_gt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_ge_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_lt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_le_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_cast_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_eda_context_t* eda = ctx->priv;
	scf_function_t*    f   = eda->f;
	scf_3ac_operand_t* src = c->srcs->data[0];
	scf_3ac_operand_t* dst = c->dsts->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (0 == dst->dag_node->color)
		return -EINVAL;

	return -EINVAL;
}

static int _eda_inst_return_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size < 1)
		return -EINVAL;

	scf_eda_context_t*  eda  = ctx->priv;
	scf_function_t*     f    = eda->f;
	scf_3ac_operand_t*  src  = NULL;
	scf_dag_node_t*     out  = NULL;

	int i;
	int j;

	for (i  = 0; i < c->srcs->size; i++) {
		src =        c->srcs->data[i];

		out = src->dag_node;

		if (out->n_pins <= 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		for (j  = 0; j < out->n_pins; j++)
			out->pins[j]->flags |= SCF_EDA_PIN_OUT;
	}

	return 0;
}

static int _eda_inst_nop_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_end_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_goto_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jz_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jnz_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jgt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jge_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jlt_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_jle_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_load_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_reload_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_save_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_and_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _eda_inst_or_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static eda_inst_handler_t eda_inst_handlers[] =
{
	{SCF_OP_ARRAY_INDEX, 	_eda_inst_array_index_handler},

	{SCF_OP_TYPE_CAST,      _eda_inst_cast_handler},
	{SCF_OP_LOGIC_NOT, 		_eda_inst_logic_not_handler},
	{SCF_OP_BIT_NOT,        _eda_inst_bit_not_handler},

	{SCF_OP_INC,            _eda_inst_inc_handler},
	{SCF_OP_DEC,            _eda_inst_dec_handler},

	{SCF_OP_INC_POST,       _eda_inst_inc_post_handler},
	{SCF_OP_DEC_POST,       _eda_inst_dec_post_handler},

	{SCF_OP_BIT_AND,        _eda_inst_bit_and_handler},
	{SCF_OP_BIT_OR,         _eda_inst_bit_or_handler},

	{SCF_OP_3AC_TEQ,        _eda_inst_teq_handler},
	{SCF_OP_3AC_CMP,        _eda_inst_cmp_handler},

	{SCF_OP_3AC_SETZ,       _eda_inst_setz_handler},
	{SCF_OP_3AC_SETNZ,      _eda_inst_setnz_handler},
	{SCF_OP_3AC_SETGT,      _eda_inst_setgt_handler},
	{SCF_OP_3AC_SETGE,      _eda_inst_setge_handler},
	{SCF_OP_3AC_SETLT,      _eda_inst_setlt_handler},
	{SCF_OP_3AC_SETLE,      _eda_inst_setle_handler},

	{SCF_OP_EQ, 			_eda_inst_eq_handler},
	{SCF_OP_NE, 			_eda_inst_ne_handler},
	{SCF_OP_GT, 			_eda_inst_gt_handler},
	{SCF_OP_GE,             _eda_inst_ge_handler},
	{SCF_OP_LT,             _eda_inst_lt_handler},
	{SCF_OP_LE,             _eda_inst_le_handler},

	{SCF_OP_ASSIGN, 		_eda_inst_assign_handler},

	{SCF_OP_AND_ASSIGN,     _eda_inst_and_assign_handler},
	{SCF_OP_OR_ASSIGN,      _eda_inst_or_assign_handler},

	{SCF_OP_RETURN, 		_eda_inst_return_handler},
	{SCF_OP_GOTO, 			_eda_inst_goto_handler},

	{SCF_OP_3AC_JZ, 		_eda_inst_jz_handler},
	{SCF_OP_3AC_JNZ, 		_eda_inst_jnz_handler},
	{SCF_OP_3AC_JGT, 		_eda_inst_jgt_handler},
	{SCF_OP_3AC_JGE, 		_eda_inst_jge_handler},
	{SCF_OP_3AC_JLT, 		_eda_inst_jlt_handler},
	{SCF_OP_3AC_JLE, 		_eda_inst_jle_handler},

	{SCF_OP_3AC_NOP, 		_eda_inst_nop_handler},
	{SCF_OP_3AC_END, 		_eda_inst_end_handler},

	{SCF_OP_3AC_SAVE,       _eda_inst_save_handler},
	{SCF_OP_3AC_LOAD,       _eda_inst_load_handler},

	{SCF_OP_3AC_RESAVE,     _eda_inst_save_handler},
	{SCF_OP_3AC_RELOAD,     _eda_inst_reload_handler},

	{SCF_OP_3AC_ASSIGN_ARRAY_INDEX,     _eda_inst_assign_array_index_handler},
	{SCF_OP_3AC_AND_ASSIGN_ARRAY_INDEX, _eda_inst_and_assign_array_index_handler},
	{SCF_OP_3AC_OR_ASSIGN_ARRAY_INDEX,  _eda_inst_or_assign_array_index_handler},
};

eda_inst_handler_t* scf_eda_find_inst_handler(const int op_type)
{
	int i;
	for (i = 0; i < sizeof(eda_inst_handlers) / sizeof(eda_inst_handlers[0]); i++) {

		eda_inst_handler_t* h = &(eda_inst_handlers[i]);

		if (op_type == h->type)
			return h;
	}
	return NULL;
}
