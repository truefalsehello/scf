#include"scf_eda.h"

#define EDA_INST_SRC_CHECK() \
	if (!c->srcs || c->srcs->size != 1) \
	return -EINVAL; \
	\
	scf_eda_context_t* eda  = ctx->priv; \
	scf_function_t*    f    = eda->f; \
	\
	scf_3ac_operand_t* src  = c->srcs->data[0]; \
	\
	if (!src || !src->dag_node) \
	return -EINVAL;

#define EDA_INST_DST_CHECK() \
	if (!c->dsts || c->dsts->size != 1) \
	return -EINVAL; \
	\
	scf_eda_context_t* eda  = ctx->priv; \
	scf_function_t*    f    = eda->f; \
	\
	scf_3ac_operand_t* dst  = c->dsts->data[0]; \
	\
	if (!dst || !dst->dag_node) \
	return -EINVAL;

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
			for (i = 0; i < _N; i++) { \
				if (!(_in)->pins[i]) \
					return -EINVAL; \
			} \
		} \
	} while (0)

#define EDA_PIN_ADD_INPUT(_in, _i, _ef, _p) \
		do { \
			ScfEcomponent* c = (_ef)->components[(_p)->cid]; \
			ScfEcomponent* R = NULL; \
			\
			if (!(_in)->pins[_i]) { \
				EDA_INST_ADD_COMPONENT(_ef, R, SCF_EDA_Resistor); \
				\
				EDA_PIN_ADD_PIN(c, (_p)->id, R, 0); \
				\
				(_in)->pins[_i] = R->pins[1]; \
			} else { \
				R = (_ef)->components[(_in)->pins[_i]->cid]; \
				\
				EDA_PIN_ADD_PIN(c, (_p)->id, R, 0); \
			} \
		} while (0)


static int __eda_bit_nand(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out)
{
	ScfEcomponent*  B  = f->ef->components[0];
	ScfEcomponent*  T0 = NULL;
	ScfEcomponent*  T1 = NULL;
	ScfEcomponent*  R  = NULL;

	EDA_INST_ADD_COMPONENT(f->ef, T0, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T1, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, R,  SCF_EDA_Resistor);

	EDA_PIN_ADD_PIN(R,  1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_C, R,  0);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_E, T1, SCF_EDA_NPN_C);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	*in0 = T0->pins[SCF_EDA_NPN_B];
	*in1 = T1->pins[SCF_EDA_NPN_B];
	*out = R ->pins[0];
	return 0;
}

static int __eda_bit_nor(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out)
{
	ScfEcomponent*  B  = f->ef->components[0];
	ScfEcomponent*  T0 = NULL;
	ScfEcomponent*  T1 = NULL;
	ScfEcomponent*  R  = NULL;

	EDA_INST_ADD_COMPONENT(f->ef, T0, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T1, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, R,  SCF_EDA_Resistor);

	EDA_PIN_ADD_PIN(R,  1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_C, R,  0);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_C, R,  0);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	*in0 = T0->pins[SCF_EDA_NPN_B];
	*in1 = T1->pins[SCF_EDA_NPN_B];
	*out = R ->pins[0];
	return 0;
}

static int __eda_bit_not(scf_function_t* f, ScfEpin** in, ScfEpin** out)
{
	ScfEcomponent*  B = f->ef->components[0];
	ScfEcomponent*  T = NULL;
	ScfEcomponent*  R = NULL;

	EDA_INST_ADD_COMPONENT(f->ef, R,  SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T,  SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R, 1,             B, SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T, SCF_EDA_NPN_C, R, 0);
	EDA_PIN_ADD_PIN(T, SCF_EDA_NPN_E, B, SCF_EDA_Battery_NEG);

	*in  = T->pins[SCF_EDA_NPN_B];
	*out = R->pins[0];
	return 0;
}

