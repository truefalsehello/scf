#include"scf_lex.h"

scf_lex_char_t* _lex_pop_char(scf_lex_t* lex)
{
	assert(lex);
	assert(lex->fp);

	if (!scf_list_empty(&lex->char_list_head)) {
		scf_list_t* l = scf_list_head(&lex->char_list_head);
		scf_lex_char_t* c = scf_list_data(l, scf_lex_char_t, list);
		scf_list_del(&c->list);
		return c;
	}

	scf_lex_char_t* c = malloc(sizeof(scf_lex_char_t));
	assert(c);

	c->c = fgetc(lex->fp);
	return c;
}

void _lex_push_char(scf_lex_t* lex, scf_lex_char_t* c)
{
	assert(lex);
	assert(c);

	scf_list_add_front(&lex->char_list_head, &c->list);
}

int _lex_op1_ll1(scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0, int type0)
{
	scf_string_t*	s = scf_string_cstr_len((char*)&c0->c, 1);
	scf_lex_word_t*	w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type0);

	lex->pos++;
	w->text = s;
	s = NULL;

	free(c0);
	c0 = NULL;

	*pword = w;
	return 0;
}

int _lex_op2_ll1(scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0,
		int type0, char* chs, int* types, int n)
{
	scf_string_t*   s  = scf_string_cstr_len((char*)&c0->c, 1);
	scf_lex_word_t* w  = NULL;
	scf_lex_char_t* c1 = _lex_pop_char(lex);

	int i;
	for (i = 0; i < n; i++) {
		if (chs[i] == c1->c)
			break;
	}

	if (i < n) {
		scf_string_cat_cstr_len(s, (char*)&c1->c, 1);
		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, types[i]);
		lex->pos += 2;

		free(c1);
		c1 = NULL;

	} else {
		_lex_push_char(lex, c1);
		c1 = NULL;
		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type0);
		lex->pos++;
	}

	w->text = s;
	s = NULL;

	free(c0);
	c0 = NULL;

	*pword = w;
	return 0;
}

int _lex_op3_ll1(scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0,
		char ch1_0, char ch1_1, char ch2, int type0, int type1, int type2, int type3)
{
	scf_lex_char_t* c1 = _lex_pop_char(lex);
	scf_lex_char_t* c2 = NULL;
	scf_lex_word_t* w  = NULL;
	scf_string_t*   s  = scf_string_cstr_len((char*)&c0->c, 1);

	if (ch1_0 == c1->c) {
		scf_string_cat_cstr_len(s, (char*)&c1->c, 1);

		c2 = _lex_pop_char(lex);

		if (ch2 == c2->c) {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);

			w         = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type0);
			w->text   = s;
			s         = NULL;
			lex->pos += 3;

			free(c2);
			c2 = NULL;
		} else {
			_lex_push_char(lex, c2);
			c2 = NULL;

			w         = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type1);
			w->text   = s;
			s         = NULL;
			lex->pos += 2;
		}

		free(c1);
		c1 = NULL;

	} else if (ch1_1 == c1->c) {
		scf_string_cat_cstr_len(s, (char*)&c1->c, 1);

		w         = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type2);
		w->text   = s;
		s         = NULL;
		lex->pos += 2;

		free(c1);
		c1 = NULL;
	} else {
		_lex_push_char(lex, c1);
		c1 = NULL;

		w       = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, type3);
		w->text = s;
		s       = NULL;
		lex->pos++;
	}

	free(c0);
	c0 = NULL;

	*pword = w;
	return 0;
}

int _lex_number_base_10(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s)
{
	scf_lex_char_t* c2;
	scf_lex_char_t* c3;
	scf_lex_word_t* w;

	int      dot   = 0;
	int      exp   = 0;
	int      neg   = 0;
	uint64_t value = 0;
	uint64_t num;

	while (1) {
		c2 = _lex_pop_char(lex);

		if (c2->c >= '0' && c2->c <= '9') {
			num    =        c2->c -  '0';
			value *= 10;
			value += num;

		} else if ('.' == c2->c) {

			c3 = _lex_pop_char(lex);

			_lex_push_char(lex, c3);

			if ('.' == c3->c) {
				c3 = NULL;

				_lex_push_char(lex, c2);
				c2 = NULL;
				break;
			}

			c3 = NULL;

			if (++dot > 1) {
				scf_loge("\n");
				return -EINVAL;
			}

		} else if ('e' == c2->c || 'E' == c2->c) {
			exp++;

			if (exp > 1) {
				scf_loge("\n");
				return -EINVAL;
			}

		} else if ('-' == c2->c) {
			neg++;

			if (0 == exp || neg > 1) {
				scf_loge("\n");
				return -EINVAL;
			}

		} else if ('_' == c2->c) {

		} else {
			_lex_push_char(lex, c2);
			c2 = NULL;
			break;
		}

		scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
		lex->pos++;

		free(c2);
		c2 = NULL;
	}

	if (exp > 0 || dot > 0) {
		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_DOUBLE);
		w->data.d = atof(s->data);
	} else {
		if (value & ~0xffffffffULL)
			w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U64);
		else
			w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U32);

		w->data.u64 = value;
	}

	w->text = s;
	s = NULL;

	*pword = w;
	return 0;
}

