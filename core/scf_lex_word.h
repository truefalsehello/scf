#ifndef SCF_LEX_WORD_H
#define SCF_LEX_WORD_H

#include"scf_string.h"
#include"scf_list.h"

enum scf_lex_words {
	SCF_LEX_WORD_PLUS = 0,		// +
	SCF_LEX_WORD_MINUS,			// -
	SCF_LEX_WORD_STAR,			// *
	SCF_LEX_WORD_DIV,			// / div
	SCF_LEX_WORD_MOD,           // %

	SCF_LEX_WORD_INC,			// ++
	SCF_LEX_WORD_DEC,			// --

	SCF_LEX_WORD_SHL,           // <<
	SCF_LEX_WORD_SHR,           // >>

	SCF_LEX_WORD_BIT_AND,		// &
	SCF_LEX_WORD_BIT_OR,		// |
	SCF_LEX_WORD_BIT_NOT,		// ~

	SCF_LEX_WORD_LOGIC_AND,		// &&
	SCF_LEX_WORD_LOGIC_OR,		// ||
	SCF_LEX_WORD_LOGIC_NOT,		// !
	SCF_LEX_WORD_LOGIC_XOR,     // xor

	SCF_LEX_WORD_ASSIGN,		 //  = assign
	SCF_LEX_WORD_ADD_ASSIGN,     // +=
	SCF_LEX_WORD_SUB_ASSIGN,     // -=
	SCF_LEX_WORD_MUL_ASSIGN,     // *=
	SCF_LEX_WORD_DIV_ASSIGN,     // /=
	SCF_LEX_WORD_MOD_ASSIGN,     // %=
	SCF_LEX_WORD_SHL_ASSIGN,     // <<=
	SCF_LEX_WORD_SHR_ASSIGN,     // >>=
	SCF_LEX_WORD_BIT_AND_ASSIGN, // &=
	SCF_LEX_WORD_BIT_OR_ASSIGN,  // |=

	SCF_LEX_WORD_LT,			// < less than
	SCF_LEX_WORD_GT,			// > greater than

	SCF_LEX_WORD_EQ,			// == equal
	SCF_LEX_WORD_NE,			// != not equal
	SCF_LEX_WORD_LE,			// <= less equal 
	SCF_LEX_WORD_GE,			// >= greater equal

	SCF_LEX_WORD_LS,			// [ left square brackets
	SCF_LEX_WORD_RS,			// ] right square brackets

	SCF_LEX_WORD_LP,			// ( left parentheses
	SCF_LEX_WORD_RP,			// ) right parentheses

	SCF_LEX_WORD_LA,            // << left angle brackets
	SCF_LEX_WORD_RA,            // >> right angle brackets

	SCF_LEX_WORD_ARROW,         // ->  arrow
	SCF_LEX_WORD_DOT,           // .   dot
	SCF_LEX_WORD_RANGE,         // ..  range
	SCF_LEX_WORD_VAR_ARGS,      // ... variable args

	SCF_LEX_WORD_LB,			// { left brace
	SCF_LEX_WORD_RB,			// } right brace

	SCF_LEX_WORD_COMMA,			// , comma
	SCF_LEX_WORD_SEMICOLON,		// ;
	SCF_LEX_WORD_COLON,			// : colon
	SCF_LEX_WORD_SPACE,			// ' ' space

	// eof
	SCF_LEX_WORD_EOF,			// EOF

	// key words
	SCF_LEX_WORD_KEY_IF,		// if
	SCF_LEX_WORD_KEY_ELSE,		// else
	SCF_LEX_WORD_KEY_THEN,      // then
	SCF_LEX_WORD_KEY_ELSIF,     // else if
	SCF_LEX_WORD_KEY_END_IF,    // end if

	SCF_LEX_WORD_KEY_FOR,       // for
	SCF_LEX_WORD_KEY_TO,        // to
	SCF_LEX_WORD_KEY_BY,        // by
	SCF_LEX_WORD_KEY_DO,        // do
	SCF_LEX_WORD_KEY_END_FOR,   // end for

	SCF_LEX_WORD_KEY_WHILE,     // while
	SCF_LEX_WORD_KEY_END_WHILE, // end while

