#include"scf_parse.h"

int _find_function(scf_node_t* node, void* arg, scf_vector_t* vec)
{
	if (SCF_FUNCTION == node->type) {

		scf_function_t* f = (scf_function_t*)node;

		return scf_vector_add(vec, f);
	}

	return 0;
}

int _find_global_var(scf_node_t* node, void* arg, scf_vector_t* vec)
{
	if (SCF_OP_BLOCK == node->type
			|| (node->type >= SCF_STRUCT && node->class_flag)) {

		scf_block_t* b = (scf_block_t*)node;

		if (!b->scope || !b->scope->vars)
			return 0;

		int i;
		for (i = 0; i < b->scope->vars->size; i++) {

			scf_variable_t* v = b->scope->vars->data[i];

			if (v->global_flag || v->static_flag) {

				int ret = scf_vector_add(vec, v);
				if (ret < 0)
					return ret;
			}
		}
	}

	return 0;
}