int _lex_number_base_16(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s)
{
	scf_lex_char_t* c2;
	scf_lex_word_t* w;

	uint64_t value = 0;
	uint64_t value2;

	while (1) {
		c2 = _lex_pop_char(lex);

		if (c2->c >= '0' && c2->c <= '9')
			value2 =        c2->c -  '0';

		else if ('a' <= c2->c && 'f' >= c2->c)
			value2    = c2->c  - 'a' + 10;

		else if ('A' <= c2->c && 'F' >= c2->c)
			value2    = c2->c  - 'A' + 10;

		else if ('_' == c2->c) {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos++;

			free(c2);
			c2 = NULL;

		} else {
			_lex_push_char(lex, c2);
			c2 = NULL;

			if (value & ~0xffffffffULL)
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U64);
			else
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U32);

			w->data.u64 = value;

			w->text = s;
			s = NULL;

			*pword = w;
			return 0;
		}

		value <<= 4;
		value  += value2;

		scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
		lex->pos++;

		free(c2);
		c2 = NULL;
	}
}

int _lex_number_base_8(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s)
{
	scf_lex_char_t* c2;
	scf_lex_word_t* w;

	uint64_t value = 0;

	while (1) {
		c2 = _lex_pop_char(lex);

		if (c2->c >= '0' && c2->c <= '7') {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos++;

			value  = (value << 3) + c2->c - '0';

			free(c2);
			c2 = NULL;

		} else if ('8' == c2->c || '9' == c2->c) {
			scf_loge("number must be 0-7 when base 8");

			free(c2);
			c2 = NULL;
			return -1;

		} else if ('_' == c2->c) {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos++;

			free(c2);
			c2 = NULL;

		} else {
			_lex_push_char(lex, c2);
			c2 = NULL;

			if (value & ~0xffffffffULL)
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U64);
			else
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U32);
			w->data.u64 = value;

			w->text = s;
			s = NULL;

			*pword = w;
			return 0;
		}
	}
}

int _lex_number_base_2(scf_lex_t* lex, scf_lex_word_t** pword, scf_string_t* s)
{
	scf_lex_char_t* c2;
	scf_lex_word_t* w;

	uint64_t value = 0;

	while (1) {
		c2 = _lex_pop_char(lex);

		if (c2->c >= '0' && c2->c <= '1') {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos++;

			value  = (value << 1) + c2->c - '0';

			free(c2);
			c2 = NULL;

		} else if (c2->c >= '2' && c2->c <= '9') {
			scf_loge("number must be 0-1 when base 2");

			free(c2);
			c2 = NULL;
			return -1;

		} else if ('_' == c2->c) {
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos++;

			free(c2);
			c2 = NULL;

		} else {
			_lex_push_char(lex, c2);
			c2 = NULL;

			if (value & ~0xffffffffULL)
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U64);
			else
				w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_U32);
			w->data.u64 = value;

			w->text = s;
			s = NULL;

			*pword = w;
			return 0;
		}
	}
}

int _lex_dot(scf_lex_t* lex, scf_lex_word_t** pword, scf_lex_char_t* c0)
{
	scf_lex_char_t* c1 = _lex_pop_char(lex);
	scf_lex_char_t* c2 = NULL;
	scf_lex_word_t* w  = NULL;
	scf_string_t*   s  = scf_string_cstr_len((char*)&c0->c, 1);

	lex->pos++;

	free(c0);
	c0 = NULL;

	if ('.' == c1->c) {

		c2 = _lex_pop_char(lex);

		if ('.' == c2->c) {
			scf_string_cat_cstr_len(s, (char*)&c1->c, 1);
			scf_string_cat_cstr_len(s, (char*)&c2->c, 1);
			lex->pos += 2;

			free(c1);
			free(c2);
			c1 = NULL;
			c2 = NULL;

			w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_VAR_ARGS);

			w->text = s;
			*pword  = w;
			return 0;
		}

		_lex_push_char(lex, c2);
		c2 = NULL;

		scf_string_cat_cstr_len(s, (char*)&c1->c, 1);
		lex->pos++;

		free(c1);
		c1 = NULL;

		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_RANGE);

		w->text = s;
		*pword  = w;
		return 0;
	}

	_lex_push_char(lex, c1);
	c1 = NULL;

	int numbers = 0;
	int dots    = 1;

	while (1) {
		c1 = _lex_pop_char(lex);

		if ('0' <= c1->c && '9' >= c1->c)
			numbers++;

		else if ('.' == c1->c)
			dots++;
		else {
			_lex_push_char(lex, c1);
			c1 = NULL;
			break;
		}

		scf_string_cat_cstr_len(s, (char*)&c1->c, 1);
		lex->pos++;

		free(c1);
		c1 = NULL;
	}

	if (numbers  > 0) {

		if (dots > 1) {
			scf_loge("\n");
			return -1;
		}

		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_CONST_DOUBLE);
		w->data.d = atof(s->data);

	} else if (1 == dots) // dot .
		w = scf_lex_word_alloc(lex->file, lex->nb_lines, lex->pos, SCF_LEX_WORD_DOT);
	else {
		scf_loge("\n");
		return -1;
	}

	w->text = s;
	s = NULL;

	*pword = w;
	return 0;
}
