#ifndef SCF_LEX_H
#define SCF_LEX_H

#include"scf_lex_word.h"

typedef struct {
	char*	text;
	int		type;
} scf_lex_key_word_t;

typedef struct {
	int		origin;
	int		escape;
} scf_lex_escape_char_t;

typedef struct {
	scf_list_t		list;	// manage list, all errors here

	scf_string_t*	message;	// error message for user

	scf_string_t*	file;	// original code file name
	int				line;	// line in the code file above	
	int				pos;	// position in the line above

} scf_lex_error_t;

typedef struct {
	scf_list_t		list;
	int				c;
} scf_lex_char_t;

typedef struct {
	scf_list_t		word_list_head; // word list head
	scf_list_t		error_list_head; // error list head

	scf_list_t		char_list_head; // temp char list head
	FILE*			fp;	// file pointer to the code

	int				nb_identities;

	scf_string_t*	file;	// original code file name
	int				nb_lines;
	int				pos;

} scf_lex_t;

scf_lex_error_t*	scf_lex_error_alloc(scf_string_t* file, int line, int pos);
void 				scf_lex_error_free(scf_lex_error_t* e);

scf_lex_char_t*    _lex_pop_char (scf_lex_t* lex);
void               _lex_push_char(scf_lex_t* lex, scf_lex_char_t* c);


int	scf_lex_open(scf_lex_t** plex, const char* path);
int scf_lex_close(scf_lex_t* lex);

int scf_lex_push_word(scf_lex_t* lex, scf_lex_word_t* word);
int scf_lex_pop_word(scf_lex_t* lex, scf_lex_word_t** pword);


int _lex_number_base_16(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s);
int _lex_number_base_10(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s);
int _lex_number_base_8 (scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s);
int _lex_number_base_2 (scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s);

int _lex_dot           (scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0);

int _lex_op1_ll1       (scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0, int type0);
int _lex_op2_ll1       (scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0, int type0, char* chs, int* types, int n);
int _lex_op3_ll1       (scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0, char ch1_0, char ch1_1, char ch2, int type0, int type1, int type2, int type3);
#endif

