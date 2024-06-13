#include"scf_ast.h"
#include"scf_operator_handler.h"
#include"scf_3ac.h"

typedef struct {
	scf_vector_t*	_breaks;
	scf_vector_t*	_continues;
	scf_vector_t*	_gotos;
	scf_vector_t*	_labels;
	scf_vector_t*	_ends;
} scf_branch_ops_t;

typedef struct {
	scf_branch_ops_t*	branch_ops;
	scf_list_t*			_3ac_list_head;

} scf_handler_data_t;

scf_branch_ops_t* scf_branch_ops_alloc()
{
	scf_branch_ops_t* branch_ops = calloc(1, sizeof(scf_branch_ops_t));

	if (branch_ops) {
		branch_ops->_breaks		= scf_vector_alloc();
		branch_ops->_continues	= scf_vector_alloc();
		branch_ops->_gotos		= scf_vector_alloc();
		branch_ops->_labels     = scf_vector_alloc();
		branch_ops->_ends       = scf_vector_alloc();
	}

	return branch_ops;
}

void scf_branch_ops_free(scf_branch_ops_t* branch_ops)
{
	if (branch_ops) {
		scf_vector_free(branch_ops->_breaks);
		scf_vector_free(branch_ops->_continues);
		scf_vector_free(branch_ops->_gotos);
		scf_vector_free(branch_ops->_labels);
		scf_vector_free(branch_ops->_ends);

		free(branch_ops);
		branch_ops = NULL;
	}
}

static int _scf_expr_calculate_internal(scf_ast_t* ast, scf_node_t* node, void* data)
{
	if (!node)
		return 0;

	scf_handler_data_t*     d = data;
	scf_operator_handler_t* h;

	int i;

	if (0 == node->nb_nodes) {
		if (scf_type_is_var(node->type) && node->var->w)
			scf_logd("node->var->w->text->data: %s\n", node->var->w->text->data);

		assert(scf_type_is_var(node->type) || SCF_LABEL == node->type || node->split_flag);
		return 0;
	}

	assert(scf_type_is_operator(node->type));
	assert(node->nb_nodes > 0);

	if (!node->op) {
		scf_loge("node->type: %d\n", node->type);
		return -1;
	}

	if (node->_3ac_done)
		return 0;
	node->_3ac_done = 1;

	if (SCF_OP_ASSOCIATIVITY_LEFT == node->op->associativity) {

		if (SCF_OP_LOGIC_AND != node->op->type && SCF_OP_LOGIC_OR != node->op->type) {

			for (i = 0; i < node->nb_nodes; i++) {
				if (_scf_expr_calculate_internal(ast, node->nodes[i], d) < 0) {
					scf_loge("\n");
					return -1;
				}
			}
		}

		h = scf_find_3ac_operator_handler(node->op->type);
		if (!h) {
			scf_loge("\n");
			return -1;
		}

		return h->func(ast, node->nodes, node->nb_nodes, d);

	} else {
		if (!scf_type_is_assign(node->op->type) && SCF_OP_ADDRESS_OF != node->op->type) {

			for (i = node->nb_nodes - 1; i >= 0; i--) {
				if (_scf_expr_calculate_internal(ast, node->nodes[i], d) < 0) {
					scf_loge("\n");
					return -1;
				}
			}
		}

		h = scf_find_3ac_operator_handler(node->op->type);
		if (!h) {
			scf_loge("op->type: %d, name: '%s'\n", node->op->type, node->op->name);
			return -1;
		}

		return h->func(ast, node->nodes, node->nb_nodes, d);
	}

	scf_loge("\n");
	return -1;
}

