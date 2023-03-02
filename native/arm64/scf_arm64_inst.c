#include"scf_arm64.h"

#define ARM64_INST_OP3_CHECK() \
	if (!c->dsts || c->dsts->size != 1) \
		return -EINVAL; \
	\
	if (!c->srcs || c->srcs->size != 2) \
		return -EINVAL; \
	\
	scf_arm64_context_t* arm64 = ctx->priv; \
	scf_function_t*      f     = arm64->f; \
	\
	scf_3ac_operand_t*   dst   = c->dsts->data[0]; \
	scf_3ac_operand_t*   src0  = c->srcs->data[0]; \
	scf_3ac_operand_t*   src1  = c->srcs->data[1]; \
	\
	if (!src0 || !src0->dag_node) \
		return -EINVAL; \
	\
	if (!src1 || !src1->dag_node) \
		return -EINVAL; \
	\
	if (!dst || !dst->dag_node) \
		return -EINVAL; \
	\
	if (src0->dag_node->var->size != src1->dag_node->var->size) {\
		scf_loge("size: %d, %d\n", src0->dag_node->var->size, src1->dag_node->var->size); \
		return -EINVAL; \
	}


static int _arm64_inst_call_stack_size(scf_3ac_code_t* c)
{
	int stack_size = 0;

	int i;
	for (i = 1; i < c->srcs->size; i++) {
		scf_3ac_operand_t*  src = c->srcs->data[i];
		scf_variable_t*     v   = src->dag_node->var;

		if (src->dag_node->rabi2)
			continue;

		int size = arm64_variable_size(v);
		if (size & 0x7)
			size = (size + 7) >> 3 << 3;

		v->sp_offset  = stack_size;
		stack_size   += size;
	}
	assert(0 == (stack_size & 0x7));

	if (stack_size & 0xf)
		stack_size += 8;

	return stack_size;
}

static int _arm64_load_const_arg(scf_register_arm64_t* rabi, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_instruction_t*  inst;
	scf_arm64_OpCode_t*   lea;
	scf_arm64_OpCode_t*   mov;
	scf_variable_t*     v;

	v = dn->var;

	int size     = arm64_variable_size(v);
	int is_float = scf_variable_float(v);

	if (SCF_FUNCTION_PTR == v->type) {

		if (v->func_ptr) {
			assert(v->const_literal_flag);

			v->global_flag = 1;
			v->local_flag  = 0;
			v->tmp_flag    = 0;

			return arm64_make_inst_M2G(c, f, rabi, NULL, v);

		} else {
			uint32_t opcode;

			opcode = (0xcb << 24) | (rabi->id << 16) | (rabi->id << 5) | rabi->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);

			scf_loge("\n");
			return -EINVAL;
		}

	} else if (scf_variable_const_string(v)) {

		return arm64_make_inst_ISTR2G(c, f, rabi, v);

	} else if (v->nb_dimentions > 0) {
		assert(v->const_literal_flag);

		return arm64_make_inst_ADR2G(c, f, rabi, v);
	}

	return arm64_make_inst_I2G(c, rabi, v->data.u64, size);
}

static int _arm64_inst_call_argv(scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_arm64_t* sp  = arm64_find_register("sp");

	scf_arm64_OpCode_t*   lea;
	scf_arm64_OpCode_t*   mov;
	scf_arm64_OpCode_t*   movx;
	scf_instruction_t*    inst;

	uint32_t opcode;

	int nb_floats = 0;
	int ret;
	int i;
	for (i = c->srcs->size - 1; i >= 1; i--) {
		scf_3ac_operand_t*    src   = c->srcs->data[i];
		scf_variable_t*       v     = src->dag_node->var;
		scf_register_arm64_t* rd    = src->rabi;
		scf_register_arm64_t* rabi  = src->dag_node->rabi2;
		scf_register_arm64_t* rs    = NULL;

		int size     = arm64_variable_size(v);
		int is_float = scf_variable_float(v);

		if (!rabi) {
			rabi = arm64_find_register_type_id_bytes(is_float, SCF_ARM64_REG_X0,  size);

			ret  = arm64_overflow_reg(rabi, c, f);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}
		}

		movx = NULL;

		if (!is_float) {
			mov  = arm64_find_OpCode(SCF_ARM64_MOV, 8, 8, SCF_ARM64_G2E);

			if (size < 8) {
				if (scf_variable_signed(v))
					movx = arm64_find_OpCode(SCF_ARM64_MOVSX, size, 8, SCF_ARM64_E2G);
				else if (size < 4)
					movx = arm64_find_OpCode(SCF_ARM64_MOVZX, size, 8, SCF_ARM64_E2G);
			}

			if (0 == src->dag_node->color) {

				ret = arm64_overflow_reg(rabi, c, f);
				if (ret < 0) {
					scf_loge("\n");
					return ret;
				}

				ret = _arm64_load_const_arg(rabi, src->dag_node, c, f);
				if (ret < 0) {
					scf_loge("\n");
					return ret;
				}

				rabi = arm64_find_register_color_bytes(rabi->color, 8);
				rs   = rabi;
			} else {
				if (src->dag_node->color < 0)
					src->dag_node->color = rabi->color;
			}
		} else {
			nb_floats++;

			if (0 == src->dag_node->color) {
				src->dag_node->color = -1;
				v->global_flag       =  1;
			}

			if (SCF_VAR_FLOAT == v->type) {
				mov  = arm64_find_OpCode(SCF_ARM64_MOVSS,    4, 4, SCF_ARM64_G2E);
				movx = arm64_find_OpCode(SCF_ARM64_CVTSS2SD, 4, 8, SCF_ARM64_E2G);
			} else
				mov  = arm64_find_OpCode(SCF_ARM64_MOVSD, size, size, SCF_ARM64_G2E);

			if (src->dag_node->color < 0)
				src->dag_node->color = rabi->color;
		}

		if (!rs) {
			assert(src->dag_node->color > 0);

			if (rd && ARM64_COLOR_CONFLICT(rd->color, src->dag_node->color)) {

				ret = arm64_overflow_reg2(rd, src->dag_node, c, f);
				if (ret < 0) {
					scf_loge("\n");
					return ret;
				}
			}

			ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);
			rs = arm64_find_register_color_bytes(rs->color, 8);
		}

		if (movx) {
			if (SCF_ARM64_MOVSX == movx->type) {
				if (1 == size)
					opcode = (0x93 << 24) | (0x1 << 22) | (0x7 << 10) | (rs->id << 5) | rs->id;
				else if (2 == size)
					opcode = (0x93 << 24) | (0x1 << 22) | (0xf << 10) | (rs->id << 5) | rs->id;
				else if (4 == size)
					opcode = (0x93 << 24) | (0x1 << 22) | (0x1f << 10) | (rs->id << 5) | rs->id;
				else
					return -EINVAL;

			} else if (SCF_ARM64_MOVZX == movx->type) {

				if (1 == size)
					opcode = (0x53 << 24) | (0x7 << 10) | (rs->id << 5) | rs->id;
				else if (2 == size)
					opcode = (0x53 << 24) | (0xf << 10) | (rs->id << 5) | rs->id;
				else
					return -EINVAL;

			} else {
				assert(SCF_ARM64_CVTSS2SD == movx->type);

				opcode = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (0x3 << 14) | (rs->id << 5) | rs->id;
			}

			inst = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		if (!rd) {
			ret = arm64_make_inst_G2P(c, f, rs, sp, v->sp_offset, size);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}
			continue;
		}

		ret = arm64_overflow_reg2(rd, src->dag_node, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}

		if (!ARM64_COLOR_CONFLICT(rd->color, rs->color)) {
			rd     = arm64_find_register_color_bytes(rd->color, rs->bytes);

			if (is_float)
				opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;
			else
				opcode = (0x1e << 24) | (0x3 << 21) | (0x1 << 14) | (rs->id << 5) | rd->id;

			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		ret = arm64_rcg_make(c, c->rcg, NULL, rd);
		if (ret < 0)
			return ret;
	}

	return nb_floats;
}

