#ifndef SCF_DFA_UTIL_H
#define SCF_DFA_UTIL_H

#include"scf_lex_word.h"
#include"scf_dfa.h"

static inline int scf_dfa_is_lp(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_LP == w->type;
}

static inline int scf_dfa_is_rp(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_RP == w->type;
}

static inline int scf_dfa_is_ls(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_LS == w->type;
}

static inline int scf_dfa_is_rs(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_RS == w->type;
}

static inline int scf_dfa_is_lb(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_LB == w->type;
}

static inline int scf_dfa_is_rb(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_RB == w->type;
}

static inline int scf_dfa_is_range(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_RANGE == w->type;
}

static inline int scf_dfa_is_comma(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_COMMA == w->type;
}

static inline int scf_dfa_is_semicolon(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_SEMICOLON == w->type;
}

static inline int scf_dfa_is_end_struct(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_STRUCT == w->type;
}

static inline int scf_dfa_is_colon(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_COLON == w->type;
}

static inline int scf_dfa_is_star(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_STAR == w->type;
}

static inline int scf_dfa_is_assign(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_ASSIGN == w->type;
}

static inline int scf_dfa_is_array(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_ARRAY == w->type;
}

static inline int scf_dfa_is_of(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_OF == w->type;
}

static inline int scf_dfa_is_identity(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return scf_lex_is_identity(w);
}

static inline int scf_dfa_is_base_type(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return scf_lex_is_base_type(w);
}

static inline int scf_dfa_is_sizeof(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_SIZEOF == w->type;
}

static inline int scf_dfa_is_vargs(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_VAR_ARGS == w->type;
}

static inline int scf_dfa_is_const(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_CONST == w->type;
}

static inline int scf_dfa_is_static(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_STATIC == w->type;
}

static inline int scf_dfa_is_extern(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_EXTERN == w->type;
}

static inline int scf_dfa_is_inline(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_INLINE == w->type;
}

static inline int scf_dfa_is_va_start(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VA_START == w->type;
}

static inline int scf_dfa_is_va_arg(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VA_ARG == w->type;
}

static inline int scf_dfa_is_va_end(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VA_END == w->type;
}

static inline int scf_dfa_is_container(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_CONTAINER == w->type;
}

static inline int scf_dfa_is_const_string(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_CONST_STRING == w->type;
}

static inline int scf_dfa_is_if(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_IF == w->type;
}

static inline int scf_dfa_is_else(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_ELSE == w->type;
}

static inline int scf_dfa_is_then(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_THEN == w->type;
}

static inline int scf_dfa_is_elsif(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_ELSIF == w->type;
}

static inline int scf_dfa_is_end_if(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_IF == w->type;
}

static inline int scf_dfa_is_while(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_WHILE == w->type;
}

static inline int scf_dfa_is_do(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_DO == w->type;
}

static inline int scf_dfa_is_to(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_TO == w->type;
}

static inline int scf_dfa_is_by(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_BY == w->type;
}

static inline int scf_dfa_is_end_while(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_WHILE == w->type;
}

static inline int scf_dfa_is_for(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_FOR == w->type;
}

static inline int scf_dfa_is_end_for(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_FOR == w->type;
}

static inline int scf_dfa_is_case(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_CASE == w->type;
}

static inline int scf_dfa_is_end_case(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_CASE == w->type;
}

static inline int scf_dfa_is_repeat(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_REPEAT == w->type;
}

static inline int scf_dfa_is_until(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_UNTIL == w->type;
}

static inline int scf_dfa_is_end_repeat(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_REPEAT == w->type;
}

static inline int scf_dfa_is_function(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_FUNCTION == w->type;
}

static inline int scf_dfa_is_end_function(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_FUNCTION == w->type;
}

static inline int scf_dfa_is_var(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR == w->type;
}

static inline int scf_dfa_is_var_input(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_INPUT == w->type;
}

static inline int scf_dfa_is_var_output(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_OUTPUT == w->type;
}

static inline int scf_dfa_is_var_in_out(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_IN_OUT == w->type;
}

static inline int scf_dfa_is_var_temp(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_TEMP == w->type;
}

static inline int scf_dfa_is_var_const(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_CONSTANT == w->type;
}

static inline int scf_dfa_is_var_global(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_GLOBAL == w->type;
}

static inline int scf_dfa_is_var_external(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_VAR_EXTERNAL == w->type;
}

static inline int scf_dfa_is_end_var(scf_dfa_t* dfa, void* word)
{
	scf_lex_word_t* w = word;

	return SCF_LEX_WORD_KEY_END_VAR == w->type;
}

#endif

