#include"scf_optimizer.h"

static int _bb_prev_find(scf_basic_block_t* bb, void* data, scf_vector_t* queue)
{
	scf_basic_block_t* prev_bb;
	scf_dag_node_t*    dn;

	int count = 0;
	int ret;
	int j;
	int k;

	for (k = 0; k < bb->exit_dn_actives->size; k++) {
		dn =        bb->exit_dn_actives->data[k];

		if (scf_vector_find(bb->entry_dn_inactives, dn))
			continue;

		if (scf_vector_find(bb->entry_dn_actives, dn))
			continue;

		if (scf_vector_find(bb->entry_dn_delivery, dn))
			continue;

		ret = scf_vector_add(bb->entry_dn_delivery, dn);
		if (ret < 0)
			return ret;
		++count;
	}

	for (j = 0; j < bb->prevs->size; j++) {
		prev_bb   = bb->prevs->data[j];

		for (k = 0; k < bb->entry_dn_actives->size; k++) {
			dn =        bb->entry_dn_actives->data[k];

			if (scf_vector_find(prev_bb->exit_dn_actives, dn))
				continue;

			ret = scf_vector_add(prev_bb->exit_dn_actives, dn);
			if (ret < 0)
				return ret;
			++count;
		}

		for (k = 0; k < bb->entry_dn_delivery->size; k++) {
			dn =        bb->entry_dn_delivery->data[k];

			if (scf_vector_find(prev_bb->exit_dn_actives, dn))
				continue;

			ret = scf_vector_add(prev_bb->exit_dn_actives, dn);
			if (ret < 0)
				return ret;
			++count;
		}

		ret = scf_vector_add(queue, prev_bb);
		if (ret < 0)
			return ret;
	}
	return count;
}

static int _optimize_active_vars(scf_ast_t* ast, scf_function_t* f, scf_vector_t* functions)
{
	if (!f)
		return -EINVAL;

	scf_list_t*        bb_list_head = &f->basic_block_list_head;
	scf_list_t*        l;
	scf_basic_block_t* bb;

	int count;
	int ret;

	if (scf_list_empty(bb_list_head))
		return 0;

	for (l = scf_list_head(bb_list_head); l != scf_list_sentinel(bb_list_head); l = scf_list_next(l)) {

		bb  = scf_list_data(l, scf_basic_block_t, list);

		ret = scf_basic_block_active_vars(bb);
		if (ret < 0)
			return ret;
	}

	do {
		l   = scf_list_tail(bb_list_head);
		bb  = scf_list_data(l, scf_basic_block_t, list);
		assert(bb->end_flag);

		ret = scf_basic_block_search_bfs(bb, _bb_prev_find, NULL);
		if (ret < 0)
			return ret;
		count = ret;

	} while (count > 0);

//	scf_basic_block_print_list(bb_list_head);
	return 0;
}

scf_optimizer_t  scf_optimizer_active_vars =
{
	.name     =  "active_vars",

	.optimize =  _optimize_active_vars,

	.flags    = SCF_OPTIMIZER_LOCAL,
};