static int _arm64_call_save_ret_regs(scf_3ac_code_t* c, scf_function_t* f, scf_function_t* pf)
{
	scf_register_arm64_t* r;
	scf_variable_t*     v;

	int i;
	for (i = 0; i < pf->rets->size; i++) {
		v  =        pf->rets->data[i];

		int is_float = scf_variable_float(v);

		if (is_float) {

			if (i > 0) {
				scf_loge("\n");
				return -1;
			}

			r = arm64_find_register_type_id_bytes(is_float, 0, 8);
		} else
			r = arm64_find_register_type_id_bytes(is_float, arm64_abi_ret_regs[i], 8);

		int ret = arm64_overflow_reg(r, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}
	return 0;
}

static int _arm64_dst_reg_valid(scf_register_arm64_t* rd, scf_register_arm64_t** updated_regs, int nb_updated, int abi_idx, int abi_total)
{
	scf_register_arm64_t* r;

	int i;
	for (i = 0; i < nb_updated; i++) {

		r  = updated_regs[i];

		if (ARM64_COLOR_CONFLICT(r->color, rd->color))
			return 0;
	}

	for (i = abi_idx; i < abi_total; i++) {

		r  = arm64_find_register_type_id_bytes(ARM64_COLOR_TYPE(rd->color), arm64_abi_ret_regs[i], rd->bytes);

		if (ARM64_COLOR_CONFLICT(r->color, rd->color))
			return 0;
	}

	return 1;
}

static int _arm64_call_update_dsts(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t** updated_regs, int max_updated)
{
	scf_3ac_operand_t*  dst;
	scf_dag_node_t*     dn;
	scf_variable_t*     v;

	scf_register_arm64_t* rd;
	scf_register_arm64_t* rs;
	scf_arm64_OpCode_t*   mov;

	int nb_float   = 0;
	int nb_int     = 0;

	int i;
	for (i  = 0; i < c->dsts->size; i++) {
		dst =        c->dsts->data[i];
		dn  =        dst->dag_node;
		v   =        dn->var;

		if (SCF_VAR_VOID == v->type && 0 == v->nb_pointers)
			continue;

		assert(0 != dn->color);

		int is_float = scf_variable_float(v);

		if (is_float)
			nb_float++;
		else
			nb_int++;
	}

	int nb_updated = 0;
	int idx_float  = 0;
	int idx_int    = 0;

	for (i  = 0; i < c->dsts->size; i++) {
		dst =        c->dsts->data[i];
		dn  =        dst->dag_node;
		v   =        dn->var;

		if (SCF_VAR_VOID == v->type && 0 == v->nb_pointers)
			continue;

		int is_float = scf_variable_float(v);
		int dst_size = arm64_variable_size (v);

		if (is_float) {
			if (i > 0) {
				scf_loge("\n");
				return -1;
			}

			scf_loge("\n");
			return -EINVAL;

			rs = arm64_find_register_type_id_bytes(is_float, SCF_ARM64_REG_X0, dst_size);

			if (SCF_VAR_FLOAT == dn->var->type)
				mov = arm64_find_OpCode(SCF_ARM64_MOVSS, dst_size, dst_size, SCF_ARM64_G2E);
			else
				mov = arm64_find_OpCode(SCF_ARM64_MOVSD, dst_size, dst_size, SCF_ARM64_G2E);

			idx_float++;
		} else {
			rs  = arm64_find_register_type_id_bytes(is_float, arm64_abi_ret_regs[idx_int], dst_size);

			mov = arm64_find_OpCode(SCF_ARM64_MOV, dst_size, dst_size, SCF_ARM64_G2E);

			idx_int++;
		}

		scf_instruction_t*  inst;

		if (dn->color > 0) {
			rd = arm64_find_register_color(dn->color);

			int rd_vars = arm64_reg_cached_vars(rd);

			if (rd_vars > 1) {
				dn->color  = -1;
				dn->loaded = 0;
				scf_vector_del(rd->dag_nodes, dn);

			} else if (rd_vars > 0
					&& !scf_vector_find(rd->dag_nodes, dn)) {
				dn->color  = -1;
				dn->loaded = 0;

			} else {
				ARM64_SELECT_REG_CHECK(&rd, dn, c, f, 0);

				if (dn->color == rs->color) {
					assert(nb_updated < max_updated);

					updated_regs[nb_updated++] = rs;
					continue;
				}

				int valid = _arm64_dst_reg_valid(rd, updated_regs, nb_updated, idx_int, nb_int);
				if (valid) {

					uint32_t opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;

					inst = arm64_make_inst(c, opcode);
					ARM64_INST_ADD_CHECK(c->instructions, inst);

					assert(nb_updated < max_updated);

					updated_regs[nb_updated++] = rd;
					continue;
				}

				assert(0  == scf_vector_del(rd->dag_nodes, dn));
				dn->color  = -1;
				dn->loaded =  0;
			}
		}

		int rs_vars = arm64_reg_cached_vars(rs);

		if (0 == rs_vars) {
			if (scf_vector_add(rs->dag_nodes, dn) < 0)
				return -ENOMEM;

			assert(nb_updated < max_updated);

			updated_regs[nb_updated++] = rs;

			dn->color  = rs->color;
			dn->loaded = 1;

		} else {
			int ret = arm64_make_inst_G2M(c, f, rs, NULL, dn->var);
			if (ret < 0)
				return ret;
		}
	}
	return nb_updated;
}

static int _arm64_inst_call_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (c->srcs->size < 1) {
		scf_loge("\n");
		return -EINVAL;
	}

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t* src0    = c->srcs->data[0];
	scf_variable_t*    var_pf  = src0->dag_node->var;
	scf_function_t*    pf      = var_pf->func_ptr;

	if (SCF_FUNCTION_PTR != var_pf->type || !pf) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* lr  = arm64_find_register("lr");
	scf_register_arm64_t* sp  = arm64_find_register("sp");
	scf_register_arm64_t* x0  = arm64_find_register("x0");
	scf_instruction_t*    inst;
	scf_instruction_t*    inst_sp  = NULL;
	scf_instruction_t*    inst_sp2 = NULL;

	lr->used = 1;
	sp->used = 1;

	int data_rela_size = f->data_relas->size;
	int text_rela_size = f->text_relas->size;
	scf_loge("f->data_relas->size: %d, f->text_relas->size: %d\n", f->data_relas->size, f->text_relas->size);

	uint32_t opcode;
	int ret;
	int i;

	if (pf->rets) {
		ret = _arm64_call_save_ret_regs(c, f, pf);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	ret = arm64_overflow_reg(x0, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	arm64_call_rabi(NULL, NULL, c);

	int32_t stack_size = _arm64_inst_call_stack_size(c);
	if (stack_size > 0) {
		inst_sp  = arm64_make_inst(c, 0);
		inst_sp2 = arm64_make_inst(c, 0);
		ARM64_INST_ADD_CHECK(c->instructions, inst_sp);
		ARM64_INST_ADD_CHECK(c->instructions, inst_sp2);
	}

	ret = _arm64_inst_call_argv(c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	scf_register_arm64_t* saved_regs[ARM64_ABI_CALLER_SAVES_NB];

	int save_size = arm64_caller_save_regs(c, f, arm64_abi_caller_saves, ARM64_ABI_CALLER_SAVES_NB, stack_size, saved_regs);
	if (save_size < 0) {
		scf_loge("\n");
		return save_size;
	}

	if (stack_size > 0) {
		assert(inst_sp);
		assert(inst_sp2);

		if (stack_size <= 0xfff)
			opcode = (0xd1 << 24) | (stack_size << 10) | (sp->id << 5) | sp->id;
		else {
			stack_size += 1 << 12;
			stack_size &= ~0xfff;

			if (stack_size < 0 || (stack_size >> 12) > 0xfff) {
				scf_loge("\n");
				return -EINVAL;
			}

			opcode = (0xd1 << 24) | (0x1 << 22) | ((stack_size >> 12) << 10) | (sp->id << 5) | sp->id;
		}

		memcpy(inst_sp->code, &opcode, 4);

		if (save_size > 0xfff) {
			scf_loge("\n");
			return -EINVAL;
		}

		if (save_size > 0) {
			opcode = (0xd1 << 24) | (save_size << 10) | (sp->id << 5) | sp->id;
			memcpy(inst_sp2->code, &opcode, 4);
		}
	}

	if (var_pf->const_literal_flag) {
		assert(0 == src0->dag_node->color);

		opcode = 0x25 << 26;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		scf_rela_t* rela = NULL;

		ARM64_RELA_ADD_CHECK(f->text_relas, rela, c, NULL, pf);
		rela->type = R_AARCH64_CALL26;

	} else {
		assert(0 != src0->dag_node->color);

		if (src0->dag_node->color > 0) {

			scf_register_arm64_t* r_pf = NULL;

			ret = arm64_select_reg(&r_pf, src0->dag_node, c, f, 1);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}

			opcode = (0xd63f << 16) | (r_pf->id << 5);
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);

		} else {
			scf_loge("\n");
			return -EINVAL;
		}
	}

	if (stack_size > 0) {

		if (stack_size <= 0xfff)
			opcode = (0x91 << 24) | (stack_size << 10) | (sp->id << 5) | sp->id;
		else
			opcode = (0x91 << 24) | (0x1 << 22) | ((stack_size >> 12) << 10) | (sp->id << 5) | sp->id;

		inst = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
	}

	int nb_updated = 0;
	scf_register_arm64_t* updated_regs[ARM64_ABI_RET_NB * 2];

	if (pf->rets && pf->rets->size > 0 && c->dsts) {

		nb_updated = _arm64_call_update_dsts(c, f, updated_regs, ARM64_ABI_RET_NB * 2);
		if (nb_updated < 0) {
			scf_loge("\n");
			return nb_updated;
		}
	}

	if (save_size > 0) {
		ret = arm64_pop_regs(c, saved_regs, save_size >> 3, updated_regs, nb_updated);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	return 0;
}

static int _arm64_inst_bit_not_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rd   = NULL;
	scf_register_arm64_t* rs   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       s    = src->dag_node;
	scf_dag_node_t*       d    = dst->dag_node;

	uint32_t opcode;

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

	if (0 == s->color) {
		uint64_t u = s->var->data.u64;

		int ret = arm64_make_inst_I2G(c, rd, u, 8);
		if (ret < 0)
			return ret;

		opcode = (0xaa << 24) | (0x1 << 21) | (rd->id << 16) | (0x1f << 10) | rd->id;

	} else {
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode = (0xaa << 24) | (0x1 << 21) | (rs->id << 16) | (0x1f << 10) | rd->id;
	}

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_inc_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rs    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    src   = c->srcs->data[0];
	scf_instruction_t*    inst  = NULL;
	scf_function_t*       f     = arm64->f;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (0 == src->dag_node->color) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	uint32_t opcode = (0x91 << 24) | (0x1 << 10) | (rs->id << 5) | rs->id;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

static int _arm64_inst_inc_post_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rd    = NULL;
	scf_register_arm64_t* rs    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    src   = c->srcs->data[0];
	scf_3ac_operand_t*    dst   = c->dsts->data[0];
	scf_instruction_t*    inst  = NULL;
	scf_function_t*       f     = arm64->f;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (0 == src->dag_node->color) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);
	ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	uint32_t opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	opcode = (0x91 << 24) | (0x1 << 10) | (rs->id << 5) | rs->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_dec_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rs    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    src   = c->srcs->data[0];
	scf_instruction_t*    inst  = NULL;
	scf_function_t*       f     = arm64->f;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (0 == src->dag_node->color) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	uint32_t opcode = (0xd1 << 24) | (0x1 << 10) | (rs->id << 5) | rs->id;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

static int _arm64_inst_dec_post_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rd    = NULL;
	scf_register_arm64_t* rs    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    src   = c->srcs->data[0];
	scf_3ac_operand_t*    dst   = c->dsts->data[0];
	scf_instruction_t*    inst  = NULL;
	scf_function_t*       f     = arm64->f;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (0 == src->dag_node->color) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);
	ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	uint32_t opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	opcode = (0xd1 << 24) | (0x1 << 10) | (rs->id << 5) | rs->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_neg_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	assert(0 != dst->dag_node->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rd   = NULL;
	scf_register_arm64_t* rs   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       s    = src->dag_node;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_variable_t*       v    = d->var;

	uint32_t opcode;

	int is_float = scf_variable_float(v);
	int size     = arm64_variable_size(v);

	if (!is_float) {

		ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

		if (0 == s->color) {
			uint64_t u = s->var->data.u64;

			int ret = arm64_make_inst_I2G(c, rd, u, 8);
			if (ret < 0)
				return ret;

			opcode = (0xcb << 24) | (rd->id << 16) | (0x1f << 10) | rd->id;

		} else {
			ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

			opcode = (0xcb << 24) | (rs->id << 16) | (0x1f << 10) | rd->id;
		}

		inst = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

#if 0
	scf_arm64_OpCode_t*   pxor = arm64_find_OpCode(SCF_ARM64_PXOR,  8, 8, SCF_ARM64_E2G);
	scf_arm64_OpCode_t*   sub  = arm64_find_OpCode(SCF_ARM64_SUBSS, 4, 4, SCF_ARM64_E2G);

	if (v->size > 4)
		sub = arm64_find_OpCode(SCF_ARM64_SUBSD, 8, 8, SCF_ARM64_E2G);

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);
	inst = arm64_make_inst_E2G(pxor, rd, rd);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (src->dag_node->color > 0) {
		ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

		inst = arm64_make_inst_E2G(sub, rd, rs);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

	} else {
		scf_rela_t* rela = NULL;

		v = src->dag_node->var;

		if (0 == src->dag_node->color) {
			v->global_flag = 1;
			v->local_flag  = 0;
			v->tmp_flag    = 0;
		}

		inst = arm64_make_inst_M2G(&rela, sub, rd, NULL, v);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	}
#endif
	return -1;
}

static int _arm64_inst_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  member = c->srcs->data[1];
	scf_instruction_t*  inst;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!member || !member->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*     vd  = dst   ->dag_node->var;
	scf_variable_t*     vb  = base  ->dag_node->var;
	scf_variable_t*     vm  = member->dag_node->var;

	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	int ret = arm64_select_reg(&rd, dst->dag_node, c, f, 0);
	if (ret < 0)
		return ret;

	ret = arm64_pointer_reg(&sib, base->dag_node, member->dag_node, c, f);
	if (ret < 0)
		return ret;

	if (scf_variable_is_struct(vm) || scf_variable_is_array(vm))

		return arm64_make_inst_ADRP2G(c, f, rd, sib.base, sib.disp);

	if (sib.index)
		return arm64_make_inst_SIB2G(c, f, rd, &sib);

	return arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
}