static int __eda_bit_and(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out)
{
	ScfEpin* nad0 = NULL;
	ScfEpin* nad1 = NULL;
	ScfEpin* nad_ = NULL;

	ScfEpin* not0 = NULL;
	ScfEpin* not_ = NULL;

	int ret = __eda_bit_nand(f, &nad0, &nad1, &nad_);
	if (ret < 0)
		return ret;

	ret = __eda_bit_not(f, &not0, &not_);
	if (ret < 0)
		return ret;

	EDA_PIN_ADD_PIN_EF(f->ef, not0, nad_);

	*in0 = nad0;
	*in1 = nad1;
	*out = not_;
	return 0;
}

static int __eda_bit_or(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out)
{
	ScfEpin* nor0 = NULL;
	ScfEpin* nor1 = NULL;
	ScfEpin* nor_ = NULL;

	ScfEpin* not0 = NULL;
	ScfEpin* not_ = NULL;

	int ret = __eda_bit_nor(f, &nor0, &nor1, &nor_);
	if (ret < 0)
		return ret;

	ret = __eda_bit_not(f, &not0, &not_);
	if (ret < 0)
		return ret;

	EDA_PIN_ADD_PIN_EF(f->ef, not0, nor_);

	*in0 = nor0;
	*in1 = nor1;
	*out = not_;
	return 0;
}

/*
   x + y =   ~(x &    y) &   (x |   y)
   x + y =    (x NAND y) &   (x |   y)
   x + y = ~(~(x NAND y) |  ~(x |   y))
   x + y =   ~(x NAND y) NOR (x NOR y))
   cf    =   ~(x NAND y)
 */
static int __eda_bit_add(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out, ScfEpin** cf)
{
	ScfEpin* nad0 = NULL;
	ScfEpin* nad1 = NULL;
	ScfEpin* nad_ = NULL;

	ScfEpin* not  = NULL;
	ScfEpin* cf_  = NULL;

	ScfEpin* nor0 = NULL;
	ScfEpin* nor1 = NULL;
	ScfEpin* nor_ = NULL;

	ScfEpin* nor2 = NULL;
	ScfEpin* nor3 = NULL;
	ScfEpin* out_ = NULL;

	int ret = __eda_bit_nand(f, &nad0, &nad1, &nad_);
	if (ret < 0)
		return ret;

	ret = __eda_bit_not(f, &not, &cf_);
	if (ret < 0)
		return ret;
	EDA_PIN_ADD_PIN_EF(f->ef, not, nad_);

	ret = __eda_bit_nor(f, &nor0, &nor1, &nor_);
	if (ret < 0)
		return ret;
	EDA_PIN_ADD_PIN_EF(f->ef, nor0, nad0);
	EDA_PIN_ADD_PIN_EF(f->ef, nor1, nad1);

	ret = __eda_bit_nor(f, &nor2, &nor3, &out_);
	if (ret < 0)
		return ret;
	EDA_PIN_ADD_PIN_EF(f->ef, nor2, cf_);
	EDA_PIN_ADD_PIN_EF(f->ef, nor3, nor_);

	*in0 = nad0;
	*in1 = nad1;
	*out = out_;
	*cf  = cf_;
	return 0;
}