	SCF_LEX_WORD_KEY_REPEAT,    // repeat
	SCF_LEX_WORD_KEY_UNTIL,     // until
	SCF_LEX_WORD_KEY_END_REPEAT,//

	SCF_LEX_WORD_KEY_BREAK,     // break
	SCF_LEX_WORD_KEY_CONTINUE,  // continue
	SCF_LEX_WORD_KEY_EXIT,      // exit

	SCF_LEX_WORD_KEY_SWITCH,	// switch
	SCF_LEX_WORD_KEY_CASE,		// case
	SCF_LEX_WORD_KEY_DEFAULT,	// default
	SCF_LEX_WORD_KEY_END_CASE,  // end case

	SCF_LEX_WORD_KEY_RETURN,	// return

	SCF_LEX_WORD_KEY_GOTO,		// goto
	SCF_LEX_WORD_KEY_ERROR,     // error

	SCF_LEX_WORD_KEY_SIZEOF,    // sizeof

	SCF_LEX_WORD_KEY_CREATE,    // create class object

	SCF_LEX_WORD_KEY_CONTAINER, // container_of
	SCF_LEX_WORD_KEY_OF,        // of
	SCF_LEX_WORD_KEY_AT,        // at
	SCF_LEX_WORD_KEY_EN,        // enable
	SCF_LEX_WORD_KEY_ENO,       // enable output

	SCF_LEX_WORD_KEY_OPERATOR,  // operator

	SCF_LEX_WORD_KEY_UNDERLINE, // _ underline

	SCF_LEX_WORD_KEY_INCLUDE,   // include

	// data types
	SCF_LEX_WORD_KEY_CHAR,		// char
	SCF_LEX_WORD_KEY_BIT,       // bit

	SCF_LEX_WORD_KEY_TIME,          // time
	SCF_LEX_WORD_KEY_TIME_OF_DATE,  // time of date
	SCF_LEX_WORD_KEY_DATE,          // date
	SCF_LEX_WORD_KEY_DATE_AND_TIME, // date and time
	SCF_LEX_WORD_KEY_STRING,        // string

	SCF_LEX_WORD_KEY_INT,		// int
	SCF_LEX_WORD_KEY_FLOAT,     // float
	SCF_LEX_WORD_KEY_DOUBLE,	// double

	SCF_LEX_WORD_KEY_INT8,      // int8_t
	SCF_LEX_WORD_KEY_INT16,     // int16_t
	SCF_LEX_WORD_KEY_INT32,     // int32_t
	SCF_LEX_WORD_KEY_INT64,     // int64_t

	SCF_LEX_WORD_KEY_UINT8,     // uint8_t
	SCF_LEX_WORD_KEY_UINT16,    // uint16_t
	SCF_LEX_WORD_KEY_UINT32,    // uint32_t
	SCF_LEX_WORD_KEY_UINT64,    // uint64_t

	SCF_LEX_WORD_KEY_INTPTR,    // intptr_t
	SCF_LEX_WORD_KEY_UINTPTR,   // uintptr_t

	SCF_LEX_WORD_KEY_VOID,      // void

	SCF_LEX_WORD_KEY_VA_START,  // va_start
	SCF_LEX_WORD_KEY_VA_ARG,    // va_arg
	SCF_LEX_WORD_KEY_VA_END,    // va_end

	// class
	SCF_LEX_WORD_KEY_CLASS,     // class

	SCF_LEX_WORD_KEY_CONST,     // const
	SCF_LEX_WORD_KEY_STATIC,    // static
	SCF_LEX_WORD_KEY_EXTERN,    // extern
	SCF_LEX_WORD_KEY_INLINE,    // inline

	// for co-routine
	SCF_LEX_WORD_KEY_ASYNC,     // async
	SCF_LEX_WORD_KEY_AWAIT,     // await

	SCF_LEX_WORD_KEY_UNION,     // union
	SCF_LEX_WORD_KEY_STRUCT,    // struct
	SCF_LEX_WORD_KEY_END_STRUCT,// end struct
	SCF_LEX_WORD_KEY_ARRAY,     // array

	SCF_LEX_WORD_KEY_TASK,      // task

	SCF_LEX_WORD_KEY_TON,       // ton
	SCF_LEX_WORD_KEY_F_TRIG,    // f trig