static int _arm64_inst_binary_assign_pointer(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t op)
{
	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  member = c->srcs->data[1];
	scf_3ac_operand_t*  src    = c->srcs->data[2];
	scf_instruction_t*  inst;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!member || !member->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*     vs  = src   ->dag_node->var;
	scf_variable_t*     vb  = base  ->dag_node->var;
	scf_variable_t*     vm  = member->dag_node->var;

	scf_register_arm64_t* rd  = NULL;
	scf_register_arm64_t* rs  = NULL;
	arm64_sib_t           sib = {0};

	int ret = arm64_pointer_reg(&sib, base->dag_node, member->dag_node, c, f);
	if (ret < 0)
		return ret;

	if (scf_variable_is_struct(vm) || scf_variable_is_array(vm))
		return -EINVAL;

	int is_float = scf_variable_float(vs);

	if (0 == src->dag_node->color) {

		if (is_float) {

			src->dag_node->color = -1;
			vs->global_flag      =  1;

			ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

		} else {
			ret = arm64_select_free_reg(&rs, c, f, 0);
			if (ret < 0)
				return ret;

			ret = arm64_make_inst_I2G(c, rs, vs->data.u64, sib.size);
			if (ret < 0)
				return ret;

			rs = arm64_find_register_color_bytes(rs->color, sib.size);
		}
	} else
		ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	if (op) {
		uint32_t opcode;

		ret = arm64_select_free_reg(&rd, c, f, is_float);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
		if (ret < 0)
			return ret;

		opcode = ((8 == sib.size) << 31)| op | (rs->id << 16) | (rd->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		rs = rd;
	}

	return arm64_make_inst_G2P(c, f, rs, sib.base, sib.disp, sib.size);
}

static int _arm64_inst_inc_dec_pointer(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  member = c->srcs->data[1];
	scf_instruction_t*  inst;

	uint32_t opcode;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!member || !member->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vb  = base  ->dag_node->var;
	scf_variable_t*       vm  = member->dag_node->var;
	scf_register_arm64_t* r   = NULL;
	arm64_sib_t           sib = {0};

	int size = arm64_variable_size(vm);

	int ret = arm64_pointer_reg(&sib, base->dag_node, member->dag_node, c, f);
	if (ret < 0)
		return ret;

	if (scf_variable_is_struct(vm) || scf_variable_is_array(vm))
		return -EINVAL;

	int is_float = scf_variable_float(vm);
	if (is_float) {
		scf_loge("\n");
		return -EINVAL;
	}

	ret = arm64_select_free_reg(&r, c, f, 0);
	if (ret < 0)
		return ret;

	if (sib.index)
		ret = arm64_make_inst_SIB2G(c, f, r, &sib);
	else
		ret = arm64_make_inst_P2G(c, f, r, sib.base, sib.disp, sib.size);
	if (ret < 0)
		return ret;

	opcode = ((8 == size) << 31) | (u24 << 24) | (0x1 << 10) | (r->id << 5) | r->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (sib.index)
		ret = arm64_make_inst_G2SIB(c, f, r, &sib);
	else
		ret = arm64_make_inst_G2P(c, f, r, sib.base, sib.disp, sib.size);
	return ret;
}

static int _arm64_inst_inc_dec_post_pointer(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  member = c->srcs->data[1];
	scf_instruction_t*  inst;

	uint32_t opcode;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!member || !member->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vd  = dst   ->dag_node->var;
	scf_variable_t*       vb  = base  ->dag_node->var;
	scf_variable_t*       vm  = member->dag_node->var;
	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	int size = arm64_variable_size(vm);

	int ret = arm64_pointer_reg(&sib, base->dag_node, member->dag_node, c, f);
	if (ret < 0)
		return ret;

	if (scf_variable_is_struct(vm) || scf_variable_is_array(vm))
		return -EINVAL;

	int is_float = scf_variable_float(vm);
	if (is_float) {
		scf_loge("\n");
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	if (sib.index)
		ret = arm64_make_inst_SIB2G(c, f, rd, &sib);
	else
		ret = arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
	if (ret < 0)
		return ret;

	opcode = ((8 == size) << 31) | (u24 << 24) | (0x1 << 10) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (sib.index)
		ret = arm64_make_inst_G2SIB(c, f, rd, &sib);
	else
		ret = arm64_make_inst_G2P(c, f, rd, sib.base, sib.disp, sib.size);
	if (ret < 0)
		return ret;

	opcode &= ~(1 << 30);
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_inc_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_pointer(ctx, c, 0x11);
}

static int _arm64_inst_dec_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_pointer(ctx, c, 0x51);
}

static int _arm64_inst_inc_post_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_post_pointer(ctx, c, 0x11);
}

static int _arm64_inst_dec_post_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_post_pointer(ctx, c, 0x51);
}

static int _arm64_inst_address_of_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  member = c->srcs->data[1];
	scf_instruction_t*  inst;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!member || !member->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	scf_variable_t*       vd  = dst   ->dag_node->var;
	scf_variable_t*       vb  = base  ->dag_node->var;
	scf_variable_t*       vm  = member->dag_node->var;

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	int ret = arm64_pointer_reg(&sib, base->dag_node, member->dag_node, c, f);
	if (ret < 0)
		return ret;

	return arm64_make_inst_ADRP2G(c, f, rd, sib.base, sib.disp);
}

static int _arm64_inst_binary_assign_array_index(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t op)
{
	if (!c->srcs || c->srcs->size != 4)
		return -EINVAL;

	scf_arm64_context_t*  arm64    = ctx->priv;
	scf_function_t*     f      = arm64->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];
	scf_3ac_operand_t*  src    = c->srcs->data[3];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vscale = scale->dag_node->var;
	scf_variable_t*       vb     = base->dag_node->var;
	scf_variable_t*       vs     = src ->dag_node->var;

	scf_register_arm64_t* rd     = NULL;
	scf_register_arm64_t* rs     = NULL;
	arm64_sib_t           sib    = {0};
	scf_instruction_t*    inst;

	int is_float = scf_variable_float(vs);
	int size     = arm64_variable_size (vs);

	if (size > vscale->data.i)
		size = vscale->data.i;

	int ret  = arm64_array_index_reg(&sib, base->dag_node, index->dag_node, scale->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	if (0 == src->dag_node->color) {

		if (is_float) {

			src->dag_node->color = -1;
			vs->global_flag      =  1;

			ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

		} else {
			ret = arm64_select_free_reg(&rs, c, f, 0);
			if (ret < 0)
				return ret;

			ret = arm64_make_inst_I2G(c, rs, vs->data.u64, size);
			if (ret < 0)
				return ret;

			rs = arm64_find_register_color_bytes(rs->color, size);
		}
	} else
		ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	if (op) {
		uint32_t opcode;

		ret = arm64_select_free_reg(&rd, c, f, is_float);
		if (ret < 0)
			return ret;

		if (sib.index)
			ret = arm64_make_inst_SIB2G(c, f, rd, &sib);
		else
			ret = arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
		if (ret < 0)
			return ret;

		opcode = ((8 == size) << 31) | op | (rs->id << 16) | (rd->id << 5) | rd->id | (is_float << 26);
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		rs = rd;
	}

	if (sib.index)
		ret = arm64_make_inst_G2SIB(c, f, rs, &sib);
	else
		ret = arm64_make_inst_G2P(c, f, rs, sib.base, sib.disp, size);
	if (ret < 0)
		return ret;

	return 0;
}

static int _arm64_inst_inc_dec_array_index(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t*  arm64    = ctx->priv;
	scf_function_t*     f      = arm64->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];

	uint32_t opcode;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vscale = scale->dag_node->var;
	scf_variable_t*       vb     = base->dag_node->var;

	scf_register_arm64_t* r      = NULL;
	arm64_sib_t           sib    = {0};
	scf_instruction_t*    inst;

	int size = vb->data_size;

	int ret  = arm64_array_index_reg(&sib, base->dag_node, index->dag_node, scale->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ret = arm64_select_free_reg(&r, c, f, 0);
	if (ret < 0)
		return ret;

	r = arm64_find_register_color_bytes(r->color, size);

	if (sib.index)
		ret = arm64_make_inst_SIB2G(c, f, r, &sib);
	else
		ret = arm64_make_inst_P2G(c, f, r, sib.base, sib.disp, size);
	if (ret < 0)
		return ret;

	opcode = ((8 == size) << 31) | (u24 << 24) | (0x1 << 10) | (r->id << 5) | r->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (sib.index)
		ret = arm64_make_inst_G2SIB(c, f, r, &sib);
	else
		ret = arm64_make_inst_G2P(c, f, r, sib.base, sib.disp, size);
	return ret;
}

static int _arm64_inst_inc_dec_post_array_index(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];

	uint32_t opcode;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vscale = scale->dag_node->var;
	scf_variable_t*       vb     = base ->dag_node->var;
	scf_variable_t*       vd     = dst  ->dag_node->var;

	scf_register_arm64_t* rd     = NULL;
	arm64_sib_t           sib    = {0};
	scf_instruction_t*    inst;

	int size = vb->data_size;

	int ret  = arm64_array_index_reg(&sib, base->dag_node, index->dag_node, scale->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	if (sib.index)
		ret = arm64_make_inst_SIB2G(c, f, rd, &sib);
	else
		ret = arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, size);
	if (ret < 0)
		return ret;

	opcode = ((8 == size) << 31) | (u24 << 24) | (0x1 << 10) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (sib.index)
		ret = arm64_make_inst_G2SIB(c, f, rd, &sib);
	else
		ret = arm64_make_inst_G2P(c, f, rd, sib.base, sib.disp, size);
	if (ret < 0)
		return ret;

	opcode &= ~(0x1 << 30);
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_inc_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_array_index(ctx, c, 0x11);
}

static int _arm64_inst_dec_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_array_index(ctx, c, 0x51);
}

static int _arm64_inst_inc_post_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_post_array_index(ctx, c, 0x11);
}

static int _arm64_inst_dec_post_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_inc_dec_post_array_index(ctx, c, 0x51);
}

static int _arm64_inst_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t*  arm64    = ctx->priv;
	scf_function_t*     f      = arm64->f;

	scf_3ac_operand_t*  dst    = c->dsts->data[0];
	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  index  = c->srcs->data[1];
	scf_3ac_operand_t*  scale  = c->srcs->data[2];

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*     vd  = dst  ->dag_node->var;
	scf_variable_t*     vb  = base ->dag_node->var;
	scf_variable_t*     vi  = index->dag_node->var;
	scf_variable_t*     vs  = scale->dag_node->var;

	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	scf_instruction_t*  inst;

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	int ret = arm64_array_index_reg(&sib, base->dag_node, index->dag_node, scale->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	if (vb->nb_dimentions > 1) {

		if (sib.index)
			ret = arm64_make_inst_ADRSIB2G(c, f, rd, &sib);
		else
			ret = arm64_make_inst_ADRP2G(c, f, rd, sib.base, sib.disp);
		return ret;
	}

	if (sib.index)
		return arm64_make_inst_SIB2G(c, f, rd, &sib);

	return arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
}

static int _arm64_inst_address_of_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   base  = c->srcs->data[0];
	scf_3ac_operand_t*   index = c->srcs->data[1];
	scf_3ac_operand_t*   scale = c->srcs->data[2];
	scf_instruction_t*   inst;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!index || !index->dag_node)
		return -EINVAL;

	if (!scale || !scale->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vd  = dst  ->dag_node->var;
	scf_variable_t*       vb  = base ->dag_node->var;
	scf_variable_t*       vi  = index->dag_node->var;
	scf_variable_t*       vs  = scale->dag_node->var;

	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	int ret = arm64_array_index_reg(&sib, base->dag_node, index->dag_node, scale->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	if (sib.index)
		ret = arm64_make_inst_ADRSIB2G(c, f, rd, &sib);
	else
		ret = arm64_make_inst_ADRP2G(c, f, rd, sib.base, sib.disp);

	return ret;
}

static int _arm64_inst_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rd    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    base  = c->srcs->data[0];
	scf_3ac_operand_t*    dst   = c->dsts->data[0];
	scf_instruction_t*    inst;
	scf_function_t*       f     = arm64->f;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	arm64_sib_t sib = {0};

	int size = base->dag_node->var->data_size;

	int ret  = arm64_dereference_reg(&sib, base->dag_node, NULL, c, f);
	if (ret < 0)
		return ret;

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	return arm64_make_inst_P2G(c, f, rd, sib.base, 0, size);
}

static int _arm64_inst_push_rax_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rax  = arm64_find_register("rax");
	scf_arm64_OpCode_t*   push;
	scf_instruction_t*  inst;

#if 0
	push = arm64_find_OpCode(SCF_ARM64_PUSH, 8,8, SCF_ARM64_G);
	inst = arm64_make_inst_G(push, rax);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
#endif
	return -1;
}

static int _arm64_inst_pop_rax_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rax  = arm64_find_register("rax");
	scf_arm64_OpCode_t*   pop;
	scf_instruction_t*  inst;
#if 0
	pop  = arm64_find_OpCode(SCF_ARM64_POP, 8,8, SCF_ARM64_G);
	inst = arm64_make_inst_G(pop, rax);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
#endif
	return -1;
}

/*

struct va_list
{
	uint8_t*  iptr;
	uint8_t*  fptr;
	uint8_t*  optr;

	intptr_t  ireg;
	intptr_t  freg;
};
*/