static int __eda_bit_adc(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** in2, ScfEpin** out, ScfEpin** cf)
{
	ScfEcomponent*  B  = f->ef->components[0];
	ScfEcomponent*  R0 = NULL;
	ScfEcomponent*  R1 = NULL;
	ScfEcomponent*  R2 = NULL;
	ScfEcomponent*  R3 = NULL;
	ScfEcomponent*  R4 = NULL;

	ScfEcomponent*  T0 = NULL;
	ScfEcomponent*  T1 = NULL;

	ScfEcomponent*  T2 = NULL;
	ScfEcomponent*  T3 = NULL;
	ScfEcomponent*  T4 = NULL;
	ScfEcomponent*  T5 = NULL;
	ScfEcomponent*  T6 = NULL;

	ScfEcomponent*  T7 = NULL;

	ScfEcomponent*  T8 = NULL;
	ScfEcomponent*  T9 = NULL;

	ScfEcomponent*  T10 = NULL;
	ScfEcomponent*  T11 = NULL;
	ScfEcomponent*  T12 = NULL;

	// T0: 0, T1: 1
	EDA_INST_ADD_COMPONENT(f->ef, R0, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T0, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T1, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R0, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_C, R0, 0);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_C, T0, SCF_EDA_NPN_E);

	// T2: 1, T3: 2, T4: 0, T5: 1, T6: 0
	EDA_INST_ADD_COMPONENT(f->ef, R1, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T2, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T3, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T4, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T5, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T6, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R1, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T2, SCF_EDA_NPN_C, R1, 0);
	EDA_PIN_ADD_PIN(T3, SCF_EDA_NPN_C, T2, SCF_EDA_NPN_E);
	EDA_PIN_ADD_PIN(T3, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	EDA_PIN_ADD_PIN(T4, SCF_EDA_NPN_C, R1, 0);
	EDA_PIN_ADD_PIN(T4, SCF_EDA_NPN_E, T2, SCF_EDA_NPN_E);

	EDA_PIN_ADD_PIN(T5, SCF_EDA_NPN_C, R1, 0);
	EDA_PIN_ADD_PIN(T6, SCF_EDA_NPN_C, T5, SCF_EDA_NPN_E);
	EDA_PIN_ADD_PIN(T6, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	// T1: e
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_E, T2, SCF_EDA_NPN_E);

	// T7: cf
	EDA_INST_ADD_COMPONENT(f->ef, R2, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T7, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R2, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T7, SCF_EDA_NPN_C, R2, 0);
	EDA_PIN_ADD_PIN(T7, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);
	EDA_PIN_ADD_PIN(T7, SCF_EDA_NPN_B, R1, 0);

	// T8, T9: out
	EDA_INST_ADD_COMPONENT(f->ef, R3, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T8, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T9, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R3, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T8, SCF_EDA_NPN_C, R3, 0);
	EDA_PIN_ADD_PIN(T8, SCF_EDA_NPN_E, R1, 0);
	EDA_PIN_ADD_PIN(T8, SCF_EDA_NPN_B, R0, 0);

	EDA_PIN_ADD_PIN(T9, SCF_EDA_NPN_C, R3, 0);
	EDA_PIN_ADD_PIN(T9, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	// T10: 1, T11: 0, T12: 2
	EDA_INST_ADD_COMPONENT(f->ef, R4,  SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T10, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T11, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T12, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R4,  1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T10, SCF_EDA_NPN_C, R4, 0);
	EDA_PIN_ADD_PIN(T11, SCF_EDA_NPN_C, R4, 0);
	EDA_PIN_ADD_PIN(T12, SCF_EDA_NPN_C, R4, 0);

	EDA_PIN_ADD_PIN(T10, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);
	EDA_PIN_ADD_PIN(T11, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);
	EDA_PIN_ADD_PIN(T12, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	// T9: b
	EDA_PIN_ADD_PIN(T9,  SCF_EDA_NPN_B, R4, 0);

	// 0: T0, T4, T6, T11
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_B, T4,  SCF_EDA_NPN_B);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_B, T6,  SCF_EDA_NPN_B);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_B, T11, SCF_EDA_NPN_B);

	// 1: T1, T2, T5, T10
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_B, T2,  SCF_EDA_NPN_B);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_B, T5,  SCF_EDA_NPN_B);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_B, T10, SCF_EDA_NPN_B);

	// 2: T3, T12
	EDA_PIN_ADD_PIN(T3, SCF_EDA_NPN_B, T12, SCF_EDA_NPN_B);

	*in0 = T0->pins[SCF_EDA_NPN_B];
	*in1 = T1->pins[SCF_EDA_NPN_B];
	*in2 = T3->pins[SCF_EDA_NPN_B];
	*out = R3->pins[0];
	*cf  = R2->pins[0];
	return 0;
}