	SCF_LEX_WORD_KEY_CONFIG,       // config
	SCF_LEX_WORD_KEY_END_CONFIG,   // end config

	SCF_LEX_WORD_KEY_RESOURCE,     // config
	SCF_LEX_WORD_KEY_END_RESOURCE, // end config

	SCF_LEX_WORD_KEY_PROGRAM,     // program
	SCF_LEX_WORD_KEY_END_PROGRAM,

	SCF_LEX_WORD_KEY_FUNCTION,    // function
	SCF_LEX_WORD_KEY_END_FUNCTION,

	SCF_LEX_WORD_KEY_FUNCTION_BLOCK,    // function block
	SCF_LEX_WORD_KEY_END_FUNCTION_BLOCK,

	SCF_LEX_WORD_KEY_VAR,          // var
	SCF_LEX_WORD_KEY_VAR_INPUT,    // var input
	SCF_LEX_WORD_KEY_VAR_OUTPUT,   // var output
	SCF_LEX_WORD_KEY_VAR_IN_OUT,   // var in out
	SCF_LEX_WORD_KEY_VAR_GLOBAL,   // var global
	SCF_LEX_WORD_KEY_VAR_EXTERNAL, // var external
	SCF_LEX_WORD_KEY_VAR_TEMP,     // var temp
	SCF_LEX_WORD_KEY_VAR_CONSTANT, // var const
	SCF_LEX_WORD_KEY_END_VAR,

	// const literal value
	SCF_LEX_WORD_CONST_CHAR,
	SCF_LEX_WORD_CONST_STRING,

	SCF_LEX_WORD_CONST_INT,
	SCF_LEX_WORD_CONST_FLOAT,
	SCF_LEX_WORD_CONST_DOUBLE,
	SCF_LEX_WORD_CONST_COMPLEX,

	SCF_LEX_WORD_CONST_U32,
	SCF_LEX_WORD_CONST_I64,
	SCF_LEX_WORD_CONST_U64,

	// identity
	SCF_LEX_WORD_ID,			// identity, start of _, a-z, A-Z, may include 0-9
};

typedef struct {
	float real;
	float imag;
} scf_complex_t;

typedef struct {
	scf_list_t		list;	// manage list, all words in this list, FIFO

	int				type;
	union {
		int32_t         i;    // value for <= int32_t
		uint32_t        u32;  // value for <= uint32_t
		int64_t         i64;  // value for int64_t
		uint64_t        u64;  // value for uint64_t
		float           f;    // value for float
		double			d;    // value for double
		scf_complex_t   z;    // value for complex
		scf_string_t*	s;    // value for string
	} data;

	scf_string_t*	text;	// original text

	scf_string_t*	file;	// original code file name
	int				line;	// line in the code file above	
	int				pos;	// position in the line above

} scf_lex_word_t;

static inline int scf_lex_is_identity(scf_lex_word_t* w)
{
	return w->type >= SCF_LEX_WORD_ID;
}

static inline int scf_lex_is_operator(scf_lex_word_t* w)
{
	return (w->type >= SCF_LEX_WORD_PLUS && w->type <= SCF_LEX_WORD_DOT)
		|| SCF_LEX_WORD_KEY_SIZEOF == w->type
		|| SCF_LEX_WORD_KEY_VA_ARG == w->type
		|| SCF_LEX_WORD_KEY_CREATE == w->type
		|| SCF_LEX_WORD_KEY_CONTAINER == w->type;
}

static inline int scf_lex_is_const(scf_lex_word_t* w)
{
	return w->type >= SCF_LEX_WORD_CONST_CHAR && w->type <= SCF_LEX_WORD_CONST_U64;
}

static inline int scf_lex_is_base_type(scf_lex_word_t* w)
{
	return SCF_LEX_WORD_KEY_CHAR <= w->type && SCF_LEX_WORD_KEY_VOID >= w->type;
}

scf_lex_word_t*		scf_lex_word_alloc(scf_string_t* file, int line, int pos, int type);
scf_lex_word_t*		scf_lex_word_clone(scf_lex_word_t* w);
void				scf_lex_word_free(scf_lex_word_t* w);

#endif

