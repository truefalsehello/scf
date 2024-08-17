#include"scf_operator.h"
#include"scf_core_types.h"

static scf_operator_t	base_operators[SCF_LAST_ITEM] = {
	[SCF_OP_EXPR] = {SCF_OP_EXPR,           "(",          0,  1,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_CALL] = {SCF_OP_CALL,           "(",          1, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_ARRAY_INDEX] = {SCF_OP_ARRAY_INDEX,    "[",          1,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_POINTER] = {SCF_OP_POINTER,        "->",         1,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_VA_START] = {SCF_OP_VA_START,       "va_start",   1,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_VA_ARG] = {SCF_OP_VA_ARG,         "va_arg",     1,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_VA_END] = {SCF_OP_VA_END,         "va_end",     1,  1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_CONTAINER] = {SCF_OP_CONTAINER,      "container",  1,  3,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_CREATE] = {SCF_OP_CREATE,         "create",     2, -1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_TYPE_CAST] = {SCF_OP_TYPE_CAST,      "(",          2,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_LOGIC_NOT] = {SCF_OP_LOGIC_NOT,      "!",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_BIT_NOT] = {SCF_OP_BIT_NOT,        "~",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_NEG] = {SCF_OP_NEG,            "-",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_POSITIVE] = {SCF_OP_POSITIVE,       "+",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_SIZEOF] = {SCF_OP_SIZEOF,         "sizeof",     2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},

	[SCF_OP_INC] = {SCF_OP_INC,            "++",         2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_DEC] = {SCF_OP_DEC,            "--",         2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},

	[SCF_OP_INC_POST] = {SCF_OP_INC_POST,       "++",         2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_DEC_POST] = {SCF_OP_DEC_POST,       "--",         2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},

	[SCF_OP_DEREFERENCE] = {SCF_OP_DEREFERENCE, 	"*",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_ADDRESS_OF] = {SCF_OP_ADDRESS_OF, 	"&",          2,  1,  SCF_OP_ASSOCIATIVITY_RIGHT},

	[SCF_OP_MUL] = {SCF_OP_MUL,            "*",          4,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_DIV] = {SCF_OP_DIV,            "/",          4,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_MOD] = {SCF_OP_MOD,            "%",          4,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_ADD] = {SCF_OP_ADD,            "+",          5,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_SUB] = {SCF_OP_SUB,            "-",          5,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_SHL] = {SCF_OP_SHL,            "<<",         6,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_SHR] = {SCF_OP_SHR,            ">>",         6,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_BIT_AND] = {SCF_OP_BIT_AND,        "&",          7,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_BIT_OR] = {SCF_OP_BIT_OR,         "|",          7,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_EQ] = {SCF_OP_EQ,             "==",         8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_NE] = {SCF_OP_NE,             "!=",         8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_GT] = {SCF_OP_GT,             ">",          8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_LT] = {SCF_OP_LT,             "<",          8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_GE] = {SCF_OP_GE,             ">=",         8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_LE] = {SCF_OP_LE,             "<=",         8,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_LOGIC_AND] = {SCF_OP_LOGIC_AND,      "&&",         9,  2,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_LOGIC_OR] = {SCF_OP_LOGIC_OR,       "||",         9,  2,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_ASSIGN] = {SCF_OP_ASSIGN,         "=",         10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_ADD_ASSIGN] = {SCF_OP_ADD_ASSIGN,     "+=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_SUB_ASSIGN] = {SCF_OP_SUB_ASSIGN,     "-=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_MUL_ASSIGN] = {SCF_OP_MUL_ASSIGN,     "*=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_DIV_ASSIGN] = {SCF_OP_DIV_ASSIGN,     "/=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_MOD_ASSIGN] = {SCF_OP_MOD_ASSIGN,     "%=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_SHL_ASSIGN] = {SCF_OP_SHL_ASSIGN,     "<<=",       10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_SHR_ASSIGN] = {SCF_OP_SHR_ASSIGN,     ">>=",       10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_AND_ASSIGN] = {SCF_OP_AND_ASSIGN,     "&=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},
	[SCF_OP_OR_ASSIGN] = {SCF_OP_OR_ASSIGN,      "|=",        10,  2,  SCF_OP_ASSOCIATIVITY_RIGHT},

	[SCF_OP_BLOCK] = {SCF_OP_BLOCK,          "{}",        15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_RETURN] = {SCF_OP_RETURN,         "return",    15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_BREAK] = {SCF_OP_BREAK,          "break",     15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_CONTINUE] = {SCF_OP_CONTINUE,       "continue",  15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_GOTO] = {SCF_OP_GOTO,           "goto",      15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_LABEL] = {SCF_LABEL,             "label",     15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_IF] = {SCF_OP_IF,             "if",        15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_WHILE] = {SCF_OP_WHILE,          "while",     15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_DO] = {SCF_OP_DO,             "do",        15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_FOR] = {SCF_OP_FOR,            "for",       15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},

	[SCF_OP_SWITCH] = {SCF_OP_SWITCH,         "switch",    15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_CASE] = {SCF_OP_CASE,           "case",      15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
	[SCF_OP_DEFAULT] = {SCF_OP_DEFAULT,        "default",   15, -1,  SCF_OP_ASSOCIATIVITY_LEFT},
};

inline scf_operator_t* scf_find_base_operator(const char* name, const int nb_operands)
{
	int i;
	for (i = 0; i < sizeof(base_operators) / sizeof(base_operators[0]); i++)
	{

		scf_operator_t *op = &base_operators[i];

		if (nb_operands == op->nb_operands && !strcmp(name, op->name))
			return op;
	}

	return NULL;
}

inline scf_operator_t* scf_find_base_operator_by_type(const int type)
{
	return type < SCF_LAST_ITEM && NULL != base_operators[type].name ?
	&(base_operators[type]) : NULL;
}