static int __eda_bit_sub0(scf_function_t* f, ScfEpin** in0, ScfEpin** in1, ScfEpin** out, ScfEpin** cf)
{
	ScfEcomponent*  B  = f->ef->components[0];
	ScfEcomponent*  R0 = NULL;
	ScfEcomponent*  R1 = NULL;
	ScfEcomponent*  R2 = NULL;
	ScfEcomponent*  R3 = NULL;

	ScfEcomponent*  T0 = NULL;
	ScfEcomponent*  T1 = NULL;
	ScfEcomponent*  T2 = NULL;
	ScfEcomponent*  T3 = NULL;

	ScfEcomponent*  T4 = NULL;
	ScfEcomponent*  T5 = NULL;

	// T0: 0, T1: 1
	EDA_INST_ADD_COMPONENT(f->ef, R0, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T0, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T1, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R0, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_C, R0, 0);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_C, T0, SCF_EDA_NPN_E);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	// T2: 1, T3: 0
	EDA_INST_ADD_COMPONENT(f->ef, R1, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T2, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T3, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R1, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T2, SCF_EDA_NPN_C, R1, 0);
	EDA_PIN_ADD_PIN(T3, SCF_EDA_NPN_C, R1, 0);
	EDA_PIN_ADD_PIN(T2, SCF_EDA_NPN_E, T3, SCF_EDA_NPN_E);
	EDA_PIN_ADD_PIN(T2, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);

	// cf
	EDA_INST_ADD_COMPONENT(f->ef, R2, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T4, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R2, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T4, SCF_EDA_NPN_C, R2, 0);
	EDA_PIN_ADD_PIN(T4, SCF_EDA_NPN_E, B,  SCF_EDA_Battery_NEG);
	EDA_PIN_ADD_PIN(T4, SCF_EDA_NPN_B, R1, 0);

	// out
	EDA_INST_ADD_COMPONENT(f->ef, R3, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T5, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R3, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T5, SCF_EDA_NPN_C, R3, 0);
	EDA_PIN_ADD_PIN(T5, SCF_EDA_NPN_E, R1, 0);
	EDA_PIN_ADD_PIN(T5, SCF_EDA_NPN_B, R0, 0);

	// 0: T0, T3
	// 1: T1, T2
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_B, T3, SCF_EDA_NPN_B);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_B, T2, SCF_EDA_NPN_B);

	*in0 = T0->pins[SCF_EDA_NPN_B];
	*in1 = T1->pins[SCF_EDA_NPN_B];
	*out = R3->pins[0];
	*cf  = R2->pins[0];
	return 0;
}

