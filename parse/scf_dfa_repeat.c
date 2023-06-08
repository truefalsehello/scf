#include"scf_dfa.h"
#include"scf_dfa_util.h"
#include"scf_parse.h"
#include"scf_stack.h"

extern scf_dfa_module_t dfa_module_repeat;

typedef struct {
	int              nb_lps;
	int              nb_rps;

	scf_block_t*     parent_block;
	scf_node_t*      parent_node;

	scf_node_t*      repeat;

	scf_dfa_hook_t*  hook_end;

} dfa_repeat_data_t;


static int _repeat_action_do(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = data;
	scf_lex_word_t*   w     = words->data[words->size - 1];
	scf_stack_t*      s     = d->module_datas[dfa_module_repeat.index];
	scf_block_t*      b     = NULL;

	scf_node_t*     repeat  = scf_node_alloc(w, SCF_OP_REPEAT, NULL);
	if (!repeat) {
		scf_loge("node alloc failed\n");
		return SCF_DFA_ERROR;
	}

	dfa_repeat_data_t* rd = calloc(1, sizeof(dfa_repeat_data_t));
	if (!rd) {
		scf_loge("module data alloc failed\n");
		return SCF_DFA_ERROR;
	}

	if (d->current_node)
		scf_node_add_child(d->current_node, repeat);
	else
		scf_node_add_child((scf_node_t*)parse->ast->current_block, repeat);

	rd->repeat       = repeat;
	rd->parent_block = parse->ast->current_block;
	rd->parent_node  = d->current_node;
	d->current_node  = repeat;

	scf_stack_push(s, rd);

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat__while"),  SCF_DFA_HOOK_END);

	return SCF_DFA_NEXT_WORD;
}

static int _repeat_action_while(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_lex_word_t* w = dfa->ops->pop_word(dfa);

	if (SCF_LEX_WORD_KEY_WHILE != w->type)
		return SCF_DFA_ERROR;

	return SCF_DFA_SWITCH_TO;
}

static int _repeat_action_lp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_lex_word_t*    w     = words->data[words->size - 1];
	scf_stack_t*       s     = d->module_datas[dfa_module_repeat.index];
	dfa_repeat_data_t* rd    = scf_stack_top(s);

	assert(!d->expr);
	d->expr_local_flag = 1;

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _repeat_action_lp_stat(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_repeat.index];
	dfa_repeat_data_t* rd    = scf_stack_top(s);

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat_lp_stat"), SCF_DFA_HOOK_POST);

	rd->nb_lps++;

	return SCF_DFA_NEXT_WORD;
}

static int _repeat_action_rp(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_lex_word_t*    w     = words->data[words->size - 1];
	scf_stack_t*       s     = d->module_datas[dfa_module_repeat.index];
	dfa_repeat_data_t* rd    = scf_stack_top(s);

	if (!d->expr) {
		scf_loge("\n");
		return SCF_DFA_ERROR;
	}

	rd->nb_rps++;

	if (rd->nb_rps == rd->nb_lps) {

		assert(1 == rd->repeat->nb_nodes);

		scf_node_add_child(rd->repeat, d->expr);
		d->expr = NULL;

		d->expr_local_flag = 0;

		return SCF_DFA_SWITCH_TO;
	}

	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat_rp"),      SCF_DFA_HOOK_POST);
	SCF_DFA_PUSH_HOOK(scf_dfa_find_node(dfa, "repeat_lp_stat"), SCF_DFA_HOOK_POST);

	return SCF_DFA_NEXT_WORD;
}

static int _repeat_action_semicolon(scf_dfa_t* dfa, scf_vector_t* words, void* data)
{
	scf_parse_t*       parse = dfa->priv;
	dfa_parse_data_t*  d     = data;
	scf_stack_t*       s     = d->module_datas[dfa_module_repeat.index];
	dfa_repeat_data_t* rd    = scf_stack_pop(s);

	assert(parse->ast->current_block == rd->parent_block);

	d->current_node = rd->parent_node;

	scf_logi("\033[31m repeat: %d, rd: %p, s->size: %d\033[0m\n", rd->repeat->w->line, rd, s->size);

	free(rd);
	rd = NULL;

	assert(s->size >= 0);

	return SCF_DFA_OK;
}

static int _dfa_init_module_repeat(scf_dfa_t* dfa)
{
	SCF_DFA_MODULE_NODE(dfa, repeat, semicolon, scf_dfa_is_semicolon, _repeat_action_semicolon);

	SCF_DFA_MODULE_NODE(dfa, repeat, lp,        scf_dfa_is_lp,        _repeat_action_lp);
	SCF_DFA_MODULE_NODE(dfa, repeat, rp,        scf_dfa_is_rp,        _repeat_action_rp);
	SCF_DFA_MODULE_NODE(dfa, repeat, lp_stat,   scf_dfa_is_lp,        _repeat_action_lp_stat);

	SCF_DFA_MODULE_NODE(dfa, repeat, _do,       scf_dfa_is_do,        _repeat_action_do);
	SCF_DFA_MODULE_NODE(dfa, repeat, _while,    scf_dfa_is_while,     _repeat_action_while);

	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_repeat.index];

	assert(!s);

	s = scf_stack_alloc();
	if (!s) {
		scf_logi("\n");
		return SCF_DFA_ERROR;
	}

	d->module_datas[dfa_module_repeat.index] = s;

	return SCF_DFA_OK;
}

static int _dfa_fini_module_repeat(scf_dfa_t* dfa)
{
	scf_parse_t*      parse = dfa->priv;
	dfa_parse_data_t* d     = parse->dfa_data;
	scf_stack_t*      s     = d->module_datas[dfa_module_repeat.index];

	if (s) {
		scf_stack_free(s);
		s = NULL;
		d->module_datas[dfa_module_repeat.index] = NULL;
	}

	return SCF_DFA_OK;
}

static int _dfa_init_syntax_repeat(scf_dfa_t* dfa)
{
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,   lp,         lp);
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,   rp,         rp);
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,   lp_stat,    lp_stat);
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,  _do,        _do);
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,  _while,     _while);
	SCF_DFA_GET_MODULE_NODE(dfa, repeat,   semicolon,  semicolon);

	SCF_DFA_GET_MODULE_NODE(dfa, expr,  entry,     expr);
	SCF_DFA_GET_MODULE_NODE(dfa, block, entry,     block);

	// repeat start
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

scf_dfa_module_t dfa_module_repeat =
{
	.name        = "repeat",

	.init_module = _dfa_init_module_repeat,
	.init_syntax = _dfa_init_syntax_repeat,

	.fini_module = _dfa_fini_module_repeat,
};

