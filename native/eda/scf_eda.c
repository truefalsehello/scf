#include"scf_eda.h"
#include"scf_basic_block.h"
#include"scf_3ac.h"

static char* component_types[SCF_EDA_Components_NB] =
{
    "None",
    "Battery",
    "Resistor",
    "Capacitor",
    "Inductor",
    "Diode",
    "Transistor",
};

int	scf_eda_open(scf_native_t* ctx, const char* arch)
{
	scf_eda_context_t* eda = calloc(1, sizeof(scf_eda_context_t));
	if (!eda)
		return -ENOMEM;

	ctx->priv = eda;
	return 0;
}

int scf_eda_close(scf_native_t* ctx)
{
	scf_eda_context_t* eda = ctx->priv;

	if (eda) {
		free(eda);
		eda = NULL;
	}
	return 0;
}

static int _eda_make_insts_for_list(scf_native_t* ctx, scf_list_t* h, int bb_offset)
{
	scf_list_t* l;
	int ret;

	for (l = scf_list_head(h); l != scf_list_sentinel(h); l = scf_list_next(l)) {

		scf_3ac_code_t* c = scf_list_data(l, scf_3ac_code_t, list);

		eda_inst_handler_t* h = scf_eda_find_inst_handler(c->op->type);
		if (!h) {
			scf_loge("3ac operator '%s' not supported\n", c->op->name);
			return -EINVAL;
		}

		ret = h->func(ctx, c);
		if (ret < 0) {
			scf_3ac_code_print(c, NULL);
			scf_loge("3ac op '%s' make inst failed\n", c->op->name);
			return ret;
		}

		if (!c->instructions)
			continue;

		scf_3ac_code_print(c, NULL);
	}

	return bb_offset;
}

int	_scf_eda_select_inst(scf_native_t* ctx)
{
	scf_eda_context_t*	eda = ctx->priv;
	scf_function_t*     f   = eda->f;
	scf_basic_block_t*  bb;
	scf_bb_group_t*     bbg;

	int i;
	int j;
	int ret = 0;

	for (i  = 0; i < f->bb_groups->size; i++) {
		bbg =        f->bb_groups->data[i];

		for (j = 0; j < bbg->body->size; j++) {
			bb =        bbg->body->data[j];

			assert(!bb->native_flag);

			scf_loge("************ bb: %d\n", bb->index);
			ret = _eda_make_insts_for_list(ctx, &bb->code_list_head, 0);
			if (ret < 0)
				return ret;
			bb->native_flag = 1;
			scf_loge("************ bb: %d\n", bb->index);
		}
	}

	for (i  = 0; i < f->bb_loops->size; i++) {
		bbg =        f->bb_loops->data[i];

		for (j = 0; j < bbg->body->size; j++) {
			bb =        bbg->body->data[j];

			assert(!bb->native_flag);

			ret = _eda_make_insts_for_list(ctx, &bb->code_list_head, 0);
			if (ret < 0)
				return ret;
			bb->native_flag = 1;
		}
	}

	return 0;
}

int scf_eda_select_inst(scf_native_t* ctx, scf_function_t* f)
{
	scf_eda_context_t* eda = ctx->priv;
	ScfEcomponent*     B   = NULL;

	scf_dag_node_t*    dn;
	scf_list_t*        l;

	eda->f = f;

	assert(!f->ef);

	f->ef = scf_efunction__alloc(f->node.w->text->data);
	if (!f->ef)
		return -ENOMEM;

	EDA_INST_ADD_COMPONENT(f, B, SCF_EDA_Battery);

	B->pins[SCF_EDA_Battery_NEG]->flags = SCF_EDA_PIN_NEG;
	B->pins[SCF_EDA_Battery_POS]->flags = SCF_EDA_PIN_POS;

	int ret = _scf_eda_select_inst(ctx);
	if (ret < 0)
		return ret;

	for (l = scf_list_head(&f->dag_list_head); l != scf_list_sentinel(&f->dag_list_head); l = scf_list_next(l)) {

		dn = scf_list_data(l, scf_dag_node_t, list);

		if (dn->var && dn->var->arg_flag) {

			int i;
			for (i = 0; i < dn->n_pins; i++)
				dn->pins[i]->flags |= SCF_EDA_PIN_IN;
		}
	}

#if 1
	ScfEcomponent* c;
	ScfEpin*       p;
	size_t i;
	size_t j;
	size_t k;

	for (i = 0; i < f->ef->n_components; i++) {
		c  =        f->ef->components[i];

		printf("c: %ld, type: %s\n", c->id, component_types[c->type]);

		for (j = 0; j < c->n_pins; j++) {
			p  =        c->pins[j];

			printf("cid: %ld, pid: %ld, flags: %#lx\n", p->cid, p->id, p->flags);

			for (k = 0; k + 1 < p->n_tos; k += 2)
				printf("to cid: %ld, pid: %ld\n", p->tos[k], p->tos[k + 1]);
		}
		printf("\n");
	}
#endif
	return 0;
}

scf_native_ops_t	native_ops_eda = {
	.name            = "eda",

	.open            = scf_eda_open,
	.close           = scf_eda_close,

	.select_inst     = scf_eda_select_inst,
};