static int _eda_inst_bit_not_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP2_CHECK()

	scf_dag_node_t* in  = src->dag_node;
	scf_dag_node_t* out = dst->dag_node;

	int i;
	int N = eda_variable_size(in->var);

	EDA_INST_IN_CHECK(in, N);

	in ->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* pi = NULL;
		ScfEpin* po = NULL;

		int ret = __eda_bit_not(f, &pi, &po);
		if (ret < 0)
			return ret;

		EDA_PIN_ADD_INPUT(in, i, f->ef, pi);

		out->pins[i] = po;
		in ->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int _eda_inst_bit_and_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* p0 = NULL;
		ScfEpin* p1 = NULL;
		ScfEpin* po = NULL;

		int ret = __eda_bit_and(f, &p0, &p1, &po);
		if (ret < 0)
			return ret;

		EDA_PIN_ADD_INPUT(in0, i, f->ef, p0);
		EDA_PIN_ADD_INPUT(in1, i, f->ef, p1);

		out->pins[i] = po;
		in0->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int _eda_inst_bit_or_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* p0 = NULL;
		ScfEpin* p1 = NULL;
		ScfEpin* po = NULL;

		int ret = __eda_bit_or(f, &p0, &p1, &po);
		if (ret < 0)
			return ret;

		EDA_PIN_ADD_INPUT(in0, i, f->ef, p0);
		EDA_PIN_ADD_INPUT(in1, i, f->ef, p1);

		out->pins[i] = po;
		in0->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int _eda_inst_add_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	ScfEcomponent*  B   = f->ef->components[0];
	ScfEpin*        pc  = NULL;

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* p0  = NULL;
		ScfEpin* p1  = NULL;
		ScfEpin* p2  = NULL;
		ScfEpin* cf  = NULL;
		ScfEpin* res = NULL;

		if (i > 0) {
			int ret = __eda_bit_adc(f, &p0, &p1, &p2, &res, &cf);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_PIN_EF(f->ef, p2, pc);

		} else {
			int ret = __eda_bit_add(f, &p0, &p1, &res, &cf);
			if (ret < 0)
				return ret;
		}

		EDA_PIN_ADD_INPUT(in0, i, f->ef, p0);
		EDA_PIN_ADD_INPUT(in1, i, f->ef, p1);

		pc         = cf; // carry flag
		cf->flags |= SCF_EDA_PIN_CF;

		out->pins[i] = res; // result
		in0->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int _eda_inst_sub_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	ScfEcomponent*  B   = f->ef->components[0];
	ScfEpin*        pc  = NULL;

	int i;
	int N = eda_variable_size(in0->var);

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* p0  = NULL;
		ScfEpin* p1  = NULL;
		ScfEpin* p2  = NULL;
		ScfEpin* p3  = NULL;
		ScfEpin* not = NULL;
		ScfEpin* cf  = NULL;
		ScfEpin* res = NULL;

		int ret = __eda_bit_not(f, &p1, &not);
		if (ret < 0)
			return ret;

		if (i > 0) {
			ret = __eda_bit_adc(f, &p0, &p2, &p3, &res, &cf);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_PIN_EF(f->ef, p3, pc);
		} else {
			ret = __eda_bit_sub0(f, &p0, &p2, &res, &cf);
			if (ret < 0)
				return ret;
		}

		EDA_PIN_ADD_PIN_EF(f->ef, p2, not);

		EDA_PIN_ADD_INPUT(in0, i, f->ef, p0);
		EDA_PIN_ADD_INPUT(in1, i, f->ef, p1);

		pc         = cf;
		cf->flags |= SCF_EDA_PIN_CF;

		out->pins[i] = res;
		in0->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int __eda_bit_mla(scf_function_t* f, ScfEpin** a, ScfEpin** b, ScfEpin** c, ScfEpin** d, ScfEpin** out, ScfEpin** cf)
{
	ScfEcomponent* B  = f->ef->components[0];
	ScfEcomponent* R0 = NULL;
	ScfEcomponent* T0 = NULL;
	ScfEcomponent* T1 = NULL;

	ScfEpin* a0   = NULL;
	ScfEpin* b0   = NULL;
	ScfEpin* ab0  = NULL;

	ScfEpin* c0   = NULL;
	ScfEpin* d0   = NULL;
	ScfEpin* cd0  = NULL;

	ScfEpin* ab1  = NULL;
	ScfEpin* cd1  = NULL;
	ScfEpin* abcd = NULL;

	ScfEpin* cf0  = NULL;
	ScfEpin* cf1  = NULL;

	ScfEpin* and0 = NULL;
	ScfEpin* and1 = NULL;
	ScfEpin* res  = NULL;

/* (c + 2a)(b + 2d) = bc + 2(ab + cd) + 4ad

B0 = bc
B1 = ab + cd = (a & b) + (c & d)

CF = (a & b) & (c & d) = ~NAND(a, b, c, d)

B1 = (a & b) ^ (c & d) = ((a & b) | (c & d)) & ~(a & b & c & d)
                       = ((a & b) | (c & d)) & NAND(a, b, c, d)

     (a & b) | (c & d) = ~(~(a  &   b)  &  ~(c  &   d))
                       = ~( (a NAND b)  &   (c NAND d))
					   =    (a NAND b) NAND (c NAND d)

B1 = [(a NAND b) NAND (c NAND d)] & NAND(a, b, c, d)
 */
	int ret = __eda_bit_nand(f, &a0, &b0, &ab0);
	if (ret < 0)
		return ret;

	ret = __eda_bit_nand(f, &c0, &d0, &cd0);
	if (ret < 0)
		return ret;

	ret = __eda_bit_nand(f, &ab1, &cd1, &abcd);
	if (ret < 0)
		return ret;

	EDA_PIN_ADD_PIN_EF(f->ef, ab0, ab1);
	EDA_PIN_ADD_PIN_EF(f->ef, cd0, cd1);

	EDA_INST_ADD_COMPONENT(f->ef, R0, SCF_EDA_Resistor);
	EDA_INST_ADD_COMPONENT(f->ef, T0, SCF_EDA_NPN);
	EDA_INST_ADD_COMPONENT(f->ef, T1, SCF_EDA_NPN);

	EDA_PIN_ADD_PIN(R0, 1,             B,  SCF_EDA_Battery_POS);
	EDA_PIN_ADD_PIN(T0, SCF_EDA_NPN_C, R0, 0);
	EDA_PIN_ADD_PIN(T1, SCF_EDA_NPN_C, T0, SCF_EDA_NPN_E);

	EDA_PIN_ADD_PIN_EF(f->ef, a0,  T0->pins[SCF_EDA_NPN_B]);
	EDA_PIN_ADD_PIN_EF(f->ef, b0,  T1->pins[SCF_EDA_NPN_B]);
	EDA_PIN_ADD_PIN_EF(f->ef, cd0, T1->pins[SCF_EDA_NPN_E]);

	ret = __eda_bit_not(f, &cf0, &cf1);
	if (ret < 0)
		return ret;
	EDA_PIN_ADD_PIN_EF(f->ef, cf0, R0->pins[0]);

	ret = __eda_bit_and(f, &and0, &and1, &res);
	if (ret < 0)
		return ret;

	EDA_PIN_ADD_PIN_EF(f->ef, and0, abcd);
	EDA_PIN_ADD_PIN_EF(f->ef, and1, cf0);

	*a   = a0;
	*b   = b0;
	*c   = c0;
	*d   = d0;
	*out = res;
	*cf  = cf1;
	return 0;
}

