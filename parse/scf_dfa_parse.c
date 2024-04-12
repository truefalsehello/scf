#include"scf_dfa.h"
#include"scf_parse.h"

extern scf_dfa_module_t  dfa_module_include;

extern scf_dfa_module_t  dfa_module_identity;

extern scf_dfa_module_t  dfa_module_expr;
extern scf_dfa_module_t  dfa_module_create;
extern scf_dfa_module_t  dfa_module_call;
extern scf_dfa_module_t  dfa_module_sizeof;
extern scf_dfa_module_t  dfa_module_container;
extern scf_dfa_module_t  dfa_module_init_data;
extern scf_dfa_module_t  dfa_module_va_arg;

extern scf_dfa_module_t  dfa_module_union;
extern scf_dfa_module_t  dfa_module_class;

extern scf_dfa_module_t  dfa_module_type;

extern scf_dfa_module_t  dfa_module_var;

extern scf_dfa_module_t  dfa_module_function;
extern scf_dfa_module_t  dfa_module_operator;

extern scf_dfa_module_t  dfa_module_if;
extern scf_dfa_module_t  dfa_module_while;
extern scf_dfa_module_t  dfa_module_do;
extern scf_dfa_module_t  dfa_module_for;
extern scf_dfa_module_t  dfa_module_switch;

#if 1
extern scf_dfa_module_t  dfa_module_break;
extern scf_dfa_module_t  dfa_module_continue;
extern scf_dfa_module_t  dfa_module_return;
extern scf_dfa_module_t  dfa_module_goto;
extern scf_dfa_module_t  dfa_module_label;
extern scf_dfa_module_t  dfa_module_async;
#endif
extern scf_dfa_module_t  dfa_module_block;

scf_dfa_module_t* dfa_modules[] =
{
	&dfa_module_include,

	&dfa_module_identity,

	&dfa_module_expr,
	&dfa_module_create,
	&dfa_module_call,
	&dfa_module_sizeof,
	&dfa_module_container,
	&dfa_module_init_data,
	&dfa_module_va_arg,

	&dfa_module_union,
	&dfa_module_class,

	&dfa_module_type,

	&dfa_module_var,

	&dfa_module_function,
	&dfa_module_operator,

	&dfa_module_if,
	&dfa_module_while,
	&dfa_module_do,
	&dfa_module_for,
	&dfa_module_switch,

#if 1
	&dfa_module_break,
	&dfa_module_continue,
	&dfa_module_goto,
	&dfa_module_return,
	&dfa_module_label,
	&dfa_module_async,
#endif
	&dfa_module_block,
};

int scf_parse_dfa_init(scf_parse_t* parse)
{
	if (scf_dfa_open(&parse->dfa, "parse", parse) < 0) {
		scf_loge("\n");
		return -1;
	}

	int nb_modules  = sizeof(dfa_modules) / sizeof(dfa_modules[0]);

	parse->dfa_data = calloc(1, sizeof(dfa_parse_data_t));
	if (!parse->dfa_data) {
		scf_loge("\n");
		return -1;
	}

	parse->dfa_data->module_datas = calloc(nb_modules, sizeof(void*));
	if (!parse->dfa_data->module_datas) {
		scf_loge("\n");
		return -1;
	}

	parse->dfa_data->current_identities = scf_stack_alloc();
	if (!parse->dfa_data->current_identities) {
		scf_loge("\n");
		return -1;
	}

	int i;
	for (i = 0; i < nb_modules; i++) {

		scf_dfa_module_t* m = dfa_modules[i];

		if (!m)
			continue;

		m->index = i;

		if (!m->init_module)
			continue;

		if (m->init_module(parse->dfa) < 0) {
			scf_loge("init module: %s\n", m->name);
			return -1;
		}
	}

	for (i = 0; i < nb_modules; i++) {

		scf_dfa_module_t* m = dfa_modules[i];

		if (!m || !m->init_syntax)
			continue;

		if (m->init_syntax(parse->dfa) < 0) {
			scf_loge("init syntax: %s\n", m->name);
			return -1;
		}
	}

	return 0;
}

static void* dfa_pop_word(scf_dfa_t* dfa)
{
	scf_parse_t* parse = dfa->priv;

	scf_lex_word_t* w = NULL;
	scf_lex_pop_word(parse->lex, &w);
	return w;
}

static int dfa_push_word(scf_dfa_t* dfa, void* word)
{
	scf_parse_t* parse = dfa->priv;

	scf_lex_word_t* w = word;
	scf_lex_push_word(parse->lex, w);
	return 0;
}

static void dfa_free_word(void* word)
{
	scf_lex_word_t* w = word;
	scf_lex_word_free(w);
}
#if 0
static int dfa_same_type(void* word, scf_dfa_word_t* dfa_word)
{
	scf_lex_word_t* w = word;
	return w->type == dfa_word->type;
}
#endif
scf_dfa_ops_t dfa_ops_parse = 
{
	.name      = "parse",

	.pop_word  = dfa_pop_word,
	.push_word = dfa_push_word,
	.free_word = dfa_free_word,
//	.same_type = dfa_same_type,
};