static int _arm64_inst_va_start_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_loge("c->srcs->size: %d\n", c->srcs->size);
	assert(3 == c->srcs->size);

	scf_register_arm64_t* rbp   = arm64_find_register("rbp");
	scf_register_arm64_t* rptr  = NULL;
	scf_register_arm64_t* rap   = NULL;
	scf_instruction_t*  inst  = NULL;
	scf_3ac_operand_t*  ap    = c->srcs->data[0];
	scf_3ac_operand_t*  ptr   = c->srcs->data[2];
	scf_arm64_OpCode_t*   mov   = arm64_find_OpCode(SCF_ARM64_MOV, 8, 8, SCF_ARM64_G2E);
	scf_arm64_OpCode_t*   lea   = arm64_find_OpCode(SCF_ARM64_LEA, 8, 8, SCF_ARM64_E2G);
	scf_variable_t*     v     = ap->dag_node->var;

	int offset_int            = -f->args_int   * 8 - 8;
	int offset_float          = -f->args_float * 8 - ARM64_ABI_NB * 8 - 8;
	int offset_others         = 16;

	if (v->bp_offset >= 0) {
		scf_loge("\n");
		return -1;
	}
#if 0
	ARM64_SELECT_REG_CHECK(&rap,  ap ->dag_node, c, f, 1);
	ARM64_SELECT_REG_CHECK(&rptr, ptr->dag_node, c, f, 0);

	inst = arm64_make_inst_P2G(lea, rptr, rbp, offset_int);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap,  0, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);


	inst = arm64_make_inst_P2G(lea, rptr, rbp, offset_float);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap,  8, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);


	inst = arm64_make_inst_P2G(lea, rptr, rbp, offset_others);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap,  16, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);


	mov  = arm64_find_OpCode(SCF_ARM64_MOV, 4, 8, SCF_ARM64_I2E);

	inst = arm64_make_inst_I2P(mov, rap,  24, (uint8_t*)&f->args_int, 4);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_I2P(mov, rap,  32, (uint8_t*)&f->args_float, 4);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
#endif
	return -1;
}

static int _arm64_inst_va_end_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	assert(2 == c->srcs->size);

	scf_register_arm64_t* rbp  = arm64_find_register("rbp");
	scf_register_arm64_t* rptr = NULL;
	scf_register_arm64_t* rap  = NULL;
	scf_instruction_t*  inst = NULL;
	scf_3ac_operand_t*  ap   = c->srcs->data[0];
	scf_3ac_operand_t*  ptr  = c->srcs->data[1];
	scf_arm64_OpCode_t*   mov  = arm64_find_OpCode(SCF_ARM64_MOV, 8, 8, SCF_ARM64_G2E);
	scf_arm64_OpCode_t*   xor  = arm64_find_OpCode(SCF_ARM64_XOR, 8, 8, SCF_ARM64_G2E);
	scf_variable_t*     v    = ap->dag_node->var;

	if (v->bp_offset >= 0) {
		scf_loge("\n");
		return -1;
	}

	ptr->dag_node->var->tmp_flag =  1;
	ptr->dag_node->color         = -1;
#if 0
	ARM64_SELECT_REG_CHECK(&rap,  ap ->dag_node, c, f, 1);
	ARM64_SELECT_REG_CHECK(&rptr, ptr->dag_node, c, f, 0);

	inst = arm64_make_inst_G2E(xor, rptr, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap, 0, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap, 8, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap, 16, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap, 24, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst_G2P(mov, rap, 32, rptr);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	ptr->dag_node->var->tmp_flag = 0;
	ptr->dag_node->color         = 0;
	ptr->dag_node->loaded        = 0;

	assert(0 == scf_vector_del(rptr->dag_nodes, ptr->dag_node));
	return 0;
#endif
	return -1;
}

static int _arm64_inst_va_arg_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	assert(1 == c->dsts->size && 3 == c->srcs->size);

	scf_register_arm64_t* rbp  = arm64_find_register("rbp");
	scf_register_arm64_t* rd   = NULL; // result
	scf_register_arm64_t* rap  = NULL; // ap
	scf_register_arm64_t* rptr = NULL; // ptr
	scf_instruction_t*  inst = NULL;

	scf_instruction_t*  inst_jge = NULL;
	scf_instruction_t*  inst_jmp = NULL;

	scf_3ac_operand_t*  dst  = c->dsts->data[0];
	scf_3ac_operand_t*  ap   = c->srcs->data[0];
	scf_3ac_operand_t*  src  = c->srcs->data[1];
	scf_3ac_operand_t*  ptr  = c->srcs->data[2];
	scf_variable_t*     v    = src->dag_node->var;

#if 0
	scf_arm64_OpCode_t*   inc  = arm64_find_OpCode(SCF_ARM64_INC, 8, 8, SCF_ARM64_E);
	scf_arm64_OpCode_t*   add  = arm64_find_OpCode(SCF_ARM64_ADD, 4, 8, SCF_ARM64_I2E);
	scf_arm64_OpCode_t*   sub  = arm64_find_OpCode(SCF_ARM64_SUB, 4, 8, SCF_ARM64_I2E);
	scf_arm64_OpCode_t*   cmp  = arm64_find_OpCode(SCF_ARM64_CMP, 4, 8, SCF_ARM64_I2E);
	scf_arm64_OpCode_t*   mov  = arm64_find_OpCode(SCF_ARM64_MOV, 8, 8, SCF_ARM64_E2G);
	scf_arm64_OpCode_t*   jge  = arm64_find_OpCode(SCF_ARM64_JGE, 4, 4, SCF_ARM64_I);
	scf_arm64_OpCode_t*   jmp  = arm64_find_OpCode(SCF_ARM64_JMP, 4, 4, SCF_ARM64_I);
	scf_arm64_OpCode_t*   mov2 = NULL;

	ARM64_SELECT_REG_CHECK(&rd,   dst->dag_node, c, f, 0);
	ARM64_SELECT_REG_CHECK(&rap,  ap ->dag_node, c, f, 1);
	ARM64_SELECT_REG_CHECK(&rptr, ptr->dag_node, c, f, 0);

	int is_float = scf_variable_float(v);
	int size     = arm64_variable_size(v);

	uint32_t nints   = ARM64_ABI_NB;
	uint32_t nfloats = ARM64_ABI_NB;
	uint32_t offset  = 0;
	uint32_t incptr  = 8;

	int idx_offset   = 24;
	int ptr_offset   = 0;

	if (is_float) {
		idx_offset   = 32;
		ptr_offset   = 8;
	}

	inst = arm64_make_inst_I2P(cmp, rap, idx_offset, (uint8_t*)&nints, 4);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst_jge = arm64_make_inst_I(jge, (uint8_t*)&offset, sizeof(offset));
	ARM64_INST_ADD_CHECK(c->instructions, inst_jge);


	inst = arm64_make_inst_P2G(mov, rptr, rap, ptr_offset);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	offset += inst->len;

	inst = arm64_make_inst_I2P(sub, rap, ptr_offset, (uint8_t*)&incptr, 4);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	offset += inst->len;

	inst_jmp = arm64_make_inst_I(jmp, (uint8_t*)&offset, sizeof(offset));
	ARM64_INST_ADD_CHECK(c->instructions, inst_jmp);
	offset += inst_jmp->len;

	uint8_t* p = (uint8_t*)&offset;
	int i;
	for (i = 0; i < 4; i++)
		inst_jge->code[jge->nb_OpCodes + i] = p[i];

	offset = 0;
	inst = arm64_make_inst_P2G(mov, rptr, rap, 16);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	offset += inst->len;

	inst = arm64_make_inst_I2P(add, rap, 16, (uint8_t*)&incptr, 4);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	offset += inst->len;

	for (i = 0; i < 4; i++)
		inst_jmp->code[jmp->nb_OpCodes + i] = p[i];

	inst = arm64_make_inst_P(inc, rap, idx_offset, 8);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if (is_float) {
		if (4 == size)
			mov2 = arm64_find_OpCode(SCF_ARM64_MOVSS, 4, 4, SCF_ARM64_E2G);
		else if (8 == size)
			mov2 = arm64_find_OpCode(SCF_ARM64_MOVSD, 8, 8, SCF_ARM64_E2G);
		else
			assert(0);
	} else
		mov2 = arm64_find_OpCode(SCF_ARM64_MOV, size, size, SCF_ARM64_E2G);

	inst = arm64_make_inst_P2G(mov2, rd, rptr, 0);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
#endif
	return -1;
}

static int _arm64_inst_address_of_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1) {
		scf_loge("\n");
		return -EINVAL;
	}

	if (!c->srcs || c->srcs->size != 1) {
		scf_loge("\n");
		return -EINVAL;
	}

	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*       f     = arm64->f;

	scf_3ac_operand_t*    dst   = c->dsts->data[0];
	scf_3ac_operand_t*    src   = c->srcs->data[0];
	scf_register_arm64_t* rd    = NULL;
	scf_instruction_t*    inst;

	if (!src || !src->dag_node) {
		scf_loge("\n");
		return -EINVAL;
	}
	assert(dst->dag_node->var->nb_pointers > 0);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	int ret = arm64_select_reg(&rd, dst->dag_node, c, f, 0);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}
	assert(dst->dag_node->color > 0);

	ret = arm64_overflow_reg2(rd, dst->dag_node, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	return arm64_make_inst_ADR2G(c, f, rd, src->dag_node->var);
}