static int _eda_inst_mul_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	EDA_INST_OP3_CHECK()

	scf_dag_node_t* in0 = src0->dag_node;
	scf_dag_node_t* in1 = src1->dag_node;
	scf_dag_node_t* out = dst ->dag_node;

	ScfEpin*  adds[256] = {NULL};
	ScfEpin*  cfs [256] = {NULL};

	int n_adds = 0;
	int n_cfs  = 0;

	int N = eda_variable_size(in0->var);
	int i;

	EDA_INST_IN_CHECK(in0, N);
	EDA_INST_IN_CHECK(in1, N);

	in0->n_pins = N;
	in1->n_pins = N;
	out->n_pins = N;

	for (i = 0; i < N; i++) {

		ScfEpin* p0j = NULL;
		ScfEpin* p0k = NULL;
		ScfEpin* p1j = NULL;
		ScfEpin* p1k = NULL;
		ScfEpin* res = NULL;
		ScfEpin* cf  = NULL;

		if (0 == i) {
			int ret = __eda_bit_and(f, &p0j, &p1k, &res);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_INPUT(in0, 0, f->ef, p0j);
			EDA_PIN_ADD_INPUT(in1, 0, f->ef, p1k);

			out->pins[0]         = res;
			in0->pins[0]->flags |= SCF_EDA_PIN_IN0;
			continue;
		}

		int j = 0;
		int k = i;

		while (j < k) {
			int ret = __eda_bit_mla(f, &p0j, &p1k, &p0k, &p1j, &adds[n_adds], &cfs[n_cfs]);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_INPUT(in0, j, f->ef, p0j);
			EDA_PIN_ADD_INPUT(in0, k, f->ef, p0k);

			EDA_PIN_ADD_INPUT(in1, j, f->ef, p1j);
			EDA_PIN_ADD_INPUT(in1, k, f->ef, p1k);

			cfs[n_cfs]->flags |= SCF_EDA_PIN_CF;

			in0->pins[j]->flags |= SCF_EDA_PIN_IN0;
			in0->pins[k]->flags |= SCF_EDA_PIN_IN0;

			n_adds++;
			n_cfs++;

			j++;
			k--;
		}

		if (j == k) {
			int ret = __eda_bit_and(f, &p0j, &p1k, &adds[n_adds]);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_INPUT(in0, j, f->ef, p0j);
			EDA_PIN_ADD_INPUT(in1, j, f->ef, p1k);

			in0->pins[j]->flags |= SCF_EDA_PIN_IN0;

			n_adds++;
		}

		while (n_adds > 1) {
			j = 0;
			k = n_adds - 1;

			while (j < k) {
				p0j = NULL;
				p1k = NULL;

				int ret = __eda_bit_add(f, &p0j, &p1k, &res, &cfs[n_cfs]);
				if (ret < 0)
					return ret;

				EDA_PIN_ADD_PIN_EF(f->ef, p0j, adds[j]);
				EDA_PIN_ADD_PIN_EF(f->ef, p1k, adds[k]);

				cfs[n_cfs++]->flags |= SCF_EDA_PIN_CF;

				adds[j] = res;

				j++;
				k--;
			}

			if (j == k)
				n_adds = j + 1;
			else
				n_adds = j;
			scf_logd("i: %d, n_adds: %d, j: %d, k: %d\n\n", i, n_adds, j, k);
		}

		out->pins[i] = adds[0];

		for (j = 0; j < n_cfs; j++)
			adds[j] = cfs[j];

		n_adds = n_cfs;
		n_cfs  = 0;
	}

	return 0;
}