static int _scf_expr_calculate(scf_ast_t* ast, scf_expr_t* e, void* data)
{
	scf_handler_data_t* d = data;

	assert(e);
	assert(e->nodes);

	scf_node_t* root = e->nodes[0];

	if (scf_type_is_var(root->type))
		return 0;

	if (_scf_expr_calculate_internal(ast, root, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	return 0;
}

static int _scf_3ac_code_NN(scf_list_t* h, int op_type, scf_node_t** dsts, int nb_dsts, scf_node_t** srcs, int nb_srcs)
{
	scf_3ac_code_t* c = scf_3ac_code_NN(op_type, dsts, nb_dsts, srcs, nb_srcs);
	if (!c) {
		scf_loge("\n");
		return -1;
	}

	scf_list_add_tail(h, &c->list);
	return 0;
}

static int _scf_3ac_code_N(scf_list_t* h, int op_type, scf_node_t* d, scf_node_t** nodes, int nb_nodes)
{
	return _scf_3ac_code_NN(h, op_type, &d, 1, nodes, nb_nodes);
}

static int _scf_3ac_code_3(scf_list_t* h, int op_type, scf_node_t* d, scf_node_t* n0, scf_node_t* n1)
{
	scf_node_t* srcs[2] = {n0, n1};

	return _scf_3ac_code_NN(h, op_type, &d, 1, srcs, 2);
}

static int _scf_3ac_code_2(scf_list_t* h, int op_type, scf_node_t* d, scf_node_t* n0)
{
	return _scf_3ac_code_NN(h, op_type, &d, 1, &n0, 1);
}

static int _scf_3ac_code_1(scf_list_t* h, int op_type, scf_node_t* n0)
{
	return _scf_3ac_code_NN(h, op_type, NULL, 0, &n0, 1);
}

static int _scf_3ac_code_dst(scf_list_t* h, int op_type, scf_node_t* d)
{
	return _scf_3ac_code_NN(h, op_type, &d, 1, NULL, 0);
}

static int _scf_3ac_code_srcN(scf_list_t* h, int op_type, scf_node_t** nodes, int nb_nodes)
{
	return _scf_3ac_code_NN(h, op_type, NULL, 0, nodes, nb_nodes);
}

static int _scf_op_va_start(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_variable_t*     vptr;
	scf_type_t*         tptr;
	scf_node_t*         nptr;
	scf_node_t*         parent = nodes[0]->parent;
	scf_node_t*         srcs[3];

	tptr = scf_block_find_type_type(ast->current_block, SCF_VAR_UINTPTR);
	vptr = SCF_VAR_ALLOC_BY_TYPE(NULL, tptr, 0, 0, NULL);
	if (!vptr)
		return -ENOMEM;
	vptr->data.u64 = 0;
	vptr->tmp_flag = 1;

	nptr = scf_node_alloc(NULL, vptr->type, vptr);
	if (!nptr)
		return -ENOMEM;

	if (scf_node_add_child(parent, nptr) < 0)
		return -ENOMEM;

	srcs[0] = parent->nodes[0];
	srcs[1] = parent->nodes[1];
	srcs[2] = nptr;

	return _scf_3ac_code_srcN(d->_3ac_list_head, SCF_OP_VA_START, srcs, 3);
}

static int _scf_op_va_end(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(1 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_variable_t*     vptr;
	scf_type_t*         tptr;
	scf_node_t*         nptr;
	scf_node_t*         parent = nodes[0]->parent;
	scf_node_t*         srcs[2];

	tptr = scf_block_find_type_type(ast->current_block, SCF_VAR_UINTPTR);
	vptr = SCF_VAR_ALLOC_BY_TYPE(NULL, tptr, 0, 0, NULL);
	if (!vptr)
		return -ENOMEM;
	vptr->data.u64   = 0;
	vptr->const_flag = 1;

	nptr = scf_node_alloc(NULL, vptr->type, vptr);
	if (!nptr)
		return -ENOMEM;

	if (scf_node_add_child(parent, nptr) < 0)
		return -ENOMEM;

	srcs[0] = parent->nodes[0];
	srcs[1] = nptr;

	return _scf_3ac_code_srcN(d->_3ac_list_head, SCF_OP_VA_END, srcs, 2);
}

static int _scf_op_va_arg(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_variable_t*     v;
	scf_variable_t*     vptr;
	scf_type_t*         tptr;
	scf_node_t*         nptr;
	scf_node_t*         parent = nodes[0]->parent;
	scf_node_t*         srcs[3];

	v = _scf_operand_get(parent);
	v->tmp_flag = 1;

	v = _scf_operand_get(nodes[1]);
	v->const_flag = 1;

	tptr = scf_block_find_type_type(ast->current_block, SCF_VAR_UINTPTR);
	vptr = SCF_VAR_ALLOC_BY_TYPE(NULL, tptr, 0, 0, NULL);
	if (!vptr)
		return -ENOMEM;

	vptr->data.u64   = 0;
	vptr->tmp_flag   = 1;
	vptr->extra_flag = 1;

	nptr = scf_node_alloc(NULL, vptr->type, vptr);
	if (!nptr)
		return -ENOMEM;

	if (scf_node_add_child(parent, nptr) < 0)
		return -ENOMEM;

	srcs[0] = parent->nodes[0];
	srcs[1] = parent->nodes[1];
	srcs[2] = nptr;

	return _scf_3ac_code_NN(d->_3ac_list_head, SCF_OP_VA_ARG, &parent, 1, srcs, 3);
}

static int _scf_op_pointer(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_variable_t*     v;

	scf_node_t* parent = nodes[0]->parent;

	v = _scf_operand_get(parent);
	v->tmp_flag = 1;

	return _scf_3ac_code_3(d->_3ac_list_head, SCF_OP_POINTER, parent, nodes[0], nodes[1]);
}

static int _scf_op_array_scale(scf_ast_t* ast, scf_node_t* parent, scf_node_t** pscale)
{
	scf_variable_t* v_member;
	scf_variable_t* v_scale;
	scf_type_t*     t_scale;
	scf_node_t*     n_scale;

	v_member = _scf_operand_get(parent);

	int size = scf_variable_size(v_member);
	assert(size > 0);

	t_scale = scf_block_find_type_type(ast->current_block, SCF_VAR_INTPTR);
	v_scale = SCF_VAR_ALLOC_BY_TYPE(NULL, t_scale, 0, 0, NULL);
	if (!v_scale)
		return -ENOMEM;
	v_scale->data.i = size;

	n_scale = scf_node_alloc(NULL, v_scale->type, v_scale);
	if (!n_scale)
		return -ENOMEM;

	*pscale = n_scale;
	return 0;
}

static int _scf_op_array_index(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d   = data;

	scf_node_t*     parent  = nodes[0]->parent;
	scf_node_t*     n_index = NULL;
	scf_node_t*     n_scale = NULL;
	scf_node_t*     srcs[3];
	scf_variable_t* v;

	int ret = _scf_expr_calculate_internal(ast, nodes[1], d);
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	ret = _scf_op_array_scale(ast, parent, &n_scale);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	n_index = nodes[1];
	while (SCF_OP_EXPR == n_index->type)
		n_index = n_index->nodes[0];

	srcs[0] = nodes[0];
	srcs[1] = n_index;
	srcs[2] = n_scale;

	v = _scf_operand_get(parent);
	v->tmp_flag = 1;

	return _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_ARRAY_INDEX, parent, srcs, 3);
}

static int _scf_op_block(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	if (0 == nb_nodes)
		return 0;

	scf_operator_handler_t* h;
	scf_operator_t*         op;
	scf_handler_data_t*     d  = data;
	scf_block_t*            b  = (scf_block_t*)(nodes[0]->parent);
	scf_block_t*            up = ast->current_block;
	scf_node_t*             node;

	if (b->node._3ac_done)
		return 0;

	ast->current_block = b;

	int i;
	int j;

	for (i = 0; i < nb_nodes; i++) {

		node = nodes[i];
		op   = node->op;

		if (!op) {
			op = scf_find_base_operator_by_type(node->type);
			if (!op) {
				scf_loge("\n");
				return -1;
			}
		}

		h = scf_find_3ac_operator_handler(op->type);
		if (!h) {
			scf_loge("\n");
			return -1;
		}

		if (h->func(ast, node->nodes, node->nb_nodes, d) < 0) {
			scf_loge("\n");
			ast->current_block = up;
			return -1;
		}

		// for goto
		if (SCF_LABEL == node->type) {

			scf_label_t*       l    = node->label;
			scf_list_t*        tail = scf_list_tail(d->_3ac_list_head);
			scf_3ac_code_t*	   end  = scf_list_data(tail, scf_3ac_code_t, list);
			scf_3ac_code_t*    c;
			scf_3ac_operand_t* dst;

			if (scf_vector_add(d->branch_ops->_labels, end) < 0) {
				scf_loge("\n");
				return -1;
			}
			end->label = l;

			int j;
			for (j = 0; j < d->branch_ops->_gotos->size; j++) {
				c  =        d->branch_ops->_gotos->data[j];

				if (!c)
					continue;

				assert(l->w);
				assert(c->label->w);

				if (!strcmp(l->w->text->data, c->label->w->text->data)) {
					dst       = c->dsts->data[0];
					dst->code = end;
				}
			}
		}
	}

	ast->current_block = up;
	return 0;
}

static int _scf_op_return(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	scf_handler_data_t* d = data;
	scf_function_t*     f = (scf_function_t*) ast->current_block;

	while (f && SCF_FUNCTION  != f->node.type)
		f = (scf_function_t*) f->node.parent;

	if (!f) {
		scf_loge("\n");
		return -1;
	}

	if (nb_nodes > f->rets->size) {
		scf_loge("\n");
		return -1;
	}

	scf_node_t* parent;
	scf_node_t* srcs[4];
	scf_expr_t* e;

	int i;
	for (i = 0; i < nb_nodes && i < 4; i++) {
		e  = nodes[i];

		if (_scf_expr_calculate_internal(ast, e->nodes[0], d) < 0) {
			scf_loge("\n");
			return -1;
		}

		srcs[i] = e->nodes[0];
	}

	if (i > 0) {
		if (_scf_3ac_code_srcN(d->_3ac_list_head, SCF_OP_RETURN, srcs, i) < 0)
			return -1;
	}

	scf_3ac_code_t* end = scf_3ac_jmp_code(SCF_OP_GOTO, NULL, NULL);

	scf_list_add_tail(d->_3ac_list_head, &end->list);

	return scf_vector_add(d->branch_ops->_ends, end);
}

static int _scf_op_break(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	scf_handler_data_t* d  = data;
	scf_node_t*         up = (scf_node_t*)ast->current_block;

	while (up
			&& SCF_OP_WHILE  != up->type
			&& SCF_OP_DO     != up->type
			&& SCF_OP_FOR    != up->type
			&& SCF_OP_SWITCH != up->type)
		up = up->parent;

	if (!up) {
		scf_loge("\n");
		return -1;
	}

	scf_3ac_code_t* jmp = scf_3ac_jmp_code(SCF_OP_GOTO, NULL, NULL);

	scf_list_add_tail(d->_3ac_list_head, &jmp->list);

	scf_vector_add(d->branch_ops->_breaks, jmp);
	return 0;
}

static int _scf_op_continue(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	scf_handler_data_t* d  = data;
	scf_node_t*         up = (scf_node_t*)ast->current_block;

	while (up
			&& SCF_OP_WHILE  != up->type
			&& SCF_OP_DO     != up->type
			&& SCF_OP_FOR    != up->type
			&& SCF_OP_SWITCH != up->type)
		up = up->parent;

	if (!up) {
		scf_loge("\n");
		return -1;
	}

	scf_3ac_code_t* jmp = scf_3ac_jmp_code(SCF_OP_GOTO, NULL, NULL);

	scf_list_add_tail(d->_3ac_list_head, &jmp->list);

	scf_vector_add(d->branch_ops->_continues, jmp);
	return 0;
}

static int _scf_op_label(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	return 0;
}

static int _scf_op_goto(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(1 == nb_nodes);

	scf_handler_data_t* d  = data;
	scf_node_t*         nl = nodes[0];
	scf_label_t*        l  = nl->label;

	assert(SCF_LABEL == nl->type);
	assert(l->w);

	scf_3ac_operand_t* dst;
	scf_3ac_code_t*    c;
	scf_3ac_code_t*    jmp = scf_3ac_jmp_code(SCF_OP_GOTO, l, NULL);

	scf_list_add_tail(d->_3ac_list_head, &jmp->list);

	int i;
	for (i = 0; i < d->branch_ops->_labels->size; i++) {
		c  =        d->branch_ops->_labels->data[i];

		if (!strcmp(l->w->text->data, c->label->w->text->data)) {
			dst = jmp->dsts->data[0];
			dst->code = c;
			break;
		}
	}

	scf_vector_add(d->branch_ops->_gotos, jmp);
	return 0;
}

static int _scf_op_cond(scf_ast_t* ast, scf_expr_t* e, scf_handler_data_t* d)
{
	while (e && SCF_OP_EXPR == e->type)
		e = e->nodes[0];

	assert(e);

	if (_scf_expr_calculate_internal(ast, e, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	int is_float   =  0;
	int is_default =  0;
	int jmp_op     = -1;

	if (e->nb_nodes > 0) {

		scf_node_t*     node = e->nodes[0];
		scf_variable_t* v    = _scf_operand_get(node);

		if (scf_variable_float(v))
			is_float = 1;
	}

	switch (e->type) {
		case SCF_OP_EQ:
			jmp_op = SCF_OP_3AC_JNZ;
			break;
		case SCF_OP_NE:
			jmp_op = SCF_OP_3AC_JZ;
			break;

		case SCF_OP_GT:
			if (!is_float)
				jmp_op = SCF_OP_3AC_JLE;
			else
				jmp_op = SCF_OP_3AC_JBE;
			break;

		case SCF_OP_GE:
			if (!is_float)
				jmp_op = SCF_OP_3AC_JLT;
			else
				jmp_op = SCF_OP_3AC_JB;
			break;

		case SCF_OP_LT:
			if (!is_float)
				jmp_op = SCF_OP_3AC_JGE;
			else
				jmp_op = SCF_OP_3AC_JAE;
			break;

		case SCF_OP_LE:
			if (!is_float)
				jmp_op = SCF_OP_3AC_JGT;
			else
				jmp_op = SCF_OP_3AC_JA;
			break;

		case SCF_OP_LOGIC_NOT:
			jmp_op = SCF_OP_3AC_JNZ;
			break;

		default:
			if (scf_type_is_assign(e->type)) {

				e = e->nodes[0];

				while (e && SCF_OP_EXPR == e->type)
					e = e->nodes[0];

				assert(e);

				if (_scf_expr_calculate_internal(ast, e, d) < 0) {
					scf_loge("\n");
					return -1;
				}
			}

			if (_scf_3ac_code_1(d->_3ac_list_head, SCF_OP_3AC_TEQ, e) < 0) {
				scf_loge("\n");
				return -1;
			}

			jmp_op     = SCF_OP_3AC_JZ;
			is_default = 1;
			break;
	};

	if (!is_default) {

		scf_list_t*     l    = scf_list_tail(d->_3ac_list_head);
		scf_3ac_code_t* c    = scf_list_data(l, scf_3ac_code_t, list);
		scf_vector_t*   dsts = c->dsts;

		if (SCF_OP_LOGIC_NOT == e->type)
			c->op  = scf_3ac_find_operator(SCF_OP_3AC_TEQ);
		else
			c->op  = scf_3ac_find_operator(SCF_OP_3AC_CMP);
		c->dsts = NULL;

		scf_vector_clear(dsts, ( void (*)(void*) ) scf_3ac_operand_free);
		scf_vector_free (dsts);
		dsts = NULL;
	}

	return jmp_op;
}

static int _scf_op_node(scf_ast_t* ast, scf_node_t* node, scf_handler_data_t* d)
{
	scf_operator_t* op = node->op;

	if (!op) {
		op = scf_find_base_operator_by_type(node->type);
		if (!op) {
			scf_loge("\n");
			return -1;
		}
	}

	scf_operator_handler_t*	h = scf_find_3ac_operator_handler(op->type);
	if (!h) {
		scf_loge("\n");
		return -1;
	}

	if (h->func(ast, node->nodes, node->nb_nodes, d) < 0) {
		scf_loge("\n");
		return -1;
	}
	return 0;
}

static int _scf_op_if(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	if (2 != nb_nodes && 3 != nb_nodes) {
		scf_loge("\n");
		return -1;
	}

	scf_handler_data_t* d = data;

	scf_expr_t* e      = nodes[0];
	scf_node_t* parent = e->parent;

	int jmp_op = _scf_op_cond(ast, e, d);
	if (jmp_op < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_3ac_operand_t* dst;
	scf_3ac_code_t*    jmp_else  = scf_3ac_jmp_code(jmp_op, NULL, NULL);
	scf_3ac_code_t*    jmp_endif = NULL;
	scf_list_t*        l;

	scf_list_add_tail(d->_3ac_list_head, &jmp_else->list);

	int i;
	for (i = 1; i < nb_nodes; i++) {
		scf_node_t* node = nodes[i];

		if (_scf_op_node(ast, node, d) < 0) {
			scf_loge("\n");
			return -1;
		}

		if (1 == i) {
			if (3 == nb_nodes) {
				jmp_endif = scf_3ac_jmp_code(SCF_OP_GOTO, NULL, NULL);
				scf_list_add_tail(d->_3ac_list_head, &jmp_endif->list);
			}

			l   = scf_list_tail(d->_3ac_list_head);
			dst = jmp_else->dsts->data[0];
			dst->code = scf_list_data(l, scf_3ac_code_t, list);
		}
	}

	int ret = scf_vector_add(d->branch_ops->_breaks, jmp_else);
	if (ret < 0)
		return ret;

	if (jmp_endif) {
		l   = scf_list_tail(d->_3ac_list_head);
		dst = jmp_endif->dsts->data[0];
		dst->code = scf_list_data(l, scf_3ac_code_t, list);

		ret = scf_vector_add(d->branch_ops->_breaks, jmp_endif);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int _scf_op_end_loop(scf_list_t* start_prev, scf_list_t* continue_prev, scf_3ac_code_t* jmp_end, scf_branch_ops_t* up_branch_ops, scf_handler_data_t* d)
{
	// change 'while (cond) {} ' to
	// 'if (cond) {
	//    do {
    //    } while (cond);
    //  }'
	// for optimizer

	// copy cond expr
	scf_3ac_operand_t* dst;
	scf_3ac_code_t*    c;
	scf_3ac_code_t*    c2;

	scf_list_t*        l;
	scf_list_t*        l2;
	scf_list_t*        cond_prev = scf_list_tail(d->_3ac_list_head);

	for (l = scf_list_next(start_prev); l != &jmp_end->list; l = scf_list_next(l)) {
		c  = scf_list_data(l, scf_3ac_code_t, list);

		c2 = scf_3ac_code_clone(c);
		if (!c2)
			return -ENOMEM;

		scf_list_add_tail(d->_3ac_list_head, &c2->list);
	}

	for (l = scf_list_next(cond_prev); l != scf_list_sentinel(d->_3ac_list_head); l = scf_list_next(l)) {
		c  = scf_list_data(l, scf_3ac_code_t, list);

		if (!scf_type_is_jmp(c->op->type))
			continue;

		for (l2 = scf_list_next(cond_prev); l2 != scf_list_sentinel(d->_3ac_list_head); l2 = scf_list_next(l2)) {
			c2  = scf_list_data(l2, scf_3ac_code_t, list);

			dst = c->dsts->data[0];

			if (dst->code == c2->origin) {
				dst->code =  c2;
				break;
			}
		}
		assert(l2 != scf_list_sentinel(d->_3ac_list_head));

		if (scf_vector_add(d->branch_ops->_breaks, c) < 0)
			return -1;
	}

	int jmp_op = -1;
	switch (jmp_end->op->type) {

		case SCF_OP_3AC_JNZ:
			jmp_op = SCF_OP_3AC_JZ;
			break;
		case SCF_OP_3AC_JZ:
			jmp_op = SCF_OP_3AC_JNZ;
			break;
		case SCF_OP_3AC_JLE:
			jmp_op = SCF_OP_3AC_JGT;
			break;
		case SCF_OP_3AC_JLT:
			jmp_op = SCF_OP_3AC_JGE;
			break;
		case SCF_OP_3AC_JGE:
			jmp_op = SCF_OP_3AC_JLT;
			break;
		case SCF_OP_3AC_JGT:
			jmp_op = SCF_OP_3AC_JLE;
			break;

		case SCF_OP_3AC_JA:
			jmp_op = SCF_OP_3AC_JBE;
			break;
		case SCF_OP_3AC_JAE:
			jmp_op = SCF_OP_3AC_JB;
			break;

		case SCF_OP_3AC_JB:
			jmp_op = SCF_OP_3AC_JAE;
			break;
		case SCF_OP_3AC_JBE:
			jmp_op = SCF_OP_3AC_JA;
			break;

		default:
			jmp_op = -1;
			break;
	};

	// add loop when true
	scf_3ac_code_t*	loop = scf_3ac_jmp_code(jmp_op, NULL, NULL);
	scf_list_add_tail(d->_3ac_list_head, &loop->list);

	// should get the real start here,
	scf_3ac_code_t*	start = scf_list_data(scf_list_next(&jmp_end->list), scf_3ac_code_t, list);

	dst       = loop->dsts->data[0];
	dst->code = start;

	// set jmp destination for 'continue',
	// it's the 'real' dst & needs not to re-fill

	int i;
	for (i = 0; i < d->branch_ops->_continues->size; i++) {
		c  =        d->branch_ops->_continues->data[i];

		assert(c->dsts);

		dst = c->dsts->data[0];
		assert(!dst->code);

		/* 'continue' will goto 'while' and re-check the condition.

		   don't goto 'do' directly, because this will jmp the cond check, and may cause a dead loop.

		   if (cond) {
		       do {
			   }
			   while (cond_)
	       }
		*/

		if (continue_prev)
			dst->code = scf_list_data(scf_list_next(continue_prev), scf_3ac_code_t, list);
		else
			dst->code = scf_list_data(scf_list_next(cond_prev), scf_3ac_code_t, list);
	}

	// get the end, it's NOT the 'real' dst, but the prev of the 'real'
	scf_3ac_code_t*	end_prev  = scf_list_data(scf_list_tail(d->_3ac_list_head), scf_3ac_code_t, list);

	for (i = 0; i < d->branch_ops->_breaks->size; i++) {
		c  =        d->branch_ops->_breaks->data[i];

		assert(c->dsts);

		dst = c->dsts->data[0];

		if (!dst->code)
			dst->code = end_prev;
	}

	if (jmp_end) {
		dst       = jmp_end->dsts->data[0];
		dst->code = end_prev;
	}

	if (up_branch_ops) {
		for (i = 0; i < d->branch_ops->_breaks->size; i++) {
			c  =        d->branch_ops->_breaks->data[i];

			if (scf_vector_add(up_branch_ops->_breaks, c) < 0)
				return -1;
		}

		if (jmp_end) {
			if (scf_vector_add(up_branch_ops->_breaks, jmp_end) < 0)
				return -1;
		}

		for (i = 0; i < d->branch_ops->_gotos->size; i++) {
			c  =        d->branch_ops->_gotos->data[i];

			if (scf_vector_add(up_branch_ops->_gotos, c) < 0)
				return -1;
		}

		for (i = 0; i < d->branch_ops->_ends->size; i++) {
			c  =        d->branch_ops->_ends->data[i];

			if (scf_vector_add(up_branch_ops->_ends, c) < 0)
				return -1;
		}
	}

	return 0;
}

static int _scf_op_do(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_expr_t*         e = nodes[1];

	assert(SCF_OP_EXPR == e->type);

	scf_list_t* start_prev = scf_list_tail(d->_3ac_list_head);

	int jmp_op = _scf_op_cond(ast, e, d);
	if (jmp_op < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_list_t*     l;
	scf_3ac_code_t* c;
	scf_3ac_code_t* jmp_end = scf_3ac_jmp_code(jmp_op, NULL, NULL);

	scf_list_add_tail(d->_3ac_list_head, &jmp_end->list);

	scf_branch_ops_t* local_branch_ops = scf_branch_ops_alloc();
	scf_branch_ops_t* up_branch_ops    = d->branch_ops;
	d->branch_ops                      = local_branch_ops;

	if (_scf_op_node(ast, nodes[0], d) < 0) {
		scf_loge("\n");
		return -1;
	}

	if (_scf_op_end_loop(start_prev, NULL, jmp_end, up_branch_ops, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	d->branch_ops    = up_branch_ops;
	scf_branch_ops_free(local_branch_ops);
	local_branch_ops = NULL;

	// delete 'cond check' at 'start of loop'
	scf_vector_del(d->branch_ops->_breaks, jmp_end);

	for (l = scf_list_next(start_prev); l != &jmp_end->list; ) {
		c  = scf_list_data(l, scf_3ac_code_t, list);
		l  = scf_list_next(l);

		scf_list_del(&c->list);
		scf_3ac_code_free(c);
		c = NULL;
	}

	scf_list_del(&jmp_end->list);
	scf_3ac_code_free(jmp_end);
	jmp_end = NULL;

	return 0;
}

static int _scf_op_while(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes || 1 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_expr_t*         e = nodes[0];

	assert(SCF_OP_EXPR == e->type);

	// we don't know the real start of the while loop here,
	// we only know it's the next of 'start_prev'
	scf_list_t* start_prev = scf_list_tail(d->_3ac_list_head);

	int jmp_op = _scf_op_cond(ast, e, d);
	if (jmp_op < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_3ac_code_t* jmp_end = scf_3ac_jmp_code(jmp_op, NULL, NULL);
	scf_list_add_tail(d->_3ac_list_head, &jmp_end->list);

	scf_branch_ops_t* local_branch_ops = scf_branch_ops_alloc();
	scf_branch_ops_t* up_branch_ops    = d->branch_ops;
	d->branch_ops                      = local_branch_ops;

	// while body
	if (2 == nb_nodes) {
		if (_scf_op_node(ast, nodes[1], d) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	if (_scf_op_end_loop(start_prev, NULL, jmp_end, up_branch_ops, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	d->branch_ops    = up_branch_ops;
	scf_branch_ops_free(local_branch_ops);
	local_branch_ops = NULL;
	return 0;
}

static int _scf_op_default(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	return 0;
}

static int _scf_op_case(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	return 0;
}

static int _scf_op_switch(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;
	scf_expr_t*         e = nodes[0];
	scf_node_t*         b = nodes[1];

	assert(SCF_OP_EXPR == e->type);

	while (e && SCF_OP_EXPR == e->type)
		e = e->nodes[0];

	if (_scf_expr_calculate_internal(ast, e, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_branch_ops_t* up_branch_ops = d->branch_ops;
	scf_block_t*      up            = ast->current_block;

	d->branch_ops      = scf_branch_ops_alloc();
	ast->current_block = (scf_block_t*)b;

	scf_3ac_operand_t* dst;
	scf_3ac_code_t*    cmp;
	scf_3ac_code_t*    end;
	scf_3ac_code_t*    c;
	scf_3ac_code_t*    jnot  = NULL;
	scf_3ac_code_t*    jnext = NULL;

	scf_node_t*        child;
	scf_expr_t*        e2;
	scf_list_t*        l;

	int i;
	for (i = 0; i < b->nb_nodes; i++) {
		child     = b->nodes[i];

		if (SCF_OP_CASE == child->type || SCF_OP_DEFAULT == child->type) {

			if (jnot) {
				jnext = scf_3ac_jmp_code(SCF_OP_GOTO, NULL, NULL);

				scf_list_add_tail(d->_3ac_list_head, &jnext->list);
				scf_vector_add(up_branch_ops->_breaks, jnext);

				dst       = jnot->dsts->data[0];
				dst->code = jnext;
				jnot      = NULL;
			}

			if (SCF_OP_CASE == child->type) {

				e2 = child->nodes[0];
				assert(SCF_OP_EXPR == e2->type);

				while (e2 && SCF_OP_EXPR == e2->type)
					e2 = e2->nodes[0];

				if (_scf_expr_calculate_internal(ast, e2, d) < 0) {
					scf_loge("\n");
					return -1;
				}

				scf_node_t* srcs[2] = {e, e2};

				cmp  = scf_3ac_code_NN(SCF_OP_3AC_CMP, NULL, 0, srcs, 2);
				jnot = scf_3ac_jmp_code(SCF_OP_3AC_JNZ, NULL, NULL);

				scf_list_add_tail(d->_3ac_list_head, &cmp->list);
				scf_list_add_tail(d->_3ac_list_head, &jnot->list);

				scf_vector_add(up_branch_ops->_breaks, jnot);
			}

			if (jnext) {
				l         = scf_list_tail(d->_3ac_list_head);
				dst       = jnext->dsts->data[0];
				dst->code = scf_list_data(l, scf_3ac_code_t, list);
				jnext     = NULL;
			}

		} else {
			if (_scf_op_node(ast, child, d) < 0) {
				scf_loge("\n");
				return -1;
			}
		}
	}

	l   = scf_list_tail(d->_3ac_list_head);
	end = scf_list_data(l, scf_3ac_code_t, list);

	for (i = 0; i < d->branch_ops->_breaks->size; i++) {
		c  =        d->branch_ops->_breaks->data[i];

		dst = c->dsts->data[0];
		if (!dst->code)
			dst->code = end;

		if (scf_vector_add(up_branch_ops->_breaks, c) < 0)
			return -1;
	}

	for (i = 0; i < d->branch_ops->_gotos->size; i++) {
		c  =        d->branch_ops->_gotos->data[i];

		if (scf_vector_add(up_branch_ops->_gotos, c) < 0)
			return -1;
	}

	for (i = 0; i < d->branch_ops->_ends->size; i++) {
		c         = d->branch_ops->_ends->data[i];

		if (scf_vector_add(up_branch_ops->_ends, c) < 0)
			return -1;
	}

	scf_branch_ops_free(d->branch_ops);
	d->branch_ops      = up_branch_ops;
	ast->current_block = up;
	return 0;
}

static int _scf_op_for(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(4 == nb_nodes);

	scf_handler_data_t* d = data;

	if (nodes[0]) {
		if (_scf_op_node(ast, nodes[0], d) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	scf_list_t*     start_prev = scf_list_tail(d->_3ac_list_head);
	scf_3ac_code_t* jmp_end    = NULL;

	if (nodes[1]) {
		assert(SCF_OP_EXPR == nodes[1]->type);

		int jmp_op = _scf_op_cond(ast, nodes[1], d);
		if (jmp_op < 0) {
			scf_loge("\n");
			return -1;
		}

		jmp_end = scf_3ac_jmp_code(jmp_op, NULL, NULL);
		scf_list_add_tail(d->_3ac_list_head, &jmp_end->list);
	}

	scf_branch_ops_t* local_branch_ops = scf_branch_ops_alloc();
	scf_branch_ops_t* up_branch_ops    = d->branch_ops;
	d->branch_ops                      = local_branch_ops;

	if (nodes[3]) {
		if (_scf_op_node(ast, nodes[3], d) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	scf_list_t* continue_prev = scf_list_tail(d->_3ac_list_head);

	if (nodes[2]) {
		if (_scf_op_node(ast, nodes[2], d) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	if (_scf_op_end_loop(start_prev, continue_prev, jmp_end, up_branch_ops, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	d->branch_ops    = up_branch_ops;
	scf_branch_ops_free(local_branch_ops);
	local_branch_ops = NULL;
	return 0;
}

static int __scf_op_call(scf_ast_t* ast, scf_function_t* f, void* data)
{
	scf_logd("f: %p, f->node->w: %s\n", f, f->node.w->text->data);

	scf_handler_data_t* d = data;

	scf_block_t*       up               = ast->current_block;
	scf_branch_ops_t*  local_branch_ops = scf_branch_ops_alloc();
	scf_branch_ops_t*  tmp_branch_ops   = d->branch_ops;

	ast->current_block = (scf_block_t*)f;
	d->branch_ops      = local_branch_ops; // use local_branch_ops, because branch code should NOT jmp over the function block

	if (_scf_op_block(ast, f->node.nodes, f->node.nb_nodes, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_list_t*        next;
	scf_3ac_operand_t* dst;
	scf_3ac_code_t*    c;
	scf_3ac_code_t*    end = scf_3ac_code_NN(SCF_OP_3AC_END, NULL, 0, NULL, 0);

	scf_list_add_tail(d->_3ac_list_head, &end->list);

	// re-fill 'break'

	int i;
	for (i = 0; i < local_branch_ops->_breaks->size; i++) {
		c  =        local_branch_ops->_breaks->data[i];

		dst = c->dsts->data[0];

		if (dst->code) {
			next      = scf_list_next(&dst->code->list);
			dst->code = scf_list_data(next, scf_3ac_code_t, list);
		} else {
			scf_loge("'break' has a bug!\n");
			return -1;
		}
	}

	// re-fill 'goto'
	for (i = 0; i < local_branch_ops->_gotos->size; i++) {
		c  =        local_branch_ops->_gotos->data[i];

		dst = c->dsts->data[0];

		if (dst->code) {
			next      = scf_list_next(&dst->code->list);
			dst->code = scf_list_data(next, scf_3ac_code_t, list);
		} else {
			scf_loge("all 'goto' should get its label in this function\n");
			return -1;
		}
	}

	// re-fill 'end'
	for (i = 0; i < local_branch_ops->_ends->size; i++) {
		c  =        local_branch_ops->_ends->data[i];

		dst = c->dsts->data[0];

		assert(!dst->code);

		if (&c->list == scf_list_prev(&end->list))
			c->op = scf_3ac_find_operator(SCF_OP_3AC_NOP);
		else
			dst->code = end;
	}

	scf_branch_ops_free(local_branch_ops);
	local_branch_ops   = NULL;

	d->branch_ops      = tmp_branch_ops;
	ast->current_block = up;
	return 0;
}

int scf_function_to_3ac(scf_ast_t* ast, scf_function_t* f, scf_list_t* _3ac_list_head)
{
	scf_handler_data_t d = {0};
	d._3ac_list_head  	 = _3ac_list_head;

	int ret = __scf_op_call(ast, f, &d);

	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	return 0;
}

static int _scf_op_create(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(nb_nodes > 0);

	scf_handler_data_t* d      = data;
	scf_node_t*         parent = nodes[0]->parent;

	scf_3ac_operand_t*  dst;
	scf_3ac_code_t*     jz;
	scf_3ac_code_t*     jmp;

	scf_variable_t*     v;
	scf_type_t*         t;
	scf_node_t*         node;
	scf_node_t*         nthis;
	scf_node_t*         nerr;
	scf_list_t*         l;

	int ret;
	int i;

	for (i = 3; i < nb_nodes; i++) {

		ret = _scf_expr_calculate_internal(ast, nodes[i], d);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	nthis = parent->result_nodes->data[0];
	nerr  = parent->result_nodes->data[1];

	nthis->type         = SCF_OP_CALL;
	nthis->result       = nthis->var;
	nthis->var          = NULL;
	nthis->op           = scf_find_base_operator_by_type(SCF_OP_CALL);
	nthis->split_flag   = 0;
	nthis->split_parent = NULL;
	scf_node_add_child(nthis, nodes[0]);
	scf_node_add_child(nthis, nodes[1]);

	nerr->type         = SCF_OP_CALL;
	nerr->result       = nerr->var;
	nerr->var          = NULL;
	nerr->op           = scf_find_base_operator_by_type(SCF_OP_CALL);
	nerr->split_flag   = 0;
	nerr->split_parent = NULL;

	for (i = 2; i < nb_nodes; i++)
		scf_node_add_child(nerr, nodes[i]);

	for (i = 1; i < nb_nodes; i++)
		nodes[i] = NULL;

	parent->nodes[0] = nerr;
	parent->nb_nodes = 1;
	nerr->parent     = parent;

	v = _scf_operand_get(nthis);
	v->tmp_flag = 1;

	v = _scf_operand_get(nerr);
	v->tmp_flag = 1;

	nthis->_3ac_done = 1;
	nerr ->_3ac_done = 1;

	ret = _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_CALL, nthis, nthis->nodes, nthis->nb_nodes);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ret = _scf_3ac_code_1(d->_3ac_list_head, SCF_OP_3AC_TEQ, nthis);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	jz  = scf_3ac_jmp_code(SCF_OP_3AC_JZ, NULL, NULL);
	scf_list_add_tail(d->_3ac_list_head, &jz->list);

	ret = _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_CALL, nerr, nerr->nodes, nerr->nb_nodes);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	jmp = scf_3ac_jmp_code(SCF_OP_GOTO,   NULL, NULL);
	scf_list_add_tail(d->_3ac_list_head, &jmp->list);

	scf_vector_add(d->branch_ops->_breaks, jz);
	scf_vector_add(d->branch_ops->_breaks, jmp);

	dst = jz->dsts->data[0];
	dst->code = jmp;

	t = scf_block_find_type_type(ast->current_block, SCF_VAR_INT);
	v = SCF_VAR_ALLOC_BY_TYPE(NULL, t, 1, 0, NULL);
	if (!v)
		return -ENOMEM;

	node = scf_node_alloc(NULL, v->type, v);
	if (!node) {
		scf_variable_free(v);
		return -ENOMEM;
	}
	v->data.i64 = -ENOMEM;

	ret = _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_ASSIGN, nerr, &node, 1);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	l   = scf_list_tail(d->_3ac_list_head);
	dst = jmp->dsts->data[0];
	dst->code = scf_list_data(l, scf_3ac_code_t, list);

	return 0;
}

static int _scf_op_call(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(nb_nodes > 0);

	scf_handler_data_t* d      = data;
	scf_variable_t*     v      = NULL;
	scf_function_t*     f      = NULL;
	scf_vector_t*       argv   = NULL;
	scf_node_t*         parent = nodes[0]->parent;

	int i;
	int ret = _scf_expr_calculate_internal(ast, nodes[0], d);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	v = _scf_operand_get(nodes[0]);
	f = v->func_ptr;

	if (f->vargs_flag) {
		if (f->argv->size > nb_nodes - 1)
			return -1;
	} else if (f->argv->size != nb_nodes - 1)
		return -1;

	argv = scf_vector_alloc();
	if (!argv) {
		scf_loge("\n");
		return -ENOMEM;
	}

	ret = scf_vector_add(argv, nodes[0]);
	if (ret < 0) {
		scf_vector_free(argv);
		return ret;
	}

	for (i = 1; i < nb_nodes; i++) {
		scf_node_t*     arg   = nodes[i];
		scf_node_t*     child = NULL;

		while (SCF_OP_EXPR == arg->type)
			arg = arg->nodes[0];

		if (scf_type_is_assign(arg->type)) {

			assert(2 == arg->nb_nodes);
			child     = arg->nodes[0];

			child->_3ac_done = 0;

			ret = _scf_expr_calculate_internal(ast, child, d);
			if (ret < 0) {
				scf_vector_free(argv);
				return ret;
			}

			arg = child;
		}

		ret = scf_vector_add(argv, arg);
		if (ret < 0) {
			scf_vector_free(argv);
			return ret;
		}

		v = _scf_operand_get(arg);

		if (!v->global_flag
				&& !v->static_flag
				&& !v->member_flag
				&& !v->local_flag
				&& !v->const_flag)
			v->tmp_flag = 1;
	}

	if (parent->result_nodes) {

		scf_node_t* node;

		for (i = 0; i < parent->result_nodes->size; i++) {
			node      = parent->result_nodes->data[i];

			v = _scf_operand_get(node);

			if (SCF_VAR_VOID == v->type && 0 == v->nb_pointers)
				v->const_flag = 1;
			v->tmp_flag = 1;
		}

		ret = _scf_3ac_code_NN(d->_3ac_list_head, SCF_OP_CALL,
				(scf_node_t**)parent->result_nodes->data, parent->result_nodes->size,
				(scf_node_t**)argv->data, argv->size);

	} else {
		v = _scf_operand_get(parent);
		if (v) {
			if (SCF_VAR_VOID == v->type && 0 == v->nb_pointers)
				v->const_flag = 1;
			v->tmp_flag = 1;
		}

		ret = _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_CALL, parent, (scf_node_t**)argv->data, argv->size);
	}

	scf_vector_free(argv);
	return ret;
}

static int _scf_op_expr(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
#if 1
	assert(1 == nb_nodes);

	scf_handler_data_t* d = data;

	int ret = _scf_expr_calculate_internal(ast, nodes[0], d);
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}
#endif
	return 0;
}

#define SCF_OP_UNARY(name, op_type) \
static int _scf_op_##name(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(1 == nb_nodes); \
	scf_handler_data_t* d = data; \
	scf_node_t* parent    = nodes[0]->parent; \
	\
	_scf_operand_get(parent)->tmp_flag = 1; \
	\
	return _scf_3ac_code_2(d->_3ac_list_head, op_type, parent, nodes[0]); \
}

SCF_OP_UNARY(neg,         SCF_OP_NEG)
SCF_OP_UNARY(positive,    SCF_OP_POSITIVE)
SCF_OP_UNARY(dereference, SCF_OP_DEREFERENCE)
SCF_OP_UNARY(logic_not,   SCF_OP_LOGIC_NOT)
SCF_OP_UNARY(bit_not,     SCF_OP_BIT_NOT)

static int _scf_op_address_of(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(1 == nb_nodes);

	scf_handler_data_t* d = data;

	scf_node_t* parent    = nodes[0]->parent;
	scf_node_t* child     = nodes[0];

	if (scf_type_is_var(child->type))
		return _scf_3ac_code_2(d->_3ac_list_head, SCF_OP_ADDRESS_OF, parent, child);

	if (SCF_OP_ARRAY_INDEX == child->type) {
		assert(2 == child->nb_nodes);

		scf_node_t* n_scale = NULL;
		scf_node_t* srcs[3];

		int i;
		for (i = 0; i < 2; i++) {
			if (_scf_expr_calculate_internal(ast, child->nodes[i], d) < 0) {
				scf_loge("\n");
				return -1;
			}
		}

		int ret = _scf_op_array_scale(ast, child, &n_scale);
		if (ret < 0)
			return ret;

		srcs[0] = child->nodes[0];
		srcs[1] = child->nodes[1];
		srcs[2] = n_scale;

		return _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_3AC_ADDRESS_OF_ARRAY_INDEX, parent, srcs, 3);
	}

	if (SCF_OP_POINTER == child->type) {
		assert(2 == child->nb_nodes);

		scf_node_t* srcs[2];

		int i;
		for (i = 0; i < 2; i++) {
			if (_scf_expr_calculate_internal(ast, child->nodes[i], d) < 0) {
				scf_loge("\n");
				return -1;
			}
		}

		srcs[0] = child->nodes[0];
		srcs[1] = child->nodes[1];

		return _scf_3ac_code_N(d->_3ac_list_head, SCF_OP_3AC_ADDRESS_OF_POINTER, parent, srcs, 2);
	}

	scf_loge("\n");
	return -1;
}

static int _scf_op_type_cast(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);

	scf_handler_data_t* d = data;

	scf_node_t* parent = nodes[0]->parent;

	_scf_operand_get(parent)->tmp_flag = 1;

	return _scf_3ac_code_2(d->_3ac_list_head, SCF_OP_TYPE_CAST, parent, nodes[1]);
}

#define SCF_OP_BINARY(name, op_type) \
static int _scf_op_##name(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(2 == nb_nodes); \
	scf_handler_data_t* d      = data; \
	scf_node_t*         parent = nodes[0]->parent; \
	\
	_scf_operand_get(parent)->tmp_flag = 1; \
	\
	return _scf_3ac_code_3(d->_3ac_list_head, op_type, parent, nodes[0], nodes[1]); \
}

SCF_OP_BINARY(add,     SCF_OP_ADD)
SCF_OP_BINARY(sub,     SCF_OP_SUB)
SCF_OP_BINARY(mul,     SCF_OP_MUL)
SCF_OP_BINARY(div,     SCF_OP_DIV)
SCF_OP_BINARY(mod,     SCF_OP_MOD)
SCF_OP_BINARY(shl,     SCF_OP_SHL)
SCF_OP_BINARY(shr,     SCF_OP_SHR)
SCF_OP_BINARY(bit_and, SCF_OP_BIT_AND)
SCF_OP_BINARY(bit_or,  SCF_OP_BIT_OR)

static int _scf_op_left_value_array_index(scf_ast_t* ast, int type, scf_node_t* left, scf_node_t* right, scf_handler_data_t* d)
{
	assert(2 == left->nb_nodes);

	scf_node_t* n_index = NULL;
	scf_node_t* n_scale = NULL;
	scf_node_t* srcs[4];

	int i;
	for (i = 0; i < 2; i++) {
		if (_scf_expr_calculate_internal(ast, left->nodes[i], d) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	n_index = left->nodes[1];
	while (SCF_OP_EXPR == n_index->type)
		n_index = n_index->nodes[0];

	int ret = _scf_op_array_scale(ast, left, &n_scale);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	srcs[0]   = left->nodes[0];
	srcs[1]   = n_index;
	srcs[i++] = n_scale;
	if (right)
		srcs[i++] = right;

	if (_scf_3ac_code_srcN(d->_3ac_list_head, type, srcs, i) < 0) {
		scf_loge("\n");
		return -1;
	}
	return 0;
}

static int _scf_op_left_value(scf_ast_t* ast, int type, scf_node_t* left, scf_node_t* right, scf_handler_data_t* d)
{
	assert(1 == left->nb_nodes || 2 == left->nb_nodes);

	scf_node_t* srcs[3];

	int i;
	for (i = 0; i < left->nb_nodes; i++) {
		if (_scf_expr_calculate_internal(ast, left->nodes[i], d) < 0) {
			scf_loge("\n");
			return -1;
		}

		srcs[i] = left->nodes[i];
	}

	if (right)
		srcs[i++] = right;

	assert(i <= 3);

	if (_scf_3ac_code_srcN(d->_3ac_list_head, type, srcs, i) < 0) {
		scf_loge("\n");
		return -1;
	}
	return 0;
}

static int _scf_op_right_value(scf_ast_t* ast, scf_node_t** pright, scf_handler_data_t* d)
{
	scf_node_t* right = *pright;

	if (_scf_expr_calculate_internal(ast, right, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	if (scf_type_is_assign(right->type)) {
		right = right->nodes[0];

		while (SCF_OP_EXPR == right->type)
			right = right->nodes[0];

		assert(!scf_type_is_assign(right->type));

		right->_3ac_done = 0;

		if (_scf_expr_calculate_internal(ast, right, d) < 0) {
			scf_loge("\n");
			return -1;
		}

	} else {
		while (SCF_OP_EXPR == right->type)
			right = right->nodes[0];

		if (SCF_OP_CALL == right->type && !right->split_flag) {

			if (right->result_nodes) {

				assert(right->result_nodes->size > 0);

				right = right->result_nodes->data[0];
			} else
				assert(right->result);
		}
	}

	*pright = right;
	return 0;
}

static int _scf_op_assign(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data)
{
	assert(2 == nb_nodes);
	scf_handler_data_t* d = data;

	scf_node_t*     parent = nodes[0]->parent;
	scf_node_t*     node0  = nodes[0];
	scf_node_t*     node1  = nodes[1];
	scf_variable_t* v0     = _scf_operand_get(node0);

	if ( _scf_op_right_value(ast, &node1, d) < 0)
		return -1;

	while (SCF_OP_EXPR == node0->type)
		node0 = node0->nodes[0];

	switch (node0->type) {
		case SCF_OP_DEREFERENCE:
			return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_DEREFERENCE, node0, node1, d);
			break;
		case SCF_OP_ARRAY_INDEX:
			return _scf_op_left_value_array_index(ast, SCF_OP_3AC_ASSIGN_ARRAY_INDEX, node0, node1, d);
			break;
		case SCF_OP_POINTER:
			return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_POINTER, node0, node1, d);
			break;
		default:
			break;
	};

	return _scf_3ac_code_2(d->_3ac_list_head, SCF_OP_ASSIGN, node0, node1);
}

#define SCF_OP_BINARY_ASSIGN(name, op) \
static int _scf_op_##name##_assign(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(2 == nb_nodes); \
	scf_handler_data_t* d = data; \
	\
	scf_node_t*     parent = nodes[0]->parent; \
	scf_node_t*     node0  = nodes[0]; \
	scf_node_t*     node1  = nodes[1]; \
	scf_variable_t* v1     = _scf_operand_get(nodes[1]); \
	\
	if ( _scf_op_right_value(ast, &node1, d) < 0) \
		return -1; \
	\
	while (SCF_OP_EXPR == node0->type) \
		node0 = node0->nodes[0]; \
	\
	int is_float = scf_type_is_float(v1->type) && 0 == v1->nb_pointers; \
	if (is_float) { \
		if (_scf_expr_calculate_internal(ast, node0, d) < 0) { \
			scf_loge("\n"); \
			return -1; \
		} \
		\
		if ( _scf_3ac_code_2(d->_3ac_list_head, parent->type, node0, node1) < 0) { \
			scf_loge("\n"); \
			return -1; \
		} \
		\
		switch (node0->type) { \
			case SCF_OP_DEREFERENCE: \
				return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_DEREFERENCE, node0, node0, d); \
				break; \
			case SCF_OP_ARRAY_INDEX: \
				return _scf_op_left_value_array_index(ast, SCF_OP_3AC_ASSIGN_ARRAY_INDEX, node0, node0, d); \
				break; \
			case SCF_OP_POINTER: \
				return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_POINTER, node0, node0, d); \
				break; \
			default: \
				break; \
		}; \
		\
		return 0; \
	} \
	\
	switch (node0->type) { \
		case SCF_OP_DEREFERENCE: \
			return _scf_op_left_value(ast, SCF_OP_3AC_##op##_ASSIGN_DEREFERENCE, node0, node1, d); \
			break; \
		case SCF_OP_ARRAY_INDEX: \
			return _scf_op_left_value_array_index(ast, SCF_OP_3AC_##op##_ASSIGN_ARRAY_INDEX, node0, node1, d); \
			break; \
		case SCF_OP_POINTER: \
			return _scf_op_left_value(ast, SCF_OP_3AC_##op##_ASSIGN_POINTER, node0, node1, d); \
			break; \
		default: \
			break; \
	}; \
	\
	return _scf_3ac_code_2(d->_3ac_list_head, SCF_OP_##op##_ASSIGN, node0, node1); \
}

SCF_OP_BINARY_ASSIGN(add, ADD)
SCF_OP_BINARY_ASSIGN(sub, SUB)
SCF_OP_BINARY_ASSIGN(and, AND)
SCF_OP_BINARY_ASSIGN(or,  OR)

#define SCF_OP_BINARY_ASSIGN2(name, op) \
static int _scf_op_##name##_assign(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(2 == nb_nodes); \
	scf_handler_data_t* d = data; \
	\
	scf_node_t*     parent = nodes[0]->parent; \
	scf_node_t*     node0  = nodes[0]; \
	scf_node_t*     node1  = nodes[1]; \
	scf_variable_t* v1     = _scf_operand_get(nodes[1]); \
	\
	if ( _scf_op_right_value(ast, &node1, d) < 0) \
		return -1; \
	\
	while (SCF_OP_EXPR == node0->type) \
		node0 = node0->nodes[0]; \
	\
	if (_scf_expr_calculate_internal(ast, node0, d) < 0) { \
		scf_loge("\n"); \
		return -1; \
	} \
	\
	if ( _scf_3ac_code_2(d->_3ac_list_head, parent->type, node0, node1) < 0) { \
		scf_loge("\n"); \
		return -1; \
	} \
	\
	switch (node0->type) { \
		case SCF_OP_DEREFERENCE: \
			return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_DEREFERENCE, node0, node0, d); \
			break; \
		case SCF_OP_ARRAY_INDEX: \
			return _scf_op_left_value_array_index(ast, SCF_OP_3AC_ASSIGN_ARRAY_INDEX, node0, node0, d); \
			break; \
		case SCF_OP_POINTER: \
			return _scf_op_left_value(ast, SCF_OP_3AC_ASSIGN_POINTER, node0, node0, d); \
			break; \
		default: \
			break; \
	}; \
	\
	return 0; \
}
SCF_OP_BINARY_ASSIGN2(shl, SHL)
SCF_OP_BINARY_ASSIGN2(shr, SHR)
SCF_OP_BINARY_ASSIGN2(mul, MUL)
SCF_OP_BINARY_ASSIGN2(div, DIV)
SCF_OP_BINARY_ASSIGN2(mod, MOD)

#define SCF_OP_UNARY_ASSIGN(name, op) \
static int __scf_op_##name(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(1 == nb_nodes); \
	scf_handler_data_t* d = data; \
	\
	scf_node_t*     node0 = nodes[0]; \
	\
	while (SCF_OP_EXPR == node0->type) \
		node0 = node0->nodes[0]; \
	\
	switch (node0->type) { \
		case SCF_OP_DEREFERENCE: \
			return _scf_op_left_value(ast, SCF_OP_3AC_##op##_DEREFERENCE, node0, NULL, d); \
			break; \
		case SCF_OP_ARRAY_INDEX: \
			return _scf_op_left_value_array_index(ast, SCF_OP_3AC_##op##_ARRAY_INDEX, node0, NULL, d); \
			break; \
		case SCF_OP_POINTER: \
			return _scf_op_left_value(ast, SCF_OP_3AC_##op##_POINTER, node0, NULL, d); \
			break; \
		default: \
			break; \
	}; \
	\
	return _scf_3ac_code_1(d->_3ac_list_head, SCF_OP_3AC_##op, node0); \
}
SCF_OP_UNARY_ASSIGN(inc, INC)
SCF_OP_UNARY_ASSIGN(dec, DEC)

#define SCF_OP_UNARY_ASSIGN2(name0, name1, op_type, post_flag) \
static int _scf_op_##name0(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	assert(1 == nb_nodes); \
	scf_handler_data_t* d  = data; \
	\
	scf_node_t*     node0  = nodes[0]; \
	scf_node_t*     parent = nodes[0]->parent; \
	\
	int ret = __scf_op_##name1(ast, nodes, nb_nodes, data); \
	if (ret < 0) { \
		scf_loge("\n"); \
		return -1; \
	} \
	\
	while (SCF_OP_EXPR == node0->type) \
		node0 = node0->nodes[0]; \
	\
	scf_list_t*     l  = scf_list_tail(d->_3ac_list_head); \
	scf_3ac_code_t* c  = scf_list_data(l, scf_3ac_code_t, list); \
	\
	ret = _scf_3ac_code_2(d->_3ac_list_head, SCF_OP_ASSIGN, parent, node0); \
	if (ret < 0) { \
		scf_loge("\n"); \
		return -1; \
	} \
	_scf_operand_get(parent)->tmp_flag = 1; \
	\
	if (post_flag) { \
	    scf_list_del(&c->list); \
		scf_list_add_tail(d->_3ac_list_head, &c->list); \
	} \
	return 0; \
}
SCF_OP_UNARY_ASSIGN2(inc,      inc, INC, 0)
SCF_OP_UNARY_ASSIGN2(dec,      dec, DEC, 0)
SCF_OP_UNARY_ASSIGN2(inc_post, inc, INC, 1)
SCF_OP_UNARY_ASSIGN2(dec_post, dec, DEC, 1)

#define SCF_OP_CMP(name, op_type) \
static int _scf_op_##name(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{\
	assert(2 == nb_nodes);\
	scf_handler_data_t* d  = data;\
	scf_node_t*     parent = nodes[0]->parent; \
	\
	scf_variable_t* v0 = _scf_operand_get(nodes[0]);\
	scf_variable_t* v1 = _scf_operand_get(nodes[1]);\
	if (scf_variable_const(v0)) { \
		if (scf_variable_const(v1)) {\
			scf_loge("result to compare 2 const var should be calculated before\n"); \
			return -EINVAL; \
		} \
		int op_type2 = op_type; \
		switch (op_type) { \
			case SCF_OP_GT: \
				op_type2 = SCF_OP_LT; \
				break; \
			case SCF_OP_GE: \
				op_type2 = SCF_OP_LE; \
				break; \
			case SCF_OP_LE: \
				op_type2 = SCF_OP_GE; \
				break; \
			case SCF_OP_LT: \
				op_type2 = SCF_OP_GT; \
				break; \
		} \
		parent->type = op_type2; \
		SCF_XCHG(nodes[0], nodes[1]); \
		return _scf_3ac_code_3(d->_3ac_list_head, op_type2, parent, nodes[0], nodes[1]); \
	} \
	return _scf_3ac_code_3(d->_3ac_list_head, op_type, parent, nodes[0], nodes[1]); \
}

SCF_OP_CMP(eq, SCF_OP_EQ)
SCF_OP_CMP(ne, SCF_OP_NE)
SCF_OP_CMP(gt, SCF_OP_GT)
SCF_OP_CMP(ge, SCF_OP_GE)
SCF_OP_CMP(lt, SCF_OP_LT)
SCF_OP_CMP(le, SCF_OP_LE)

static int _scf_op_logic_and_jmp(scf_ast_t* ast, scf_node_t* node, scf_handler_data_t* d)
{
	scf_node_t* parent = node->parent;

	if (_scf_expr_calculate_internal(ast, node, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	int is_float   = 0;
	int is_default = 0;
	int jmp_op;
	int set_op;

	while (SCF_OP_EXPR == node->type)
		node = node->nodes[0];

	if (node->nb_nodes > 0) {

		scf_variable_t* v = _scf_operand_get(node->nodes[0]);

		if (scf_variable_float(v))
			is_float = 1;
	}

	switch (node->type) {
		case SCF_OP_EQ:
			set_op = SCF_OP_3AC_SETZ;
			jmp_op = SCF_OP_3AC_JNZ;
			break;
		case SCF_OP_NE:
			set_op = SCF_OP_3AC_SETNZ;
			jmp_op = SCF_OP_3AC_JZ;
			break;

		case SCF_OP_GT:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETGT;
				jmp_op = SCF_OP_3AC_JLE;
			} else {
				set_op = SCF_OP_3AC_SETA;
				jmp_op = SCF_OP_3AC_JBE;
			}
			break;

		case SCF_OP_GE:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETGE;
				jmp_op = SCF_OP_3AC_JLT;
			} else {
				set_op = SCF_OP_3AC_SETAE;
				jmp_op = SCF_OP_3AC_JB;
			}
			break;

		case SCF_OP_LT:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETLT;
				jmp_op = SCF_OP_3AC_JGE;
			} else {
				set_op = SCF_OP_3AC_SETB;
				jmp_op = SCF_OP_3AC_JAE;
			}
			break;
		case SCF_OP_LE:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETLE;
				jmp_op = SCF_OP_3AC_JGT;
			} else {
				set_op = SCF_OP_3AC_SETB;
				jmp_op = SCF_OP_3AC_JA;
			}
			break;

		default:
			if (_scf_3ac_code_1(d->_3ac_list_head, SCF_OP_3AC_TEQ, node) < 0) {
				scf_loge("\n");
				return -1;
			}

			if (_scf_3ac_code_dst(d->_3ac_list_head, SCF_OP_3AC_SETNZ, parent) < 0) {
				scf_loge("\n");
				return -1;
			}

			jmp_op     = SCF_OP_3AC_JZ;
			is_default = 1;
			break;
	};

	if (!is_default) {
		scf_list_t*     l    = scf_list_tail(d->_3ac_list_head);
		scf_3ac_code_t* c    = scf_list_data(l, scf_3ac_code_t, list);
		scf_vector_t*   dsts = c->dsts;

		c->op   = scf_3ac_find_operator(SCF_OP_3AC_CMP);
		c->dsts = NULL;

		scf_vector_clear(dsts, ( void (*)(void*) ) scf_3ac_operand_free);
		scf_vector_free (dsts);
		dsts = NULL;

		if (_scf_3ac_code_dst(d->_3ac_list_head, set_op, parent) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	scf_variable_t* v = _scf_operand_get(parent);
	v->tmp_flag = 1;
	return jmp_op;
}

static int _scf_op_logic_or_jmp(scf_ast_t* ast, scf_node_t* node, scf_handler_data_t* d)
{
	scf_node_t* parent = node->parent;

	if (_scf_expr_calculate_internal(ast, node, d) < 0) {
		scf_loge("\n");
		return -1;
	}

	int is_float   = 0;
	int is_default = 0;
	int jmp_op;
	int set_op;

	while (SCF_OP_EXPR == node->type)
		node = node->nodes[0];

	if (node->nb_nodes > 0) {

		scf_variable_t* v = _scf_operand_get(node->nodes[0]);

		if (scf_variable_float(v))
			is_float = 1;
	}

	switch (node->type) {
		case SCF_OP_EQ:
			set_op = SCF_OP_3AC_SETZ;
			jmp_op = SCF_OP_3AC_JZ;
			break;
		case SCF_OP_NE:
			set_op = SCF_OP_3AC_SETNZ;
			jmp_op = SCF_OP_3AC_JNZ;
			break;

		case SCF_OP_GT:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETGT;
				jmp_op = SCF_OP_3AC_JGT;
			} else {
				set_op = SCF_OP_3AC_SETA;
				jmp_op = SCF_OP_3AC_JA;
			}
			break;

		case SCF_OP_GE:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETGE;
				jmp_op = SCF_OP_3AC_JGE;
			} else {
				set_op = SCF_OP_3AC_SETAE;
				jmp_op = SCF_OP_3AC_JAE;
			}
			break;

		case SCF_OP_LT:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETLT;
				jmp_op = SCF_OP_3AC_JLT;
			} else {
				set_op = SCF_OP_3AC_SETB;
				jmp_op = SCF_OP_3AC_JB;
			}
			break;

		case SCF_OP_LE:
			if (!is_float) {
				set_op = SCF_OP_3AC_SETLE;
				jmp_op = SCF_OP_3AC_JLE;
			} else {
				set_op = SCF_OP_3AC_SETBE;
				jmp_op = SCF_OP_3AC_JBE;
			}
			break;

		default:
			if (_scf_3ac_code_1(d->_3ac_list_head, SCF_OP_3AC_TEQ, node) < 0) {
				scf_loge("\n");
				return -1;
			}

			if (_scf_3ac_code_dst(d->_3ac_list_head, SCF_OP_3AC_SETNZ, parent) < 0) {
				scf_loge("\n");
				return -1;
			}

			jmp_op     = SCF_OP_3AC_JNZ;
			is_default = 1;
			break;
	};

	if (!is_default) {
		scf_list_t*     l    = scf_list_tail(d->_3ac_list_head);
		scf_3ac_code_t* c    = scf_list_data(l, scf_3ac_code_t, list);
		scf_vector_t*   dsts = c->dsts;

		c->op   = scf_3ac_find_operator(SCF_OP_3AC_CMP);
		c->dsts = NULL;

		scf_vector_clear(dsts, ( void(*)(void*) ) scf_3ac_operand_free);
		scf_vector_free (dsts);
		dsts = NULL;

		if (_scf_3ac_code_dst(d->_3ac_list_head, set_op, parent) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	scf_variable_t* v = _scf_operand_get(parent);
	v->tmp_flag = 1;
	return jmp_op;
}

#define SCF_OP_LOGIC(name) \
static int _scf_op_logic_##name(scf_ast_t* ast, scf_node_t** nodes, int nb_nodes, void* data) \
{ \
	scf_handler_data_t* d = data; \
	scf_node_t* parent    = nodes[0]->parent; \
	int jmp_op = _scf_op_logic_##name##_jmp(ast, nodes[0], d); \
	if (jmp_op < 0) \
		return -1; \
	\
	scf_3ac_code_t* jmp = scf_3ac_jmp_code(jmp_op, NULL, NULL); \
	if (!jmp) \
		return -ENOMEM; \
	\
	scf_list_add_tail(d->_3ac_list_head, &jmp->list); \
	if (_scf_op_logic_##name##_jmp(ast, nodes[1], d) < 0) \
		return -1; \
	\
	jmp->dsts = scf_vector_alloc(); \
	if (!jmp->dsts) \
		return -ENOMEM; \
	scf_3ac_operand_t* dst = scf_3ac_operand_alloc(); \
	if (!dst) \
		return -ENOMEM; \
	if (scf_vector_add(jmp->dsts, dst) < 0) \
		return -ENOMEM; \
	\
	scf_list_t* l  = scf_list_tail(d->_3ac_list_head); \
	dst->code = scf_list_data(l, scf_3ac_code_t, list); \
	\
	int ret = scf_vector_add(d->branch_ops->_breaks, jmp); \
	if (ret < 0) \
		return ret; \
	return 0; \
}
SCF_OP_LOGIC(and)
SCF_OP_LOGIC(or)

scf_operator_handler_t _3ac_operator_handlers[] = {
	{SCF_OP_EXPR,           _scf_op_expr},
	{SCF_OP_CALL,           _scf_op_call},
	{SCF_OP_CREATE,         _scf_op_create},

	{SCF_OP_ARRAY_INDEX,    _scf_op_array_index},
	{SCF_OP_POINTER,        _scf_op_pointer},

	{SCF_OP_VA_START,       _scf_op_va_start},
	{SCF_OP_VA_ARG,         _scf_op_va_arg},
	{SCF_OP_VA_END,         _scf_op_va_end},

	{SCF_OP_TYPE_CAST,      _scf_op_type_cast},
	{SCF_OP_LOGIC_NOT,      _scf_op_logic_not},
	{SCF_OP_BIT_NOT,        _scf_op_bit_not},
	{SCF_OP_NEG,            _scf_op_neg},
	{SCF_OP_POSITIVE,       _scf_op_positive},

	{SCF_OP_INC,            _scf_op_inc},
	{SCF_OP_DEC,            _scf_op_dec},

	{SCF_OP_INC_POST,       _scf_op_inc_post},
	{SCF_OP_DEC_POST,       _scf_op_dec_post},

	{SCF_OP_DEREFERENCE,    _scf_op_dereference},
	{SCF_OP_ADDRESS_OF,     _scf_op_address_of},

	{SCF_OP_MUL,            _scf_op_mul},
	{SCF_OP_DIV,            _scf_op_div},
	{SCF_OP_MOD,            _scf_op_mod},

	{SCF_OP_ADD,            _scf_op_add},
	{SCF_OP_SUB,            _scf_op_sub},

	{SCF_OP_SHL,            _scf_op_shl},
	{SCF_OP_SHR,            _scf_op_shr},

	{SCF_OP_BIT_AND,        _scf_op_bit_and},
	{SCF_OP_BIT_OR,         _scf_op_bit_or},

	{SCF_OP_EQ,             _scf_op_eq},
	{SCF_OP_NE,             _scf_op_ne},
	{SCF_OP_GT,             _scf_op_gt},
	{SCF_OP_LT,             _scf_op_lt},
	{SCF_OP_GE,             _scf_op_ge},
	{SCF_OP_LE,             _scf_op_le},

	{SCF_OP_LOGIC_AND,      _scf_op_logic_and},
	{SCF_OP_LOGIC_OR,       _scf_op_logic_or},

	{SCF_OP_ASSIGN,         _scf_op_assign},
	{SCF_OP_ADD_ASSIGN,     _scf_op_add_assign},
	{SCF_OP_SUB_ASSIGN,     _scf_op_sub_assign},
	{SCF_OP_MUL_ASSIGN,     _scf_op_mul_assign},
	{SCF_OP_DIV_ASSIGN,     _scf_op_div_assign},
	{SCF_OP_MOD_ASSIGN,     _scf_op_mod_assign},
	{SCF_OP_SHL_ASSIGN,     _scf_op_shl_assign},
	{SCF_OP_SHR_ASSIGN,     _scf_op_shr_assign},
	{SCF_OP_AND_ASSIGN,     _scf_op_and_assign},
	{SCF_OP_OR_ASSIGN,      _scf_op_or_assign},


	{SCF_OP_BLOCK,          _scf_op_block},
	{SCF_OP_RETURN,         _scf_op_return},
	{SCF_OP_BREAK,          _scf_op_break},
	{SCF_OP_CONTINUE,       _scf_op_continue},
	{SCF_OP_GOTO,           _scf_op_goto},
	{SCF_LABEL,             _scf_op_label},

	{SCF_OP_IF,             _scf_op_if},
	{SCF_OP_WHILE,          _scf_op_while},
	{SCF_OP_DO,             _scf_op_do},
	{SCF_OP_FOR,            _scf_op_for},

	{SCF_OP_SWITCH,         _scf_op_switch},
	{SCF_OP_CASE,           _scf_op_case},
	{SCF_OP_DEFAULT,        _scf_op_default},
};

scf_operator_handler_t* scf_find_3ac_operator_handler(const int type)
{
	int i;
	for (i = 0; i < sizeof(_3ac_operator_handlers) / sizeof(_3ac_operator_handlers[0]); i++) {

		scf_operator_handler_t* h = &_3ac_operator_handlers[i];

		if (type == h->type)
			return h;
	}

	return NULL;
}
