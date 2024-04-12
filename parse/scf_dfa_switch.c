#include"scf_dfa.h"
#include"scf_dfa_util.h"
#include"scf_parse.h"
#include"scf_stack.h"

extern scf_dfa_module_t dfa_module_switch;

typedef struct {
	int              nb_lps;
	int              nb_rps;

	scf_block_t*     parent_block;
	scf_node_t*      parent_node;

	scf_node_t*      _switch;
	scf_node_t*      child;

} dfa_switch_data_t;

static int _switch_is_end(scf_dfa_t* dfa, void* word)
{
	return 1;
}

static int _switch_action_switch(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = data;
	scf_lex_word_t*   w     = words->data[words->size - 1];
	scf_stack_t*      s     = d->module_datas[dfa_module_switch.index];

	scf_node_t*     _switch = scf_node_alloc(w, SCF_OP_SWITCH, NULL);
	if (!_switch) {
		scf_loge("node alloc failed\n");
		return SCF_DFA_ERROR;
	}

	dfa_switch_data_t* sd = calloc(1, sizeof(dfa_switch_data_t));
	if (!sd) {
		scf_loge("module data alloc failed\n");
		return SCF_DFA_ERROR;
	}

	if (d->current_node)
		scf_node_add_child(d->current_node, _switch);
	else
		scf_node_add_child((scf_node_t*)parse->ast->current_block, _switch);

	sd->_switch      = _switch;
	sd->parent_block = parse->ast->current_block;
	sd->parent_node  = d->current_node;
	d->current_node  = _switch;

	scf_stack_push(s, sd);

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_lp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;

	assert(!d->expr);
	d->expr_local_flag = 1;

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_lp_stat(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	dfa_parse_data_t*  d  = data;
	scf_stack_t*       s  = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd = scf_stack_top(s);

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_lp_stat"), SCF_DFA_HOOK_POST);

	sd->nb_lps++;

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_rp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd    = scf_stack_top(s);

	if (!d->expr) {
		scf_loge("\n");
		return SCF_DFA_ERROR;
	}

	sd->nb_rps++;

	if (sd->nb_rps == sd->nb_lps) {

		assert(0 == sd->_switch->nb_nodes);

		scf_node_add_child(sd->_switch, d->expr);
		d->expr = NULL;
		d->expr_local_flag = 0;

		SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_end"), SCF_DFA_HOOK_END);

		return SCF_DFA_SWITCH_TO;
	}

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "switch_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_case(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_lex_word_t*    w     = words->data[words->size - 1];
	scf_stack_t*       s     = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd    = scf_stack_top(s);

	assert(!d->expr);
	d->expr_local_flag = 1;

	sd->child = scf_node_alloc(w, SCF_OP_CASE, NULL);
	if (!sd->child)
		return SCF_DFA_ERROR;

	scf_node_add_child((scf_node_t*)parse->ast->current_block, sd->child);

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_default(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_lex_word_t*    w     = words->data[words->size - 1];
	scf_stack_t*       s     = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd    = scf_stack_top(s);

	assert(!d->expr);

	sd->child = scf_node_alloc(w, SCF_OP_DEFAULT, NULL);
	if (!sd->child)
		return SCF_DFA_ERROR;

	scf_node_add_child((scf_node_t*)parse->ast->current_block, sd->child);

	return SCF_DFA_NEXT_WORD;
}

static int _switch_action_colon(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd    = scf_stack_top(s);

	if (SCF_OP_CASE == sd->child->type) {

		if (!d->expr) {
			scf_loge("NOT found the expr for case\n");
			return SCF_DFA_ERROR;
		}

		scf_node_add_child(sd->child, d->expr);
		d->expr = NULL;
		d->expr_local_flag = 0;

	} else {
		assert(SCF_OP_DEFAULT == sd->child->type);
		assert(!d->expr);
	}

	return SCF_DFA_OK;
}

static int _switch_action_end(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_switch.index];
	dfa_switch_data_t* sd    = scf_stack_pop(s);

	assert(parse->ast->current_block == sd->parent_block);

	d->current_node = sd->parent_node;

	scf_logi("\033[31m switch: %d, sd: %p, s->size: %d\033[0m\n", sd->_switch->w->line, sd, s->size);

	free(sd);
	sd = NULL;

	assert(s->size >= 0);

	return SCF_DFA_OK;
}

static int _dfa_init_module_switch(scf_dfa_t* dfa)
{
	SCF_DFA_MODULE_NODE(dfa, switch, lp,        scf_dfa_is_lp,      _switch_action_lp);
	SCF_DFA_MODULE_NODE(dfa, switch, rp,        scf_dfa_is_rp,      _switch_action_rp);
	SCF_DFA_MODULE_NODE(dfa, switch, lp_stat,   scf_dfa_is_lp,      _switch_action_lp_stat);
	SCF_DFA_MODULE_NODE(dfa, switch, colon,     scf_dfa_is_colon,   _switch_action_colon);

	SCF_DFA_MODULE_NODE(dfa, switch, _switch,   scf_dfa_is_switch,  _switch_action_switch);
	SCF_DFA_MODULE_NODE(dfa, switch, _case,     scf_dfa_is_case,    _switch_action_case);
	SCF_DFA_MODULE_NODE(dfa, switch, _default,  scf_dfa_is_default, _switch_action_default);
	SCF_DFA_MODULE_NODE(dfa, switch, end,       _switch_is_end,     _switch_action_end);

	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_switch.index];

	assert(!s);

	s = scf_stack_alloc();
	if (!s) {
		scf_logi("\n");
		return SCF_DFA_ERROR;
	}

	d->module_datas[dfa_module_switch.index] = s;

	return SCF_DFA_OK;
}

static int _dfa_fini_module_switch(scf_dfa_t* dfa)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_switch.index];

	if (s) {
		scf_stack_free(s);
		s = NULL;
		d->module_datas[dfa_module_switch.index] = NULL;
	}

	return SCF_DFA_OK;
}

static int _dfa_init_syntax_switch(scf_dfa_t* dfa)
{
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   lp,        lp);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   rp,        rp);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   lp_stat,   lp_stat);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   colon,     colon);

	SCF_DFA_GET_MODULE_NODE(dfa, switch,   _switch,   _switch);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   _case,     _case);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   _default,  _default);
	SCF_DFA_GET_MODULE_NODE(dfa, switch,   end,       end);

	SCF_DFA_GET_MODULE_NODE(dfa, expr,  entry,     expr);
	SCF_DFA_GET_MODULE_NODE(dfa, block, entry,     block);

	scf_dfa_node_add_child(_switch,  lp);
	scf_dfa_node_add_child(lp,       expr);
	scf_dfa_node_add_child(expr,     rp);
	scf_dfa_node_add_child(rp,       block);

	scf_dfa_node_add_child(_case,    expr);
	scf_dfa_node_add_child(expr,     colon);
	scf_dfa_node_add_child(_default, colon);

	scf_logi("\n");
	return 0;
}

scf_dfa_module_t dfa_module_switch =
{
	.name        = "switch",

	.init_module = _dfa_init_module_switch,
	.init_syntax = _dfa_init_syntax_switch,

	.fini_module = _dfa_fini_module_switch,
};