static int _eda_inst_div_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return -EINVAL;
}

static int _eda_inst_inc_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	scf_loge("\n");
	EDA_INST_OP2_CHECK()

	scf_dag_node_t* in  = src->dag_node;
	scf_dag_node_t* out = dst->dag_node;
	ScfEpin*        pc  = NULL;

	int i;
	int N = eda_variable_size(out->var);

	EDA_INST_IN_CHECK(in,  N);
	EDA_INST_IN_CHECK(out, N);

	in ->n_pins = N;
	out->n_pins = N;

	scf_loge("in: %p, out: %p\n", in, out);

	for (i = 0; i < N; i++) {

		ScfEpin* p0  = NULL;
		ScfEpin* p1  = NULL;
		ScfEpin* cf  = NULL;
		ScfEpin* res = NULL;

		if (i > 0) {
			int ret = __eda_bit_add(f, &p0, &p1, &res, &cf);
			if (ret < 0)
				return ret;

			EDA_PIN_ADD_PIN_EF(f->ef, p1, pc);

		} else {
			int ret = __eda_bit_not(f, &p0, &res);
			if (ret < 0)
				return ret;

			cf = p0;
		}

		EDA_PIN_ADD_INPUT(in, i, f->ef, p0);

		pc         = cf;
		cf->flags |= SCF_EDA_PIN_CF;

		out->pins[i] = res;
		in ->pins[i]->flags |= SCF_EDA_PIN_IN0;
	}

	return 0;
}

static int _eda_inst_dec_handler(scf_native_t* ctx, scf_3ac_code_t* c)
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
			scf_loge("out: %p\n", out);
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

	{SCF_OP_BIT_AND,        _eda_inst_bit_and_handler},
	{SCF_OP_BIT_OR,         _eda_inst_bit_or_handler},

	{SCF_OP_ADD,            _eda_inst_add_handler},
	{SCF_OP_SUB,            _eda_inst_sub_handler},
	{SCF_OP_MUL,            _eda_inst_mul_handler},
	{SCF_OP_DIV,            _eda_inst_div_handler},

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