static int _arm64_inst_div_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rs0  = NULL;
	scf_register_arm64_t* rs1  = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(s0->var)) {

		if (0 == s0->color) {
			s0->color = -1;
			s0->var->global_flag = 1;

		} else if (0 == s1->color) {
			s1->color = -1;
			s1->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd,  d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rs0, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rs1, s1, c, f, 1);

		opcode  = (0x1e   << 24)  | (0x1    << 21) | (0x3 << 11);
		opcode |= (rs1->id << 16) | (rs0->id << 5) | rd->id;
		opcode |= ((8 == rd->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (0 == s1->color) {

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rs0, s0, c, f, 1);

		int ret = arm64_select_free_reg(&rs1, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs1, s1->var->data.u64, rs1->bytes);
		if (ret < 0)
			return ret;

	} else if (0 == s0->color) {

		if (!scf_variable_const_interger(s0->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rs1, s1, c, f, 1);

		int ret = arm64_select_free_reg(&rs0, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs0, s0->var->data.u64, rs0->bytes);
		if (ret < 0)
			return ret;

	} else {
		ARM64_SELECT_REG_CHECK(&rs0, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rs1, s1, c, f, 1);
	}

	opcode = ((8 == rs0->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs1->id << 16) | (0x2 << 10) | (rs0->id << 5) | rd->id;

	if (scf_variable_signed(s0->var))
		opcode |= 0x3 << 10;

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_mod_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rs0  = NULL;
	scf_register_arm64_t* rs1  = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src0->dag_node->var)) {

		assert(scf_variable_float(src1->dag_node->var));
		assert(scf_variable_float(dst->dag_node->var));
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (0 == s1->color) {

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rs0, s0, c, f, 1);

		int ret = arm64_select_free_reg(&rs1, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs1, s1->var->data.u64, rs1->bytes);
		if (ret < 0)
			return ret;

	} else if (0 == s0->color) {

		if (!scf_variable_const_interger(s0->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rs1, s1, c, f, 1);

		int ret = arm64_select_free_reg(&rs0, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs0, s0->var->data.u64, rs0->bytes);
		if (ret < 0)
			return ret;

	} else {
		ARM64_SELECT_REG_CHECK(&rs0, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rs1, s1, c, f, 1);
	}

	opcode = ((8 == rs0->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs1->id << 16) | (0x2 << 10) | (rs0->id << 5) | rd->id;

	if (scf_variable_signed(s0->var))
		opcode |= 0x3 << 10;

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	opcode = ((8 == rs0->bytes) << 31) | (0x1b << 24) | (rs1->id << 16) | (0x1 << 15) | (rs0->id << 10) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_mul_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rm   = NULL;
	scf_register_arm64_t* rn   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (0 == s0->color)
		SCF_XCHG(s0, s1);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(s0->var)) {

		if (0 == s0->color) {
			s0->color = -1;
			s0->var->global_flag = 1;

		} else if (0 == s1->color) {
			s1->color = -1;
			s1->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

		opcode  = (0x1e   << 24) | (0x1    << 21) | (0x1 << 11);
		opcode |= (rm->id << 16) | (rn->id << 5)  | rd->id;
		opcode |= ((8 == rd->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
	ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);

	if (0 == s1->color) {

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		s1->color = -1;
		s1->var->tmp_flag = 1;
		ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

		opcode = (0x9b << 24) | (rm->id << 16) | (0x1f << 10) | (rn->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		s1->color  = 0;
		s1->loaded = 0;
		s1->var->tmp_flag = 0;
		assert(0 == scf_vector_del(rm->dag_nodes, s1));
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

	opcode = (0x9b << 24) | (rm->id << 16) | (0x1f << 10) | (rn->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_mul_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (src->dag_node->var->size != dst->dag_node->var->size)
		return -EINVAL;

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	assert(0 != d->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(s->var)) {

		if (0 == s->color) {
			s->color = -1;
			s->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode  = (0x1e   << 24) | (0x1    << 21) | (0x1 << 11);
		opcode |= (rs->id << 16) | (rd->id << 5)  | rd->id;
		opcode |= ((8 == rd->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

	if (0 == s->color) {

		if (!scf_variable_const_interger(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		s->color = -1;
		s->var->tmp_flag = 1;
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode = (0x9b << 24) | (rs->id << 16) | (0x1f << 10) | (rd->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		s->color  = 0;
		s->loaded = 0;
		s->var->tmp_flag = 0;
		assert(0 == scf_vector_del(rs->dag_nodes, s));
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	opcode = (0x9b << 24) | (rs->id << 16) | (0x1f << 10) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_add_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rm   = NULL;
	scf_register_arm64_t* rn   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (0 == s0->color)
		SCF_XCHG(s0, s1);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src0->dag_node->var)) {

		if (0 == s1->color) {
			s1->color = -1;
			s1->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

		opcode  = (0x1e   << 24) | (0x1    << 21) | (0x1 << 13) | (0x1 << 11);
		opcode |= (rm->id << 16) | (rn->id <<  5) | rd->id;
		opcode |= (8 == rd->bytes) << 22;

		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	if (0 == s1->color) {

		if (scf_variable_const_string(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		uint64_t u = s1->var->data.u64;

		uint32_t sh  = 0;
		uint32_t imm = 0;

		if (u <= 0xfff)
			imm = u;

		else if (0 == (u & 0xfff) && (u >> 12) <= 0xfff) {
			sh  = 1;
			imm = u >> 12;

		} else {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);

		opcode = (0x91 << 24) | (sh << 22) | (imm << 10) | (rn->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
	ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);
	ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

	opcode = (0x8b << 24) | (rm->id << 16) | (rn->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_add_sub_assign(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24i, uint32_t u24r)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (src->dag_node->var->size != dst->dag_node->var->size)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	assert(0 != d->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(s->var)) {

		if (0 == s->color) {
			s->color = -1;
			s->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		uint32_t op;

		if (0x11 == u24i)
			op   =  0x5;

		else if (0x51 == u24i)
			op   =  0x7;
		else
			return -EINVAL;

		opcode  = (0x1e   << 24) | (0x1    << 21) | (op << 11);
		opcode |= (rs->id << 16) | (rd->id <<  5) | rd->id;
		opcode |= (8 == rd->bytes) << 22;

		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	if (0 == s->color) {

		if (scf_variable_const_string(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		if (!scf_variable_const_interger(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		uint64_t u = s->var->data.u64;

		uint32_t sh  = 0;
		uint32_t imm = 0;

		if (u <= 0xfff)
			imm = u;

		else if (0 == (u & 0xfff) && (u >> 12) <= 0xfff) {
			sh  = 1;
			imm = u >> 12;

		} else {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

		opcode = ((8 == rd->bytes) << 31) | (u24i << 24) | (sh << 22) | (imm << 10) | (rd->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);
	ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	opcode = ((8 == rd->bytes) << 31) | (u24r << 24) | (rs->id << 16) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_add_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_add_sub_assign(ctx, c, 0x11, 0x0b);
}

static int _arm64_inst_sub_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_add_sub_assign(ctx, c, 0x51, 0x4b);
}

static int _arm64_inst_sub_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rm   = NULL;
	scf_register_arm64_t* rn   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src0->dag_node->var)) {

		if (0 == s0->color) {
			s0->color = -1;
			s0->var->global_flag = 1;

		} else if (0 == s1->color) {
			s1->color = -1;
			s1->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

		opcode  = (0x1e   << 24) | (0x1    << 21) | (0x7 << 11);
		opcode |= (rm->id << 16) | (rn->id << 5)  | rd->id;
		opcode |= ((8 == rd->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	int neg = 0;

	if (0 == s0->color) {
		neg = 1;
		SCF_XCHG(s0, s1);
	}

	if (0 == s1->color) {

		if (scf_variable_const_string(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		uint64_t u = s1->var->data.u64;

		uint32_t sh  = 0;
		uint32_t imm = 0;

		if (u <= 0xfff)
			imm = u;

		else if (0 == (u & 0xfff) && (u >> 12) <= 0xfff) {
			sh  = 1;
			imm = u >> 12;

		} else {
			scf_loge("\n");
			return -EINVAL;
		}

		ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
		ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);

		opcode = ((8 == rn->bytes) << 31) | (0x51 << 24) | (sh << 22) | (imm << 10) | (rn->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (neg) {
			opcode = ((8 == rd->bytes) << 31) | (0x4b << 24) | (rd->id << 16) | (0x1f << 5) | rd->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
	ARM64_SELECT_REG_CHECK(&rn, s0, c, f, 1);
	ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

	opcode = ((8 == rn->bytes) << 31) | (0x4b << 24) | (rm->id << 16) | (rn->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_inst_bit_op(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t op)
{
	ARM64_INST_OP3_CHECK()

	scf_register_arm64_t* rm   = NULL;
	scf_register_arm64_t* rn   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	assert(0 != d->color);
	assert(0 != s0->color || 0 != s1->color);

	if (0 == s0->color)
		SCF_XCHG(s0, s1);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src0->dag_node->var)) {

		assert(scf_variable_float(src1->dag_node->var));
		assert(scf_variable_float(dst->dag_node->var));
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);
	ARM64_SELECT_REG_CHECK(&rn, d,  c, f, 1);

	if (0 == s1->color) {

		if (!scf_variable_const_interger(s1->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		s1->color = -1;
		s1->var->tmp_flag = 1;
		ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

		opcode = (op << 24) | (rm->id << 16) | (rn->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		s1->color  = 0;
		s1->loaded = 0;
		s1->var->tmp_flag = 0;
		assert(0 == scf_vector_del(rm->dag_nodes, s1));
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rm, s1, c, f, 1);

	opcode = (op << 24) | (rm->id << 16) | (rn->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_inst_bit_op_assign(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t op)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (src->dag_node->var->size != dst->dag_node->var->size)
		return -EINVAL;

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	assert(0 != d->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src->dag_node->var)) {

		assert(scf_variable_float(dst->dag_node->var));
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (0 == s->color) {

		if (!scf_variable_const_interger(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		s->color = -1;
		s->var->tmp_flag = 1;
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode = (op << 24) | (rs->id << 16) | (rd->id << 5) | rd->id;
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		s->color  = 0;
		s->loaded = 0;
		s->var->tmp_flag = 0;
		assert(0 == scf_vector_del(rs->dag_nodes, s));
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	opcode = (op << 24) | (rs->id << 16) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_bit_and_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return arm64_inst_bit_op(ctx, c, 0x8a);
}

static int _arm64_inst_bit_or_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return arm64_inst_bit_op(ctx, c, 0xaa);
}

static int _arm64_inst_and_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return arm64_inst_bit_op_assign(ctx, c, 0x8a);
}

static int _arm64_inst_or_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return arm64_inst_bit_op_assign(ctx, c, 0xaa);
}

static int _arm64_inst_teq_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rs;
	scf_instruction_t*    inst;

	uint32_t opcode;

	if (0 == src->dag_node->color) {
		scf_loge("src->dag_node->var: %p\n", src->dag_node->var);
		return -1;
	}

	ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

	opcode = ((8 == rs->bytes) << 1) | (0x6a << 24) | (rs->id << 16) | (rs->id << 5) | 0x1f;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_set(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t cc)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rd;
	scf_instruction_t*    inst;

	uint32_t opcode;

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_logic_not_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	int ret = _arm64_inst_teq_handler(ctx, c);
	if (ret < 0)
		return ret;

	return _arm64_inst_set(ctx, c, 0x1);
}

static int _arm64_inst_cmp_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*    f     = arm64->f;
	scf_3ac_operand_t* s0    = c->srcs->data[0];
	scf_3ac_operand_t* s1    = c->srcs->data[1];

	if (!s0 || !s0->dag_node)
		return -EINVAL;

	if (!s1 || !s1->dag_node)
		return -EINVAL;

	scf_instruction_t*    inst;
	scf_register_arm64_t* rs1  = NULL;
	scf_register_arm64_t* rs0  = NULL;
	scf_dag_node_t*       ds0  = s0->dag_node;
	scf_dag_node_t*       ds1  = s1->dag_node;
	scf_rela_t*           rela = NULL;

	uint32_t  opcode;

	if (ds0->var->size != ds1->var->size)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (0 == ds0->color) {
		scf_loge("src0 should be a var\n");
		if (ds0->var->w)
			scf_loge("src0: '%s'\n", ds0->var->w->text->data);
		else
			scf_loge("src0: v_%#lx\n", 0xffff & (uintptr_t)ds0->var);
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rs0, ds0, c, f, 1);

	if (scf_variable_float(ds0->var)) {

		if (0 == ds1->color) {
			ds1->color = -1;
			ds1->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rs0, ds0, c, f, 1);
		ARM64_SELECT_REG_CHECK(&rs1, ds1, c, f, 1);

		opcode  = (0x1e   << 24)  | (0x1    << 21) | (0x1 << 13);
		opcode |= (rs1->id << 16) | (rs0->id << 5);
		opcode |= ((8 == rs0->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	if (0 == ds1->color) {

		uint64_t u = ds1->var->data.u64;

		if (u <= 0xfff)
			opcode = (0x71 << 24) | (u << 10) | (rs0->id << 5) | 0x1f;

		else if (0 == (u & 0xfff) && (u >> 12) <= 0xfff)
			opcode = (0x71 << 24) | (1 << 22) | (u << 10) | (rs0->id << 5) | 0x1f;

		else {
			ds1->loaded =  0;
			ds1->color  = -1;
			ARM64_SELECT_REG_CHECK(&rs1, ds1, c, f, 1);

			opcode = (0x6b << 24) | (rs1->id << 16) | (rs0->id << 5) | 0x1f;

			ds1->loaded = 0;
			ds1->color  = 0;
			assert(0 == scf_vector_del(rs1->dag_nodes, ds1));
		}

		if (rs0->bytes > 4)
			opcode |= (0x1 << 31);

		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rs1, ds1, c, f, 1);

	opcode = (0x6b << 24) | (rs1->id << 16) | (rs0->id << 5) | 0x1f;

	if (rs0->bytes > 4)
		opcode |= (0x1 << 31);

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

#define ARM64_INST_SET(name, cc) \
static int _arm64_inst_##name##_handler(scf_native_t* ctx, scf_3ac_code_t* c) \
{ \
	return _arm64_inst_set(ctx, c, cc); \
}
ARM64_INST_SET(setz,  0x1)
ARM64_INST_SET(setnz, 0x0)
ARM64_INST_SET(setgt, 0xd)
ARM64_INST_SET(setge, 0xb)
ARM64_INST_SET(setlt, 0xa)
ARM64_INST_SET(setle, 0xc)

#define ARM64_INST_CMP_SET(name, cc) \
static int _arm64_inst_##name##_handler(scf_native_t* ctx, scf_3ac_code_t* c) \
{ \
	int ret = _arm64_inst_cmp_handler(ctx, c); \
	if (ret < 0) \
	   return ret; \
	return _arm64_inst_set(ctx, c, cc); \
}
ARM64_INST_CMP_SET(eq, 0x1)
ARM64_INST_CMP_SET(ne, 0x0)
ARM64_INST_CMP_SET(gt, 0xd)
ARM64_INST_CMP_SET(ge, 0xb)
ARM64_INST_CMP_SET(lt, 0xa)
ARM64_INST_CMP_SET(le, 0xc)

static int _arm64_inst_cast_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_instruction_t*   inst;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (0 == dst->dag_node->color)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rd = NULL;
	scf_register_arm64_t* rs = NULL;
	scf_dag_node_t*       d  = dst->dag_node;
	scf_dag_node_t*       s  = src->dag_node;
	scf_variable_t*       vs = s->var;
	scf_variable_t*       vd = d->var;

	int src_size = arm64_variable_size(vs);
	int dst_size = arm64_variable_size(vd);

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

	uint32_t opcode = 0;

	if (scf_variable_float(vs)) {

		if (0 == s->color) {
			s->color = -1;
			vs->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		if (scf_variable_float(vd)) {

			if (rd->bytes == rs->bytes)
				opcode = (0x1e << 24) | ((8 == rd->bytes) << 22) | (0x1 << 21) | (0x1 << 14) | (rs->id << 5) | rd->id;

			else if (4 == rs->bytes)
				opcode = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (0x1 << 15) | (0x1 << 14) | (rs->id << 5) | rd->id;
			else
				opcode = (0x1e << 24) | (0x1 << 22) | (0x1 << 21) | (0x1 << 17) | (0x1 << 14) | (rs->id << 5) | rd->id;

		} else {
			opcode  = ((8 == rd->bytes) << 31) | (0x1e << 24) | ((8 == rs->bytes) << 22) | (0x7 << 19) | (rs->id << 5) | rd->id;

			opcode |= (!scf_variable_signed(vd)) << 16;
		}

		inst = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;

	} else if (scf_variable_float(vd)) {

		if (0 == s->color) {

			if (!scf_variable_const_interger(vs))
				return -EINVAL;

			if (src_size < dst_size)
				scf_variable_extend_bytes(vs, 8);

			int ret = arm64_select_free_reg(&rs, c, f, 0);
			if (ret < 0)
				return ret;

			ret = arm64_make_inst_I2G(c, rs, vs->data.u64, dst_size);
			if (ret < 0)
				return ret;

		} else
			ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode  = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (rs->id << 5) | rd->id;

		opcode |= ((8 == rs->bytes) << 31) | ((8 == rd->bytes) << 22);

		opcode |= (!scf_variable_signed(vd)) << 16;

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	if (vs->nb_dimentions > 0)
		return arm64_make_inst_ADR2G(c, f, rd, vs);

	scf_logw("src_size: %d, dst_size: %d\n", src_size, dst_size);

	if (0 == s->color) {

		if (!scf_variable_const_interger(vs))
			return -EINVAL;

		if (src_size < dst_size)
			scf_variable_extend_bytes(vs, 8);

		return arm64_make_inst_I2G(c, rd, vs->data.u64, dst_size);
	}

	ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	if (src_size < dst_size) {

		if (scf_variable_signed(vs)) {

			opcode = (0x93 << 24) | (0x1 << 22) | (rs->id << 5) | rd->id;

			if (1 == src_size)
				opcode |= 0x7 << 10;

			else if (2 == src_size)
				opcode |= 0xf << 10;

			else if (4 == src_size)
				opcode |= 0x1f << 10;

		} else {
			opcode = (0x53 << 24) | (rs->id << 5) | rd->id;

			if (1 == src_size)
				opcode |= 0x7 << 10;

			else if (2 == src_size)
				opcode |= 0xf << 10;
		}
	}

	if (0 == opcode)
		opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_div_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (src->dag_node->var->size != dst->dag_node->var->size)
		return -EINVAL;

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	assert(0 != d->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(s->var)) {

		if (0 == s->color) {
			s->color = -1;
			s->var->global_flag = 1;
		}

		ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

		opcode  = (0x1e   << 24) | (0x1    << 21) | (0x3 << 11);
		opcode |= (rs->id << 16) | (rd->id << 5)  | rd->id;
		opcode |= ((8 == rd->bytes) << 22);

		inst    = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

	if (0 == s->color) {

		if (!scf_variable_const_interger(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		int ret = arm64_select_free_reg(&rs, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs, s->var->data.u64, rs->bytes);
		if (ret < 0)
			return ret;

	} else
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	opcode = ((8 == rd->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs->id << 16) | (0x2 << 10) | (rd->id << 5) | rd->id;

	if (scf_variable_signed(s->var))
		opcode |= 0x3 << 10;

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_mod_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*   dst   = c->dsts->data[0];
	scf_3ac_operand_t*   src   = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (src->dag_node->var->size != dst->dag_node->var->size)
		return -EINVAL;

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_register_arm64_t* r    = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	assert(0 != d->color);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	if (scf_variable_float(src->dag_node->var)) {

		assert(scf_variable_float(dst->dag_node->var));
		return -EINVAL;
	}

	ARM64_SELECT_REG_CHECK(&rd, d, c, f, 0);

	if (0 == s->color) {

		if (!scf_variable_const_interger(s->var)) {
			scf_loge("\n");
			return -EINVAL;
		}

		int ret = arm64_select_free_reg(&rs, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs, s->var->data.u64, rs->bytes);
		if (ret < 0)
			return ret;

	} else
		ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	int ret = arm64_select_free_reg(&r, c, f, 0);
	if (ret < 0)
		return ret;

	opcode = ((8 == rd->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs->id << 16) | (0x2 << 10) | (rd->id << 5) | r->id;

	if (scf_variable_signed(s->var))
		opcode |= 0x3 << 10;

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	opcode = ((8 == rd->bytes) << 31) | (0x1b << 24) | (rs->id << 16) | (0x1 << 15) | (rd->id << 10) | (r->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_return_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size < 1)
		return -EINVAL;

	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*       f     = arm64->f;
	scf_3ac_operand_t*    src   = NULL;
	scf_variable_t*       v     = NULL;
	scf_instruction_t*    inst  = NULL;
	scf_rela_t*           rela  = NULL;

	scf_register_arm64_t* rd    = NULL;
	scf_register_arm64_t* rs    = NULL;
	scf_register_arm64_t* sp    = arm64_find_register("sp");
	scf_register_arm64_t* fp    = arm64_find_register("fp");

	scf_arm64_OpCode_t*   pop;
	scf_arm64_OpCode_t*   mov;
	scf_arm64_OpCode_t*   ret;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	int i;
	for (i  = 0; i < c->srcs->size; i++) {
		src =        c->srcs->data[i];

		v   = src->dag_node->var;

		int size     = arm64_variable_size(v);
		int is_float = scf_variable_float(v);

		if (i > 0 && is_float) {
			scf_loge("\n");
			return -1;
		}

		int retsize = size > 4 ? 8 : 4;

		if (is_float) {
			rd = arm64_find_register_type_id_bytes(is_float, 0, retsize);

			if (0 == src->dag_node->color) {
				src->dag_node->color = -1;
				v->global_flag       =  1;
			}

			scf_loge("\n");
			return -1;

		} else {
			rd = arm64_find_register_type_id_bytes(is_float, arm64_abi_ret_regs[i], retsize);

			if (0 == src->dag_node->color) {
				if (rd->bytes > size)
					scf_variable_extend_bytes(v, rd->bytes);

				int ret = arm64_make_inst_I2G(c, rd, v->data.u64, rd->bytes);
				if (ret < 0)
					return ret;
				continue;
			}
		}

		scf_logd("rd: %s, rd->dag_nodes->size: %d\n", rd->name, rd->dag_nodes->size);

		if (src->dag_node->color > 0) {

			int start = c->instructions->size;

			ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

			if (!ARM64_COLOR_CONFLICT(rd->color, rs->color)) {

				int ret = arm64_save_reg(rd, c, f);
				if (ret < 0)
					return ret;

				uint32_t opcode;

				if (rd->bytes > size) {

					if (scf_variable_signed(v)) {

						if (1 == size)
							opcode = (0x93 << 24) | (0x1 << 22) | (0x7 << 10) | (rs->id << 5) | rd->id;

						else if (2 == size)
							opcode = (0x93 << 24) | (0x1 << 22) | (0xf << 10) | (rs->id << 5) | rd->id;

						else if (4 == size)
							opcode = (0x93 << 24) | (0x1 << 22) | (0x1f << 10) | (rs->id << 5) | rd->id;

						else {
							scf_loge("\n");
							return -EINVAL;
						}

						inst   = arm64_make_inst(c, opcode);
						ARM64_INST_ADD_CHECK(c->instructions, inst);

					} else {
						if (1 == size)
							opcode = (0x53 << 24) | (0x7 << 10) | (rs->id << 5) | rd->id;

						else if (2 == size)
							opcode = (0x53 << 24) | (0xf << 10) | (rs->id << 5) | rd->id;

						inst   = arm64_make_inst(c, opcode);
						ARM64_INST_ADD_CHECK(c->instructions, inst);

						opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;
						ARM64_INST_ADD_CHECK(c->instructions, inst);
					}
				} else {
					opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;
					inst   = arm64_make_inst(c, opcode);
					ARM64_INST_ADD_CHECK(c->instructions, inst);
				}

				scf_instruction_t* tmp;
				int j;
				int k;
				for (j = start; j < c->instructions->size; j++) {
					tmp           = c->instructions->data[j];

					for (k = j - 1; k >= j - start; k--)
						c->instructions->data[k + 1] = c->instructions->data[k];

					c->instructions->data[j - start] = tmp;
				}
			}
		} else {
			int ret = arm64_save_reg(rd, c, f);
			if (ret < 0)
				return ret;

			ret = arm64_make_inst_M2G(c, f, rd, NULL, v);
			if (ret < 0)
				return ret;
		}
	}
	return 0;
}

static int _arm64_inst_memset_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 3)
		return -EINVAL;

	scf_arm64_context_t*  arm64   = ctx->priv;
	scf_function_t*     f     = arm64->f;
	scf_3ac_operand_t*  dst   = c->srcs->data[0];
	scf_3ac_operand_t*  data  = c->srcs->data[1];
	scf_3ac_operand_t*  count = c->srcs->data[2];
	scf_instruction_t*  inst  = NULL;

	scf_register_arm64_t*	rax   = arm64_find_register("rax");
	scf_register_arm64_t*	rcx   = arm64_find_register("rcx");
	scf_register_arm64_t*	rdi   = arm64_find_register("rdi");
	scf_register_arm64_t*	rd;
	scf_arm64_OpCode_t*   mov;
	scf_arm64_OpCode_t*   stos;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	int ret = arm64_overflow_reg2(rdi, dst->dag_node, c, f);
	if (ret < 0)
		return ret;

	ret = arm64_overflow_reg2(rax, data->dag_node, c, f);
	if (ret < 0)
		return ret;

	ret = arm64_overflow_reg2(rcx, count->dag_node, c, f);
	if (ret < 0)
		return ret;

#if 0
#define ARM64_MEMSET_LOAD_REG(r, dn) \
	do { \
		int size = arm64_variable_size(dn->var); \
		assert(8 == size); \
		\
		if (0 == dn->color) { \
			mov  = arm64_find_OpCode(SCF_ARM64_MOV, size, size, SCF_ARM64_I2G); \
			inst = arm64_make_inst_I2G(mov, r, (uint8_t*)&dn->var->data, size); \
			ARM64_INST_ADD_CHECK(c->instructions, inst); \
			\
		} else { \
			if (dn->color < 0) \
				dn->color = r->color; \
			ARM64_SELECT_REG_CHECK(&rd, dn, c, f, 1); \
			\
			if (!ARM64_COLOR_CONFLICT(rd->color, r->color)) { \
				mov  = arm64_find_OpCode(SCF_ARM64_MOV, size, size, SCF_ARM64_G2E); \
				inst = arm64_make_inst_G2E(mov, r, rd); \
				ARM64_INST_ADD_CHECK(c->instructions, inst); \
			} \
		} \
	} while (0)
	ARM64_MEMSET_LOAD_REG(rdi, dst  ->dag_node);
	ARM64_MEMSET_LOAD_REG(rax, data ->dag_node);
	ARM64_MEMSET_LOAD_REG(rcx, count->dag_node);

	stos = arm64_find_OpCode(SCF_ARM64_STOS, 1, 8, SCF_ARM64_G);
	inst = arm64_make_inst(stos, 1);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
#endif
	return -1;
}

static int _arm64_inst_nop_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return 0;
}

static int _arm64_inst_end_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* sp  = arm64_find_register("sp");
	scf_register_arm64_t* fp  = arm64_find_register("fp");
	scf_register_arm64_t* r;

	uint32_t sp_fp = 0x910003bf;
	uint32_t ret   = 0xd65f03c0;

	scf_instruction_t*  inst = NULL;

	int i;
	for (i = ARM64_ABI_CALLEE_SAVES_NB - 1; i >= 0; i--) {

		r  = arm64_find_register_type_id_bytes(0, arm64_abi_callee_saves[i], 8);

		if (!r->used) {
			r  = arm64_find_register_type_id_bytes(0, arm64_abi_callee_saves[i], 4);

			if (!r->used)
				continue;
		}

		uint32_t pop = (0xf8 << 24) | (0x1 << 22) | (0x8 << 12) | (0x1 << 10) | (0x1f << 5) | r->id;

		inst = arm64_make_inst(c, pop);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
	}

	inst = arm64_make_inst(c, sp_fp);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	uint32_t pop = (0xf8 << 24) | (0x1 << 22) | (0x8 << 12) | (0x1 << 10) | (0x1f << 5) | fp->id;
	inst = arm64_make_inst(c, pop);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	inst = arm64_make_inst(c, ret);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

#define ARM64_INST_JMP(name, opcode) \
static int _arm64_inst_##name##_handler(scf_native_t* ctx, scf_3ac_code_t* c) \
{ \
	if (!c->dsts || c->dsts->size != 1) \
		return -EINVAL; \
	\
	scf_3ac_operand_t* dst  = c->dsts->data[0]; \
	scf_instruction_t* inst = NULL; \
	\
	if (!dst->bb) \
		return -EINVAL; \
	\
	if (0 == opcode) \
		return -EINVAL; \
	\
	if (!c->instructions) { \
		c->instructions = scf_vector_alloc(); \
		if (!c->instructions) \
			return -ENOMEM; \
	} \
	\
	inst = arm64_make_inst(c, opcode); \
	ARM64_INST_ADD_CHECK(c->instructions, inst); \
	return 0;\
}

ARM64_INST_JMP(goto, 0x14000000)
ARM64_INST_JMP(jz,   0x54000000)
ARM64_INST_JMP(jnz,  0x54000001)
ARM64_INST_JMP(jgt,  0x5400000c)
ARM64_INST_JMP(jge,  0x5400000a)
ARM64_INST_JMP(jlt,  0x5400000b)
ARM64_INST_JMP(jle,  0x5400000d)

ARM64_INST_JMP(ja,   0)
ARM64_INST_JMP(jb,   0)
ARM64_INST_JMP(jae,  0)
ARM64_INST_JMP(jbe,  0)

static int _arm64_inst_load_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* r   = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;

	scf_3ac_operand_t*  dst = c->dsts->data[0];
	scf_dag_node_t*     dn  = dst->dag_node;

	int ret;
	int i;

	if (dn->color < 0)
		return 0;

	scf_variable_t* v = dn->var;

	assert(dn->color > 0);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	r = arm64_find_register_color(dn->color);

	if (arm64_reg_used(r, dn)) {
		dn->color  = -1;
		dn->loaded =  0;
		scf_vector_del(r->dag_nodes, dn);
		return 0;
	}

	ret = arm64_load_reg(r, dn, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ret = scf_vector_add_unique(r->dag_nodes, dn);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}
	return 0;
}

static int _arm64_inst_reload_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* r   = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;

	scf_3ac_operand_t*  dst = c->dsts->data[0];
	scf_dag_node_t*     dn  = dst->dag_node;
	scf_dag_node_t*     dn2 = NULL;

	int ret;
	int i;

	if (dn->color < 0)
		return 0;
	assert(dn->color > 0);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	r   = arm64_find_register_color(dn->color);

	ret = arm64_overflow_reg2(r, dn, c, f);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	dn->loaded = 0;
	ret = arm64_load_reg(r, dn, c, f);
	if (ret < 0)
		return ret;

	ret = scf_vector_add_unique(r->dag_nodes, dn);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	return 0;
}

static int _arm64_inst_save_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_3ac_operand_t*  src = c->srcs->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_function_t*     f   = arm64->f;
	scf_dag_node_t*     dn  = src->dag_node;

	if (dn->color < 0)
		return 0;

	if (!dn->loaded)
		return 0;

	scf_variable_t* v = dn->var;
	assert(dn->color > 0);

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	return arm64_save_var(dn, c, f);
}

static int _arm64_inst_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;
	scf_variable_t*       v    = s->var;

	uint32_t opcode;

	assert(0 != d->color);

	int size     = arm64_variable_size(v);
	int is_float = scf_variable_float(v);

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (is_float) {
		assert(scf_variable_float(v));

		if (0 == s->color) {
			s->color = -1;
			v->global_flag = 1;
			return arm64_make_inst_M2GF(c, f, rd, NULL, v);
		}

		ARM64_SELECT_REG_CHECK(&rs, d,  c, f, 1);

		opcode = (0x1e << 24) | ((8 == rd->bytes) << 22) | (0x1 << 21) | (rs->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		return 0;
	}

	if (0 == s->color) {

		if (scf_variable_const_string(v))
			return arm64_make_inst_ISTR2G(c, f, rd, v);

		if (!scf_variable_const_interger(v)) {
			scf_loge("\n");
			return -EINVAL;
		}

		if (rd->bytes > size)
			scf_variable_extend_bytes(v, rd->bytes);

		return arm64_make_inst_I2G(c, rd, v->data.u64, rd->bytes);
	}

	ARM64_SELECT_REG_CHECK(&rs, s, c, f, 1);

	opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_shift(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src0  = c->srcs->data[0];
	scf_3ac_operand_t*   src1  = c->srcs->data[1];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!src0 || !src0->dag_node)
		return -EINVAL;

	if (!src1 || !src1->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rs0  = NULL;
	scf_register_arm64_t* rs1  = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst ->dag_node;
	scf_dag_node_t*       s0   = src0->dag_node;
	scf_dag_node_t*       s1   = src1->dag_node;

	uint32_t opcode;

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (0 == s0->color) {

		if (scf_variable_signed(s0->var))
			scf_variable_sign_extend(s0->var, 8);
		else
			scf_variable_zero_extend(s0->var, 8);

		int ret = arm64_make_inst_I2G(c, rd, s0->var->data.u64, rd->bytes);
		if (ret < 0)
			return ret;

		rs0 = rd;
	} else
		ARM64_SELECT_REG_CHECK(&rs0, s0,  c, f, 1);

	if (0 == s1->color) {

		int ret = arm64_select_free_reg(&rs1, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs1, s1->var->data.u64, 1);
		if (ret < 0)
			return ret;
	} else
		ARM64_SELECT_REG_CHECK(&rs1, s1,  c, f, 1);

	uint32_t op = (0x9a << 24) | (0x3 << 22) | (0x1 << 13);

	if (SCF_OP_SHR == c->op->type) {
		if (scf_variable_signed(s0->var))
			op |= 0x1 << 11;
		else
			op |= 0x1 << 10;
	}

	opcode = op | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_shift_assign(scf_native_t* ctx, scf_3ac_code_t* c)
{
	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;
	scf_3ac_operand_t*   src   = c->srcs->data[0];
	scf_3ac_operand_t*   dst   = c->dsts->data[0];

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!dst || !dst->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_register_arm64_t* rs   = NULL;
	scf_register_arm64_t* rd   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_dag_node_t*       d    = dst->dag_node;
	scf_dag_node_t*       s    = src->dag_node;

	uint32_t opcode;

	ARM64_SELECT_REG_CHECK(&rd, d,  c, f, 0);

	if (0 == s->color) {

		int ret = arm64_select_free_reg(&rs, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, rs, s->var->data.u64, 1);
		if (ret < 0)
			return ret;
	} else
		ARM64_SELECT_REG_CHECK(&rs, s,  c, f, 1);

	uint32_t op = (0x9a << 24) | (0x3 << 22) | (0x1 << 13);

	if (SCF_OP_SHR == c->op->type) {
		if (scf_variable_signed(s->var))
			op |= 0x1 << 11;
		else
			op |= 0x1 << 10;
	}

	opcode = op | (rs->id << 16) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_shl_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_shift(ctx, c);
}

static int _arm64_inst_shr_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_shift(ctx, c);
}

static int _arm64_inst_shl_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_shift_assign(ctx, c);
}

static int _arm64_inst_shr_assign_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_shift_assign(ctx, c);
}

static int _arm64_inst_binary_assign_dereference(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t op)
{
	if (!c->srcs || c->srcs->size != 2)
		return -EINVAL;

	scf_arm64_context_t* arm64 = ctx->priv;
	scf_function_t*      f     = arm64->f;

	scf_3ac_operand_t*  base   = c->srcs->data[0];
	scf_3ac_operand_t*  src    = c->srcs->data[1];
	scf_instruction_t*  inst;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!src || !src->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	scf_variable_t*       vs  = src ->dag_node->var;
	scf_variable_t*       vb  = base->dag_node->var;

	scf_register_arm64_t* rs  = NULL;
	scf_register_arm64_t* rd  = NULL;
	arm64_sib_t           sib = {0};

	int ret = arm64_dereference_reg(&sib, base->dag_node, NULL, c, f);
	if (ret < 0)
		return ret;

	int is_float = scf_variable_float(vs);

	if (0 == src->dag_node->color) {

		if (is_float) {

			src->dag_node->color = -1;
			vs->global_flag      =  1;

			ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);

		} else {
			ret = arm64_select_free_reg(&rs, c, f, 0);
			if (ret < 0)
				return ret;

			ret = arm64_make_inst_I2G(c, rs, vs->data.u64, sib.size);
			if (ret < 0)
				return ret;

			rs = arm64_find_register_color_bytes(rs->color, sib.size);
		}
	} else
		ARM64_SELECT_REG_CHECK(&rs, src->dag_node, c, f, 1);


	if (op) {
		uint32_t opcode;

		ret = arm64_select_free_reg(&rd, c, f, is_float);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_P2G(c, f, rd, sib.base, sib.disp, sib.size);
		if (ret < 0)
			return ret;

		opcode = ((8 == sib.size) << 31)| op | (rs->id << 16) | (rd->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		rs = rd;
	}

	return arm64_make_inst_G2P(c, f, rs, sib.base, sib.disp, sib.size);
}

static int _arm64_inst_assign_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_dereference(ctx, c, 0);
}

static int _arm64_inst_add_assign_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_dereference(ctx, c, 0x0b << 24);
}

static int _arm64_inst_sub_assign_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_dereference(ctx, c, 0x4b << 24);
}

static int _arm64_inst_and_assign_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_dereference(ctx, c, 0x0a << 24);
}

static int _arm64_inst_or_assign_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_dereference(ctx, c, 0x2a << 24);
}

static int _arm64_inst_assign_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_pointer(ctx, c, 0);
}

static int _arm64_inst_add_assign_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_pointer(ctx, c, 0x0b << 24);
}

static int _arm64_inst_sub_assign_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_pointer(ctx, c, 0x4b << 24);
}

static int _arm64_inst_and_assign_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_pointer(ctx, c, 0x0a << 24);
}

static int _arm64_inst_or_assign_pointer_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_pointer(ctx, c, 0x2a << 24);
}

static int _arm64_inst_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_array_index(ctx, c, 0);
}

static int _arm64_inst_add_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_array_index(ctx, c, 0x0b << 24);
}

static int _arm64_inst_sub_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_array_index(ctx, c, 0x4b << 24);
}

static int _arm64_inst_and_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_array_index(ctx, c, 0x0a << 24);
}

static int _arm64_inst_or_assign_array_index_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inst_binary_assign_array_index(ctx, c, 0x2a << 24);
}

static int _arm64_inc_dec_dereference(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->srcs || c->srcs->size != 1) {
		scf_loge("\n");
		return -EINVAL;
	}

	scf_register_arm64_t* r     = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    base  = c->srcs->data[0];
	scf_instruction_t*    inst;
	scf_function_t*       f     = arm64->f;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	arm64_sib_t sib = {0};

	int size = base->dag_node->var->data_size;

	int ret  = arm64_dereference_reg(&sib, base->dag_node, NULL, c, f);
	if (ret < 0)
		return ret;

	ret = arm64_select_free_reg(&r, c, f, 0);
	if (ret < 0)
		return ret;

	ret = arm64_make_inst_P2G(c, f, r, sib.base, 0, size);
	if (ret < 0)
		return ret;

	uint32_t opcode = (u24 << 24) | (0x1 << 10) | (r->id << 5) | r->id;

	if (size > 4)
		opcode |= 0x1 << 31;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return arm64_make_inst_G2P(c, f, r, sib.base, 0, size);
}

static int _arm64_inc_dec_post_dereference(scf_native_t* ctx, scf_3ac_code_t* c, uint32_t u24)
{
	if (!c->srcs || c->srcs->size != 1)
		return -EINVAL;

	if (!c->dsts || c->dsts->size != 1)
		return -EINVAL;

	scf_register_arm64_t* rd    = NULL;
	scf_arm64_context_t*  arm64 = ctx->priv;
	scf_3ac_operand_t*    base  = c->srcs->data[0];
	scf_3ac_operand_t*    dst   = c->dsts->data[0];
	scf_instruction_t*    inst;
	scf_function_t*       f     = arm64->f;

	if (!base || !base->dag_node)
		return -EINVAL;

	if (!c->instructions) {
		c->instructions = scf_vector_alloc();
		if (!c->instructions)
			return -ENOMEM;
	}

	arm64_sib_t sib = {0};

	int size = base->dag_node->var->data_size;

	int ret  = arm64_dereference_reg(&sib, base->dag_node, NULL, c, f);
	if (ret < 0)
		return ret;

	ARM64_SELECT_REG_CHECK(&rd, dst->dag_node, c, f, 0);

	ret = arm64_make_inst_P2G(c, f, rd, sib.base, 0, size);
	if (ret < 0)
		return ret;

	uint32_t opcode = (u24 << 24) | (0x1 << 10) | (rd->id << 5) | rd->id;

	if (size > 4)
		opcode |= 0x1 << 31;

	inst = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	ret = arm64_make_inst_G2P(c, f, rd, sib.base, 0, size);
	if (ret < 0)
		return ret;

	opcode &= ~(0x1 << 30);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

static int _arm64_inst_dec_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inc_dec_dereference(ctx, c, 0x51);
}

static int _arm64_inst_inc_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inc_dec_dereference(ctx, c, 0x11);
}

static int _arm64_inst_dec_post_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inc_dec_post_dereference(ctx, c, 0x51);
}

static int _arm64_inst_inc_post_dereference_handler(scf_native_t* ctx, scf_3ac_code_t* c)
{
	return _arm64_inc_dec_post_dereference(ctx, c, 0x11);
}

static arm64_inst_handler_t arm64_inst_handlers[] = {

	{SCF_OP_CALL,			_arm64_inst_call_handler},
	{SCF_OP_ARRAY_INDEX, 	_arm64_inst_array_index_handler},
	{SCF_OP_POINTER,        _arm64_inst_pointer_handler},

	{SCF_OP_TYPE_CAST,      _arm64_inst_cast_handler},
	{SCF_OP_LOGIC_NOT, 		_arm64_inst_logic_not_handler},
	{SCF_OP_BIT_NOT,        _arm64_inst_bit_not_handler},
	{SCF_OP_NEG, 			_arm64_inst_neg_handler},

	{SCF_OP_VA_START,       _arm64_inst_va_start_handler},
	{SCF_OP_VA_ARG,         _arm64_inst_va_arg_handler},
	{SCF_OP_VA_END,         _arm64_inst_va_end_handler},

	{SCF_OP_INC,            _arm64_inst_inc_handler},
	{SCF_OP_DEC,            _arm64_inst_dec_handler},

	{SCF_OP_INC_POST,       _arm64_inst_inc_post_handler},
	{SCF_OP_DEC_POST,       _arm64_inst_dec_post_handler},

	{SCF_OP_DEREFERENCE, 	_arm64_inst_dereference_handler},
	{SCF_OP_ADDRESS_OF, 	_arm64_inst_address_of_handler},

	{SCF_OP_MUL, 			_arm64_inst_mul_handler},
	{SCF_OP_DIV, 			_arm64_inst_div_handler},
	{SCF_OP_MOD,            _arm64_inst_mod_handler},

	{SCF_OP_ADD, 			_arm64_inst_add_handler},
	{SCF_OP_SUB, 			_arm64_inst_sub_handler},

	{SCF_OP_SHL,            _arm64_inst_shl_handler},
	{SCF_OP_SHR,            _arm64_inst_shr_handler},

	{SCF_OP_BIT_AND,        _arm64_inst_bit_and_handler},
	{SCF_OP_BIT_OR,         _arm64_inst_bit_or_handler},

	{SCF_OP_3AC_TEQ,        _arm64_inst_teq_handler},
	{SCF_OP_3AC_CMP,        _arm64_inst_cmp_handler},

	{SCF_OP_3AC_SETZ,       _arm64_inst_setz_handler},
	{SCF_OP_3AC_SETNZ,      _arm64_inst_setnz_handler},
	{SCF_OP_3AC_SETGT,      _arm64_inst_setgt_handler},
	{SCF_OP_3AC_SETGE,      _arm64_inst_setge_handler},
	{SCF_OP_3AC_SETLT,      _arm64_inst_setlt_handler},
	{SCF_OP_3AC_SETLE,      _arm64_inst_setle_handler},

	{SCF_OP_EQ, 			_arm64_inst_eq_handler},
	{SCF_OP_NE, 			_arm64_inst_ne_handler},
	{SCF_OP_GT, 			_arm64_inst_gt_handler},
	{SCF_OP_GE,             _arm64_inst_ge_handler},
	{SCF_OP_LT,             _arm64_inst_lt_handler},
	{SCF_OP_LE,             _arm64_inst_le_handler},

	{SCF_OP_ASSIGN, 		_arm64_inst_assign_handler},

	{SCF_OP_ADD_ASSIGN,     _arm64_inst_add_assign_handler},
	{SCF_OP_SUB_ASSIGN,     _arm64_inst_sub_assign_handler},

	{SCF_OP_MUL_ASSIGN,     _arm64_inst_mul_assign_handler},
	{SCF_OP_DIV_ASSIGN,     _arm64_inst_div_assign_handler},
	{SCF_OP_MOD_ASSIGN,     _arm64_inst_mod_assign_handler},

	{SCF_OP_SHL_ASSIGN,     _arm64_inst_shl_assign_handler},
	{SCF_OP_SHR_ASSIGN,     _arm64_inst_shr_assign_handler},

	{SCF_OP_AND_ASSIGN,     _arm64_inst_and_assign_handler},
	{SCF_OP_OR_ASSIGN,      _arm64_inst_or_assign_handler},

	{SCF_OP_RETURN, 		_arm64_inst_return_handler},
	{SCF_OP_GOTO, 			_arm64_inst_goto_handler},

	{SCF_OP_3AC_JZ, 		_arm64_inst_jz_handler},
	{SCF_OP_3AC_JNZ, 		_arm64_inst_jnz_handler},
	{SCF_OP_3AC_JGT, 		_arm64_inst_jgt_handler},
	{SCF_OP_3AC_JGE, 		_arm64_inst_jge_handler},
	{SCF_OP_3AC_JLT, 		_arm64_inst_jlt_handler},
	{SCF_OP_3AC_JLE, 		_arm64_inst_jle_handler},

	{SCF_OP_3AC_JA,         _arm64_inst_ja_handler},
	{SCF_OP_3AC_JB,         _arm64_inst_jb_handler},
	{SCF_OP_3AC_JAE,        _arm64_inst_jae_handler},
	{SCF_OP_3AC_JBE,        _arm64_inst_jbe_handler},

	{SCF_OP_3AC_NOP, 		_arm64_inst_nop_handler},
	{SCF_OP_3AC_END, 		_arm64_inst_end_handler},

	{SCF_OP_3AC_SAVE,       _arm64_inst_save_handler},
	{SCF_OP_3AC_LOAD,       _arm64_inst_load_handler},

	{SCF_OP_3AC_RESAVE,     _arm64_inst_save_handler},
	{SCF_OP_3AC_RELOAD,     _arm64_inst_reload_handler},

	{SCF_OP_3AC_INC,        _arm64_inst_inc_handler},
	{SCF_OP_3AC_DEC,        _arm64_inst_dec_handler},

	{SCF_OP_3AC_PUSH_RAX,   _arm64_inst_push_rax_handler},
	{SCF_OP_3AC_POP_RAX,    _arm64_inst_pop_rax_handler},

	{SCF_OP_3AC_MEMSET,     _arm64_inst_memset_handler},

	{SCF_OP_3AC_ASSIGN_DEREFERENCE,     _arm64_inst_assign_dereference_handler},
	{SCF_OP_3AC_ASSIGN_ARRAY_INDEX,     _arm64_inst_assign_array_index_handler},
	{SCF_OP_3AC_ASSIGN_POINTER,         _arm64_inst_assign_pointer_handler},

	{SCF_OP_3AC_ADD_ASSIGN_DEREFERENCE, _arm64_inst_add_assign_dereference_handler},
	{SCF_OP_3AC_ADD_ASSIGN_ARRAY_INDEX, _arm64_inst_add_assign_array_index_handler},
	{SCF_OP_3AC_ADD_ASSIGN_POINTER,     _arm64_inst_add_assign_pointer_handler},

	{SCF_OP_3AC_SUB_ASSIGN_DEREFERENCE, _arm64_inst_sub_assign_dereference_handler},
	{SCF_OP_3AC_SUB_ASSIGN_ARRAY_INDEX, _arm64_inst_sub_assign_array_index_handler},
	{SCF_OP_3AC_SUB_ASSIGN_POINTER,     _arm64_inst_sub_assign_pointer_handler},

	{SCF_OP_3AC_AND_ASSIGN_DEREFERENCE, _arm64_inst_and_assign_dereference_handler},
	{SCF_OP_3AC_AND_ASSIGN_ARRAY_INDEX, _arm64_inst_and_assign_array_index_handler},
	{SCF_OP_3AC_AND_ASSIGN_POINTER,     _arm64_inst_and_assign_pointer_handler},

	{SCF_OP_3AC_OR_ASSIGN_DEREFERENCE,  _arm64_inst_or_assign_dereference_handler},
	{SCF_OP_3AC_OR_ASSIGN_ARRAY_INDEX,  _arm64_inst_or_assign_array_index_handler},
	{SCF_OP_3AC_OR_ASSIGN_POINTER,      _arm64_inst_or_assign_pointer_handler},

	{SCF_OP_3AC_INC_DEREFERENCE,        _arm64_inst_inc_dereference_handler},
	{SCF_OP_3AC_INC_ARRAY_INDEX,        _arm64_inst_inc_array_index_handler},
	{SCF_OP_3AC_INC_POINTER,            _arm64_inst_inc_pointer_handler},

	{SCF_OP_3AC_INC_POST_DEREFERENCE,   _arm64_inst_inc_post_dereference_handler},
	{SCF_OP_3AC_INC_POST_ARRAY_INDEX,   _arm64_inst_inc_post_array_index_handler},
	{SCF_OP_3AC_INC_POST_POINTER,       _arm64_inst_inc_post_pointer_handler},

	{SCF_OP_3AC_DEC_DEREFERENCE,        _arm64_inst_dec_dereference_handler},
	{SCF_OP_3AC_DEC_ARRAY_INDEX,        _arm64_inst_dec_array_index_handler},
	{SCF_OP_3AC_DEC_POINTER,            _arm64_inst_dec_pointer_handler},

	{SCF_OP_3AC_DEC_POST_DEREFERENCE,   _arm64_inst_dec_post_dereference_handler},
	{SCF_OP_3AC_DEC_POST_ARRAY_INDEX,   _arm64_inst_dec_post_array_index_handler},
	{SCF_OP_3AC_DEC_POST_POINTER,       _arm64_inst_dec_post_pointer_handler},

	{SCF_OP_3AC_ADDRESS_OF_ARRAY_INDEX, _arm64_inst_address_of_array_index_handler},
	{SCF_OP_3AC_ADDRESS_OF_POINTER,     _arm64_inst_address_of_pointer_handler},
};

arm64_inst_handler_t* scf_arm64_find_inst_handler(const int op_type)
{
	int i;
	for (i = 0; i < sizeof(arm64_inst_handlers) / sizeof(arm64_inst_handlers[0]); i++) {

		arm64_inst_handler_t* h = &(arm64_inst_handlers[i]);

		if (op_type == h->type)
			return h;
	}
	return NULL;
}
