#include"scf_dfa.h"
#include"scf_dfa_util.h"
#include"scf_parse.h"
#include"scf_stack.h"

extern scf_dfa_module_t dfa_module_do;

typedef struct {
	int              nb_lps;
	int              nb_rps;

	scf_block_t*     parent_block;
	scf_node_t*      parent_node;

	scf_node_t*      _do;

} dfa_do_data_t;


static int _do_action_do(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = data;
	scf_lex_word_t*   w     = words->data[words->size - 1];
	scf_stack_t*      s     = d->module_datas[dfa_module_do.index];
	scf_block_t*      b     = NULL;

	scf_node_t* _do = scf_node_alloc(w, SCF_OP_DO, NULL);
	if (!_do) {
		scf_loge("node alloc failed\n");
		return SCF_DFA_ERROR;
	}

	dfa_do_data_t* dd = calloc(1, sizeof(dfa_do_data_t));
	if (!dd) {
		scf_loge("module data alloc failed\n");
		return SCF_DFA_ERROR;
	}

	if (d->current_node)
		scf_node_add_child(d->current_node, _do);
	else
		scf_node_add_child((scf_node_t*)parse->ast->current_block, _do);

	dd->_do          = _do;
	dd->parent_block = parse->ast->current_block;
	dd->parent_node  = d->current_node;
	d->current_node  = _do;

	scf_stack_push(s, dd);

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do__while"),  SCF_DFA_HOOK_END);

	return SCF_DFA_NEXT_WORD;
}

static int _do_action_while(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_lex_word_t* w = dfa->ops->pop_word(dfa);

	if (SCF_LEX_WORD_KEY_WHILE != w->type)
		return SCF_DFA_ERROR;

	return SCF_DFA_SWITCH_TO;
}

static int _do_action_lp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;

	assert(!d->expr);
	d->expr_local_flag = 1;

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _do_action_lp_stat(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	dfa_parse_data_t*  d  = data;
	scf_stack_t*       s  = d->module_datas[dfa_module_do.index];
	dfa_do_data_t*     dd = scf_stack_top(s);

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do_lp_stat"), SCF_DFA_HOOK_POST);

	dd->nb_lps++;

	return SCF_DFA_NEXT_WORD;
}

static int _do_action_rp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_lex_word_t*    w     = words->data[words->size - 1];
	scf_stack_t*       s     = d->module_datas[dfa_module_do.index];
	dfa_do_data_t*     dd    = scf_stack_top(s);

	if (!d->expr) {
		scf_loge("\n");
		return SCF_DFA_ERROR;
	}

	dd->nb_rps++;

	if (dd->nb_rps == dd->nb_lps) {

		assert(1 == dd->_do->nb_nodes);

		scf_node_add_child(dd->_do, d->expr);
		d->expr = NULL;

		d->expr_local_flag = 0;

		return SCF_DFA_SWITCH_TO;
	}

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "do_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _do_action_semicolon(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_do.index];
	dfa_do_data_t*     dd    = scf_stack_pop(s);

	assert(parse->ast->current_block == dd->parent_block);

	d->current_node = dd->parent_node;

	scf_logi("\033[31m do: %d, dd: %p, s->size: %d\033[0m\n", dd->_do->w->line, dd, s->size);

	free(dd);
	dd = NULL;

	assert(s->size >= 0);

	return SCF_DFA_OK;
}

static int _dfa_init_module_do(scf_dfa_t* dfa)
{
	SCF_DFA_MODULE_NODE(dfa, do, semicolon, scf_dfa_is_semicolon, _do_action_semicolon);

	SCF_DFA_MODULE_NODE(dfa, do, lp,        scf_dfa_is_lp,        _do_action_lp);
	SCF_DFA_MODULE_NODE(dfa, do, rp,        scf_dfa_is_rp,        _do_action_rp);
	SCF_DFA_MODULE_NODE(dfa, do, lp_stat,   scf_dfa_is_lp,        _do_action_lp_stat);

	SCF_DFA_MODULE_NODE(dfa, do, _do,       scf_dfa_is_do,        _do_action_do);
	SCF_DFA_MODULE_NODE(dfa, do, _while,    scf_dfa_is_while,     _do_action_while);

	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_do.index];

	assert(!s);

	s = scf_stack_alloc();
	if (!s) {
		scf_logi("\n");
		return SCF_DFA_ERROR;
	}

	d->module_datas[dfa_module_do.index] = s;

	return SCF_DFA_OK;
}

static int _dfa_fini_module_do(scf_dfa_t* dfa)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_do.index];

	if (s) {
		scf_stack_free(s);
		s = NULL;
		d->module_datas[dfa_module_do.index] = NULL;
	}

	return SCF_DFA_OK;
}

static int _dfa_init_syntax_do(scf_dfa_t* dfa)
{
	SCF_DFA_GET_MODULE_NODE(dfa, do,   lp,         lp);
	SCF_DFA_GET_MODULE_NODE(dfa, do,   rp,         rp);
	SCF_DFA_GET_MODULE_NODE(dfa, do,   lp_stat,    lp_stat);
	SCF_DFA_GET_MODULE_NODE(dfa, do,  _do,        _do);
	SCF_DFA_GET_MODULE_NODE(dfa, do,  _while,     _while);
	SCF_DFA_GET_MODULE_NODE(dfa, do,   semicolon,  semicolon);

	SCF_DFA_GET_MODULE_NODE(dfa, expr,  entry,     expr);
	SCF_DFA_GET_MODULE_NODE(dfa, block, entry,     block);

	// do start
	scf_vector_add(dfa->syntaxes,   _do);

	scf_dfa_node_add_child(_do,      block);
	scf_dfa_node_add_child(block,   _while);

	scf_dfa_node_add_child(_while,   lp);
	scf_dfa_node_add_child(lp,       expr);
	scf_dfa_node_add_child(expr,     rp);
	scf_dfa_node_add_child(rp,       semicolon);

	scf_logi("\n");
	return 0;
}

scf_dfa_module_t dfa_module_do =
{
	.name        = "do",

	.init_module = _dfa_init_module_do,
	.init_syntax = _dfa_init_syntax_do,

	.fini_module = _dfa_fini_module_do,
};
