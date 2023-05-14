#include"scf_risc.h"

int arm32_inst_I2G(scf_3ac_code_t* c, scf_register_t* rd, uint64_t imm, int bytes)
{
	scf_instruction_t* inst;

	uint32_t opcode;

	// mov rd, imm[15:0]
	opcode = (0xe3 << 24) | (((imm >> 12 ) & 0xf) << 16) | (rd->id << 12) | (imm & 0xfff);
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	imm >>= 16;
	if (imm & 0xffff) {

		// movk rd, imm[31:16]
		opcode = (0xe3 << 24) | (0x1 << 22) | (((imm >> 12 ) & 0xf) << 16) | (rd->id << 12) | (imm & 0xfff);
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);
	}

	return 0;
}

int arm32_inst_ADR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs)
{
	scf_register_t*       fp   = f->rops->find_register("fp");
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int64_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = f->rops->variable_size(vs);

	if (vs->local_flag || vs->tmp_flag) {

		offset = vs->bp_offset;

		if (offset >= 0 && offset <= 0xff)

			opcode = (0xe2 << 24) | (0x8 << 20) | (fp->id << 16) | (rd->id << 12) | offset;

		else if (offset < 0 && -offset <= 0xff)

			opcode = (0xe2 << 24) | (0x4 << 20) | (fp->id << 16) | (rd->id << 12) | (-offset);

		else {
			int ret = arm32_inst_I2G(c, rd, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe0 << 24) | (0x8 << 20) | (fp->id << 16) | (rd->id << 12) | rd->id;
		}

		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

	} else if (vs->global_flag) {
		offset = 0;

		opcode = (0xe3 << 24) | (rd->id << 12);
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);
		RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
		rela->type = R_ARM_REL32;

		opcode = (0xe3 << 24) | (0x4 << 20) | (rd->id << 12);
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

		opcode = (0xe7 << 24) | (0x9 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

		opcode = (0xe0 << 24) | (0x8 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

	} else {
		scf_loge("temp var should give a register\n");
		return -EINVAL;
	}

	return 0;
}

int arm32_inst_M2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t*       fp   = f->rops->find_register("fp");
	scf_register_t*       ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;

	int size = f->rops->variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			opcode = (0xe3 << 24) | (rd->id << 12);
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_ARM_REL32;

			opcode = (0xe3 << 24) | (0x4 << 20) | (rd->id << 12);
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode = (0xe7 << 24) | (0x9 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode = (0xe0 << 24) | (0x8 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

			rb = rd;

		} else {
			scf_loge("temp var should give a register\n");
			return -EINVAL;
		}

	} else {
		if (vs->local_flag || vs->tmp_flag)
			offset = vs->bp_offset;
		else
			offset = vs->offset;
	}

	if (1 == size) {

		if (scf_variable_signed(vs)) {

			if (offset >= 0 && offset < 0xff)
				opcode = (0xe1 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xd) << 4) | (offset & 0xf);

			else if (offset < 0 && offset >= -0xff) {
				offset = -offset;
				opcode = (0xe1 << 24) | (0x5 << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xd) << 4) | (offset & 0xf);

			} else {
				int ret = risc_select_free_reg(&ri, c, f, 0);
				if (ret < 0) {
					scf_loge("\n");
					return -EINVAL;
				}

				ret = arm32_inst_I2G(c, ri, offset, 4);
				if (ret < 0)
					return ret;

				opcode = (0xe1 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | (0xd << 4) | ri->id;
			}

		} else {
			if (offset >= 0 && offset < 0xfff)
				opcode = (0xe5 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | offset;

			else if (offset < 0 && offset >= -0xfff) {
				offset = -offset;
				opcode = (0xe5 << 24) | (0x5 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

			} else {
				int ret = risc_select_free_reg(&ri, c, f, 0);
				if (ret < 0) {
					scf_loge("\n");
					return -EINVAL;
				}

				ret = arm32_inst_I2G(c, ri, offset, 4);
				if (ret < 0)
					return ret;

				opcode = (0xe7 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | ri->id;
			}
		}

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xff)
			opcode = (0xe1 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		else if (offset < 0 && offset >= -0xff) {
			offset = -offset;
			opcode = (0xe1 << 24) | (0x5 << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe1 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | (0xb << 4) | ri->id;
		}

		if (scf_variable_signed(vs))
			opcode |= 0x1 << 6;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x1 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | ri->id;
		}

	} else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm32_inst_G2M(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t* fp   = f->rops->find_register("fp");
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;

	int size = f->rops->variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			int ret = risc_select_free_reg(&rb, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_ADR2G(c, f, rb, vs);
			if (ret < 0)
				return -EINVAL;

		} else {
			scf_loge("temp var should give a register\n");
			return -EINVAL;
		}

	} else {
		if (vs->local_flag || vs->tmp_flag)
			offset = vs->bp_offset;
		else
			offset = vs->offset;
	}

	if (1 == size) {

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x4 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | ri->id;
		}

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xff)
			opcode = (0xe1 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		else if (offset < 0 && offset >= -0xff) {
			offset = -offset;
			opcode = (0xe1 << 24) | (0x4 << 20) | (rb->id << 16) | (rs->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe1 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | (0xb << 4) | ri->id;
		}

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x0 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | ri->id;
		}

	} else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}
////////
int arm32_inst_ISTR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* v)
{
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int size1 = f->rops->variable_size(v);

	assert(4 == rd->bytes);
	assert(4 == size1);

	v->global_flag = 1;
	v->local_flag  = 0;
	v->tmp_flag    = 0;

	uint32_t opcode;

	opcode = (0xe3 << 24) | (rd->id << 12);
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	RISC_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	rela->type = R_ARM_REL32;

	opcode = (0xe3 << 24) | (0x4 << 20) | (rd->id << 12);
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	opcode = (0xe7 << 24) | (0x9 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	opcode = (0xe0 << 24) | (0x8 << 20) | (rd->id << 16) | (rd->id << 12) | 0xf;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm32_inst_G2P(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, int32_t offset, int size)
{
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode;

	if (!rb)
		return -EINVAL;

	if (1 == size) {

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x4 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | ri->id;
		}

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xff)
			opcode = (0xe1 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		else if (offset < 0 && offset >= -0xff) {
			offset = -offset;
			opcode = (0xe1 << 24) | (0x4 << 20) | (rb->id << 16) | (rs->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe1 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | (0xb << 4) | ri->id;
		}

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x0 << 20) | (rb->id << 16) | (rs->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | ri->id;
		}

	} else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm32_inst_P2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset, int size)
{
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	if (!rb)
		return -EINVAL;

	if (1 == size) {

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x5 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | ri->id;
		}

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xff)
			opcode = (0xe1 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		else if (offset < 0 && offset >= -0xff) {
			offset = -offset;
			opcode = (0xe1 << 24) | (0x5 << 20) | (rb->id << 16) | (rd->id << 12) | (((offset & 0xf0) | 0xb) << 4) | (offset & 0xf);

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe1 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | (0xb << 4) | ri->id;
		}

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		if (offset >= 0 && offset < 0xfff)
			opcode = (0xe5 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		else if (offset < 0 && offset >= -0xfff) {
			offset = -offset;
			opcode = (0xe5 << 24) | (0x1 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

		} else {
			int ret = risc_select_free_reg(&ri, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			ret = arm32_inst_I2G(c, ri, offset, 4);
			if (ret < 0)
				return ret;

			opcode = (0xe7 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | ri->id;
		}

	} else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm32_inst_ADRP2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset)
{
	scf_register_t* r    = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode = 0;

	if (offset >= 0 && offset <= 0xff)
		opcode = (0xe2 << 24) | (0x8 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

	else if (offset < 0 && offset >= -0xff) {
		offset = -offset;
		opcode = (0xe2 << 24) | (0x4 << 20) | (rb->id << 16) | (rd->id << 12) | offset;

	} else {
		int ret = risc_select_free_reg(&r, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm32_inst_I2G(c, r, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (0xe0 << 24) | (0x8 << 20) | (rb->id << 16) | (rd->id << 12) | r->id;
	}

	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm32_inst_ADRSIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib)
{
	scf_register_t* rb   = sib->base;
	scf_register_t* ri   = sib->index;
	scf_instruction_t*    inst = NULL;

	assert(0 == sib->disp);

	if (!rb || !ri)
		return -EINVAL;

	uint32_t opcode;
	uint32_t SH;

	if (1 == sib->scale)
		SH = 0;

	else if (2 == sib->scale)
		SH = 1;

	else if (4 == sib->scale)
		SH = 2;
	else
		return -EINVAL;

	opcode = (0xe0 << 24) | (0x8 << 20) | (rb->id << 16) | (rd->id << 12) | (SH << 7) | ri->id;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm32_inst_SIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib)
{
	scf_register_t*       rb   = sib->base;
	scf_register_t*       ri   = sib->index;
	scf_instruction_t*    inst = NULL;

	assert(0 == sib->disp);

	if (!rb || !ri)
		return -EINVAL;

	int scale  = sib->scale;
	int size   = sib->size;

	uint32_t opcode;

	if (1 == size)
		opcode = (0xe7 << 24) | (0xd << 20) | (rb->id << 16) | (rd->id << 12) | ri->id;

	else if (2 == size)
		opcode = (0xf8 << 24) | (0x3 << 20) | (rb->id << 16) | (rd->id << 12) | (0x1 << 4) | ri->id;

	else if (4 == size)
		opcode = (0xe7 << 24) | (0x9 << 20) | (rb->id << 16) | (rd->id << 12) | (0x2 << 7) | ri->id;
	else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm32_inst_G2SIB(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_sib_t* sib)
{
	scf_register_t* rb   = sib->base;
	scf_register_t* ri   = sib->index;
	scf_instruction_t*    inst = NULL;

	assert(0 == sib->disp);

	if (!rb || !ri)
		return -EINVAL;

	int scale  = sib->scale;
	int size   = sib->size;

	uint32_t opcode;

	if (1 == size)
		opcode = (0xe7 << 24) | (0xc << 20) | (rb->id << 16) | (rs->id << 12) | ri->id;

	else if (2 == size)
		opcode = (0xf8 << 24) | (0x2 << 20) | (rb->id << 16) | (rs->id << 12) | (0x1 << 4) | ri->id;

	else if (4 == size)
		opcode = (0xe7 << 24) | (0x8 << 20) | (rb->id << 16) | (rs->id << 12) | (0x2 << 7) | ri->id;
	else
		return -EINVAL;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm32_inst_M2GF(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t* fp   = f->rops->find_register("fp");
	scf_register_t* ro   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t D    = 0;
	uint32_t V    = 0;

	int size = f->rops->variable_size(vs);

	scf_loge("\n");
	return -EINVAL;

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			int ret = risc_select_free_reg(&rb, c, f, 0);
			if (ret < 0)
				return ret;

			opcode = (0xe3 << 24) | (rb->id << 12);
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_ARM_REL32;

			opcode = (0xe3 << 24) | (0x4 << 20) | (rb->id << 12);
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode = (0xe7 << 24) | (0x9 << 20) | (rb->id << 16) | (rb->id << 12) | 0xf;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode = (0xe0 << 24) | (0x8 << 20) | (rb->id << 16) | (rb->id << 12) | 0xf;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);

		} else {
			scf_loge("temp var should give a register\n");
			return -EINVAL;
		}

	} else {
		if (vs->local_flag || vs->tmp_flag)
			offset = vs->bp_offset;
		else
			offset = vs->offset;
	}


	if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >>= 1;
		SIZE   =   1;
		D      =   rd->id & 0x1;
		V      =   rd->id >> 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >>= 2;
		SIZE   =   2;
		D      =   rd->id & 0x1;
		V      =   rd->id >> 1;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >>= 3;
		SIZE   =   3;
		V      =   rd->id & 0xf;
		D      =  (rd->id >> 4) & 0x1;
	} else
		return -EINVAL;

	if (offset >= 0 && offset < 0xff)
		opcode = (0xed << 24) | (0x9 << 20) | (rb->id << 16) | (0x2 << 10) | (SIZE << 8) | offset;

	else if (offset < 0 && offset >= -0xff) {
		offset = -offset;
		opcode = (0xed << 24) | (0x1 << 20) | (rb->id << 16) | (0x2 << 10) | (SIZE << 8) | offset;

	} else {
		int ret = risc_select_free_reg(&ro, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm32_inst_I2G(c, ro, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (0xe0 << 24) | (0x8 << 20) | (rb->id << 16) | (ro->id << 12) | (SIZE << 7) | ro->id;
		inst    = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

		opcode = (0xed << 24) | (0x9 << 20) | (ro->id << 16) | (0x2 << 10) | (SIZE << 8) | 0;
	}

	opcode |= (D << 22) | (V << 12);
	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

scf_instruction_t* arm32_inst_PUSH(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe5 << 24) | (0x2 << 20) | (0xd << 16) | (r->id << 12) | 0x4;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_POP(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe4 << 24) | (0x9 << 20) | (0xd << 16) | (r->id << 12) | 0x4;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_RET(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xe12fff1e;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MOV_SP(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xa << 20) | (rd->id << 12) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MOV_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xa << 20) | (rd->id << 12) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MVN(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xe << 20) | (rd->id << 12) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FMOV_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xee << 24) | (0xe << 20) | (rd->id << 12) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MOVSX(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	if (1 == size)
		opcode = (0xe6 << 24) | (0xa << 20) | (0xf << 16) | (rd->id << 12) | (0x7 << 4) | rs->id;

	else if (2 == size)
		opcode = (0xe6 << 24) | (0xb << 20) | (0xf << 16) | (rd->id << 12) | (0x7 << 4) | rs->id;
	else
		return NULL;

	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MOVZX(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	if (1 == size)
		opcode = (0xe6 << 24) | (0xe << 20) | (0xf << 16) | (rd->id << 12) | (0x7 << 4) | rs->id;

	else if (2 == size)
		opcode = (0xe6 << 24) | (0xf << 20) | (0xf << 16) | (rd->id << 12) | (0x7 << 4) | rs->id;
	else
		return NULL;

	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CVTSS2SD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           M  = rs->id & 0x1;
	uint32_t           Vm = rs->id >>  1;
	uint32_t           D  = rd->id >>  4;
	uint32_t           Vd = rd->id & 0xf;

	opcode = (0xee << 24) | (0xb7 << 16) | (0xa << 8) | (0xc << 4) | (D << 22) | (Vd << 12) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CVTSD2SS(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           M  = rd->id >>  4;
	uint32_t           Vm = rd->id & 0xf;
	uint32_t           D  = rs->id & 0x1;
	uint32_t           Vd = rs->id >>  1;

	opcode = (0xee << 24) | (0xb7 << 16) | (0xb << 8) | (0xc << 4) | (D << 22) | (Vd << 12) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CVTF2SI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           M  = rs->id & 0x1;
	uint32_t           Vm = rs->id >>  1;
	uint32_t           D  = 15 >> 4;
	uint32_t           Vd = 15 & 0xf;
	uint32_t           SZ = (8 == rs->bytes) ? 0x3 : 0x2;

	opcode  = (0xee << 24) | (0xb << 20) | (D << 22) | (0xd << 16) | (Vd << 12) | (0x2 << 10) | (SZ << 8) | (0x3 << 6) | (M << 5) | Vm;
	inst    = risc_make_inst(c, opcode);
	if (!inst)
		return NULL;

	if (scf_vector_add(c->instructions, inst) < 0) {
		free(inst);
		return NULL;
	}

	opcode = (0xee << 24) | (0x1 << 20) | (Vd << 16) | (rd->id << 12) | (0xa << 8) | (D << 7) | (0x1 << 4);
	inst   = risc_make_inst(c, opcode);
	return inst;
}

scf_instruction_t* arm32_inst_CVTF2UI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           M  = rs->id & 0x1;
	uint32_t           Vm = rs->id >>  1;
	uint32_t           D  = 15 >> 4;
	uint32_t           Vd = 15 & 0xf;
	uint32_t           SZ = (8 == rs->bytes) ? 0x3 : 0x2;

	opcode = (0xee << 24) | (0xb << 20) | (D << 22) | (0xc << 16) | (Vd << 12) | (0x2 << 10) | (SZ << 8) | (0x3 << 6) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);
	if (!inst)
		return NULL;

	if (scf_vector_add(c->instructions, inst) < 0) {
		free(inst);
		return NULL;
	}

	opcode = (0xee << 24) | (0x1 << 20) | (Vd << 16) | (rd->id << 12) | (0xa << 8) | (D << 7) | (0x1 << 4);
	inst   = risc_make_inst(c, opcode);
	return inst;
}

scf_instruction_t* arm32_inst_CVTSI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           D  = rs->id & 0x1;
	uint32_t           Vd = rs->id >>  1;
	uint32_t           M  = 15 >> 4;
	uint32_t           Vm = 15 & 0xf;
	uint32_t           SZ = (8 == rd->bytes) ? 0x3 : 0x2;

	opcode = (0xee << 24) | (0x0 << 20) | (Vm << 16) | (rd->id << 12) | (0xa << 8) | (M << 7) | (0x1 << 4);
	inst   = risc_make_inst(c, opcode);
	if (!inst)
		return NULL;

	if (scf_vector_add(c->instructions, inst) < 0) {
		free(inst);
		return NULL;
	}

	opcode = (0xee << 24) | (0xb << 20) | (D << 22) | (0x8 << 16) | (Vd << 12) | (0x2 << 10) | (SZ << 8) | (0x3 << 6) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CVTUI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	uint32_t           D  = rs->id & 0x1;
	uint32_t           Vd = rs->id >>  1;
	uint32_t           M  = 15 >> 4;
	uint32_t           Vm = 15 & 0xf;
	uint32_t           SZ = (8 == rd->bytes) ? 0x3 : 0x2;

	opcode = (0xee << 24) | (0x0 << 20) | (Vm << 16) | (rd->id << 12) | (0xa << 8) | (M << 7) | (0x1 << 4);
	inst   = risc_make_inst(c, opcode);
	if (!inst)
		return NULL;

	if (scf_vector_add(c->instructions, inst) < 0) {
		free(inst);
		return NULL;
	}

	opcode = (0xee << 24) | (0xb << 20) | (D << 22) | (0x8 << 16) | (Vd << 12) | (0x2 << 10) | (SZ << 8) | (0x1 << 6) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SUB_IMM(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xff) {
		scf_loge("NOT support too big imm: %#lx\n", imm);
		return NULL;
	}

	opcode = (0xe2 << 24) | (0x4 << 20) | (rs->id << 16) | (rd->id << 12) | imm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CMP_IMM(scf_3ac_code_t* c, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xff) {
		scf_loge("NOT support too big imm: %#lx\n", imm);
		return NULL;
	}

	opcode = (0xe3 << 24) | (0x5 << 20) | (rs->id << 16) | imm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_ADD_IMM(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xff) {
		scf_loge("NOT support too big imm: %#lx\n", imm);
		return NULL;
	}

	opcode = (0xe2 << 24) | (0x8 << 20) | (rs->id << 16) | (rd->id << 12) | imm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_ADD_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe0 << 24) | (0x8 << 20) | (rs0->id << 16) | (rd->id << 12) | rs1->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SHL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xa << 20) | (rd->id << 12) | (rs1->id << 8) | (0x1 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SHR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xa << 20) | (rd->id << 12) | (rs1->id << 8) | (0x3 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_ASR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0xa << 20) | (rd->id << 12) | (rs1->id << 8) | (0x5 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_AND_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe0 << 24) | (0x0 << 20) | (rs0->id << 16) | (rd->id << 12) | rs1->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_OR_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0x8 << 20) | (rs0->id << 16) | (rd->id << 12) | rs1->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SUB_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe0 << 24) | (0x4 << 20) | (rs0->id << 16) | (rd->id << 12) | rs1->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_CMP_G(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0x5 << 20) | (rs0->id << 16) | rs1->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FCMP(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           D;
	uint32_t           M;
	uint32_t           Vd;
	uint32_t           Vm;
	uint32_t           SIZE;

	if (8 == rs0->bytes) {
		D  = rs0->id >> 4;
		Vd = rs0->id & 0xf;

		M  = rs1->id >> 4;
		Vm = rs1->id & 0xf;

		SIZE = 3;

	} else {
		D  = rs0->id & 0x1;
		Vd = rs0->id >> 1;

		M  = rs1->id & 0x1;
		Vm = rs1->id >> 1;

		SIZE = 2;
	}

	opcode = (0xee << 24) | (0x1 << 23) | (D << 22) | (0x3 << 20) | (0x4 << 16) | (Vd << 12) | (0x2 << 10) | (SIZE << 8) | (0x1 << 6) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_NEG(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0x3 << 20) | (rs->id << 16) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_TEQ(scf_3ac_code_t* c, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe1 << 24) | (0x3 << 20) | (rs->id << 16) | rs->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FADD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           D;
	uint32_t           M;
	uint32_t           N;
	uint32_t           Vd;
	uint32_t           Vm;
	uint32_t           Vn;
	uint32_t           SIZE;

	if (8 == rd->bytes) {
		D  = rd->id >> 4;
		Vd = rd->id & 0xf;

		M  = rs1->id >> 4;
		Vm = rs1->id & 0xf;

		N  = rs0->id >> 4;
		Vn = rs0->id & 0xf;

		SIZE = 3;

	} else {
		D  = rd->id & 0x1;
		Vd = rd->id >> 1;

		M  = rs1->id & 0x1;
		Vm = rs1->id >> 1;

		N  = rs0->id & 0x1;
		Vn = rs0->id >> 1;

		SIZE = 2;
	}

	opcode = (0xee << 24) | (D << 22) | (0x3 << 20) | (Vn << 16) | (Vd << 12) | (0x2 << 10) | (SIZE << 8) | (N << 7) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FSUB(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           D;
	uint32_t           M;
	uint32_t           N;
	uint32_t           Vd;
	uint32_t           Vm;
	uint32_t           Vn;
	uint32_t           SIZE;

	if (8 == rd->bytes) {
		D  = rd->id >> 4;
		Vd = rd->id & 0xf;

		M  = rs1->id >> 4;
		Vm = rs1->id & 0xf;

		N  = rs0->id >> 4;
		Vn = rs0->id & 0xf;

		SIZE = 3;

	} else {
		D  = rd->id & 0x1;
		Vd = rd->id >> 1;

		M  = rs1->id & 0x1;
		Vm = rs1->id >> 1;

		N  = rs0->id & 0x1;
		Vn = rs0->id >> 1;

		SIZE = 2;
	}

	opcode = (0xee << 24) | (D << 22) | (0x3 << 20) | (Vn << 16) | (Vd << 12) | (0x2 << 10) | (SIZE << 8) | (N << 7) | (0x1 << 6) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MUL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe0 << 24) | (rd->id << 16) | (rs1->id << 8) | (0x9 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FMUL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           D;
	uint32_t           M;
	uint32_t           N;
	uint32_t           Vd;
	uint32_t           Vm;
	uint32_t           Vn;
	uint32_t           SIZE;

	if (8 == rd->bytes) {
		D  = rd->id >> 4;
		Vd = rd->id & 0xf;

		M  = rs1->id >> 4;
		Vm = rs1->id & 0xf;

		N  = rs0->id >> 4;
		Vn = rs0->id & 0xf;

		SIZE = 3;

	} else {
		D  = rd->id & 0x1;
		Vd = rd->id >> 1;

		M  = rs1->id & 0x1;
		Vm = rs1->id >> 1;

		N  = rs0->id & 0x1;
		Vn = rs0->id >> 1;

		SIZE = 2;
	}

	opcode = (0xee << 24) | (0x0 << 23) | (D << 22) | (0x1 << 21) | (Vn << 16) | (Vd << 12) | (0x2 << 10) | (SIZE << 8) | (N << 7) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_FDIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           D;
	uint32_t           M;
	uint32_t           N;
	uint32_t           Vd;
	uint32_t           Vm;
	uint32_t           Vn;
	uint32_t           SIZE;

	if (8 == rd->bytes) {
		D  = rd->id >> 4;
		Vd = rd->id & 0xf;

		M  = rs1->id >> 4;
		Vm = rs1->id & 0xf;

		N  = rs0->id >> 4;
		Vn = rs0->id & 0xf;

		SIZE = 3;

	} else {
		D  = rd->id & 0x1;
		Vd = rd->id >> 1;

		M  = rs1->id & 0x1;
		Vm = rs1->id >> 1;

		N  = rs0->id & 0x1;
		Vn = rs0->id >> 1;

		SIZE = 2;
	}

	opcode = (0xee << 24) | (0x1 << 23) | (D << 22) | (Vn << 16) | (Vd << 12) | (0x2 << 10) | (SIZE << 8) | (N << 7) | (M << 5) | Vm;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_DIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe7 << 24) | (0x3 << 20) | (rd->id << 16) | (0xf << 12) | (rs1->id << 8) | (0x1 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SDIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe7 << 24) | (0x1 << 20) | (rd->id << 16) | (0xf << 12) | (rs1->id << 8) | (0x1 << 4) | rs0->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_MSUB(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rm, scf_register_t* rn, scf_register_t* ra)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xe0 << 24) | (0x6 << 20) | (rd->id << 16) | (ra->id << 12) | (rm->id << 8) | (0x9 << 4) | rn->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

int arm32_inst_BL(scf_3ac_code_t* c, scf_function_t* f, scf_function_t* pf)
{
	scf_instruction_t* inst;
	scf_rela_t*        rela;
	uint32_t           opcode;

	opcode = 0xeb << 24;
	inst   = risc_make_inst(c, opcode);

	RISC_INST_ADD_CHECK(c->instructions, inst);
	RISC_RELA_ADD_CHECK(f->text_relas,   rela, c, NULL, pf);

	rela->type = R_ARM_CALL;
	return 0;
}

scf_instruction_t* arm32_inst_BLR(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xe12fff30 | r->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_SETZ(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 1;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm32_inst_SETNZ(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm32_inst_SETGT(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xd;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm32_inst_SETGE(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xb;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm32_inst_SETLT(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xa;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm32_inst_SETLE(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xc;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JMP(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xea000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JZ(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x0a000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JNZ(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x1a000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JGT(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xca000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JGE(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xaa000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JLT(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xba000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JLE(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xda000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm32_inst_JA(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm32_inst_JB(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm32_inst_JAE(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm32_inst_JBE(scf_3ac_code_t* c)
{
	return NULL;
}

void arm32_set_jmp_offset(scf_instruction_t* inst, int32_t bytes)
{
	bytes  -= 8; // 'pc = current + 8'
	bytes >>= 2;

	assert(bytes < 0x7fffff && bytes > -0x7fffff);

	inst->code[0] |= 0xff &  bytes;
	inst->code[1] |= 0xff & (bytes >>  8);
	inst->code[2] |= 0xff & (bytes >> 16);
}

int arm32_cmp_update(scf_3ac_code_t* c, scf_function_t* f, scf_instruction_t* cmp)
{
	scf_instruction_t* inst;
	scf_register_t*    r12 = f->rops->find_register_type_id_bytes(0, 12, 4);
	scf_register_t*    r14 = f->rops->find_register_type_id_bytes(0, 14, 4);
	scf_register_t*    r0;

	uint32_t opcode;
	uint32_t mov;
	uint32_t i0;
	uint32_t i1;

	opcode  = cmp->code[0];
	opcode |= cmp->code[1] <<  8;
	opcode |= cmp->code[2] << 16;
	opcode |= cmp->code[3] << 24;

	switch (cmp->code[3]) {
		// arm32
		case 0xe3:  // imm
			i0   = (opcode >> 16) & 0xf;
			r0   = f->rops->find_register_type_id_bytes(0, i0, 4);
			inst = f->iops->MOV_G(c, r12, r0);  // use r12 to backup r0
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode &= ~(0xf << 16);
			opcode |=  (0xc << 16);
			break;

		case 0xe1:  // register
			i0   = (opcode >> 16) & 0xf;
			i1   = (opcode      ) & 0xf;

			r0   = f->rops->find_register_type_id_bytes(0, i0, 4);
			inst = f->iops->MOV_G(c, r12, r0);  // use r16 to backup r0
			RISC_INST_ADD_CHECK(c->instructions, inst);

			r0   = f->rops->find_register_type_id_bytes(0, i1, 4);
			inst = f->iops->MOV_G(c, r14, r0);  // use r17 to backup r1
			RISC_INST_ADD_CHECK(c->instructions, inst);

			opcode &= ~(0xf << 16);
			opcode |=  (0xc << 16);

			opcode &= ~0xf;
			opcode |=  0xe;
			break;
		default:
			scf_loge("%#x\n", opcode);
			return -EINVAL;
			break;
	};

	cmp->code[0] = 0xff &  opcode;
	cmp->code[1] = 0xff & (opcode >>  8);
	cmp->code[2] = 0xff & (opcode >> 16);
	cmp->code[3] = 0xff & (opcode >> 24);
	return 0;
}

int arm32_set_rel_veneer(scf_function_t* f)
{
	scf_basic_block_t* bb;
	scf_instruction_t* inst;
	scf_3ac_code_t*    c;
	scf_3ac_code_t*    end;
	scf_list_t*        l;
	scf_rela_t*        r;

	int bytes = f->init_code_bytes;

	for (l = scf_list_head(&f->basic_block_list_head); l != scf_list_sentinel(&f->basic_block_list_head); l = scf_list_next(l)) {
		bb = scf_list_data(l, scf_basic_block_t, list);

		bytes += bb->code_bytes;
	}

	l   = scf_list_tail(&bb->code_list_head);
	end = scf_list_data(l, scf_3ac_code_t, list);

	int i;
	for (i = 0; i < f->data_relas->size; i++) {
		r  =        f->data_relas->data[i];

		int offset = bytes - r->inst_offset - 16; // 'pc = current + 8'

		inst = risc_make_inst(end, offset - 4);
		RISC_INST_ADD_CHECK(end->instructions, inst);

		r->inst_offset   = bytes;

		bytes           += 4;
		end->inst_bytes += 4;
		bb ->code_bytes += 4;

		r->inst->code[0]  = 0xff & offset;
		r->inst->code[1] &= 0xf0;
		r->inst->code[1] |= 0x0f & (offset >> 8);
		r->inst->code[2] &= 0xf0;
		r->inst->code[2] |= 0x0f & (offset >> 12);

		int j;
		for (j = 0; j < r->code->instructions->size; j++) {
			inst      = r->code->instructions->data[j];

			if (inst == r->inst)
				break;
		}

		assert(r->code->instructions->size > j + 1);

		inst = r->code->instructions->data[j + 1];

		inst->code[0]  = 0xff & (offset >> 16);
		inst->code[1] &= 0xf0;
		inst->code[1] |= 0x0f & (offset >> 24);
		inst->code[2] &= 0xf0;
		inst->code[2] |= 0x0f & (offset >> 28);
	}

	return 0;
}

scf_inst_ops_t  inst_ops_arm32 =
{
	.name      = "arm32",

	.BL        = arm32_inst_BL,
	.BLR       = arm32_inst_BLR,
	.PUSH      = arm32_inst_PUSH,
	.POP       = arm32_inst_POP,
	.TEQ       = arm32_inst_TEQ,
	.NEG       = arm32_inst_NEG,

	.MOVZX     = arm32_inst_MOVZX,
	.MOVSX     = arm32_inst_MOVSX,
	.MVN       = arm32_inst_MVN,
	.MOV_G     = arm32_inst_MOV_G,
	.MOV_SP    = arm32_inst_MOV_SP,

	.ADD_G     = arm32_inst_ADD_G,
	.ADD_IMM   = arm32_inst_ADD_IMM,
	.SUB_G     = arm32_inst_SUB_G,
	.SUB_IMM   = arm32_inst_SUB_IMM,
	.CMP_G     = arm32_inst_CMP_G,
	.CMP_IMM   = arm32_inst_CMP_IMM,
	.AND_G     = arm32_inst_AND_G,
	.OR_G      = arm32_inst_OR_G,

	.MUL       = arm32_inst_MUL,
	.DIV       = arm32_inst_DIV,
	.SDIV      = arm32_inst_SDIV,
	.MSUB      = arm32_inst_MSUB,

	.SHL       = arm32_inst_SHL,
	.SHR       = arm32_inst_SHR,
	.ASR       = arm32_inst_ASR,

	.CVTSS2SD  = arm32_inst_CVTSS2SD,
	.CVTSD2SS  = arm32_inst_CVTSD2SS,
	.CVTF2SI   = arm32_inst_CVTF2SI,
	.CVTF2UI   = arm32_inst_CVTF2UI,
	.CVTSI2F   = arm32_inst_CVTSI2F,
	.CVTUI2F   = arm32_inst_CVTUI2F,

	.FCMP      = arm32_inst_FCMP,
	.FADD      = arm32_inst_FADD,
	.FSUB      = arm32_inst_FSUB,
	.FMUL      = arm32_inst_FMUL,
	.FDIV      = arm32_inst_FDIV,
	.FMOV_G    = arm32_inst_FMOV_G,

	.JA        = arm32_inst_JA,
	.JB        = arm32_inst_JB,
	.JZ        = arm32_inst_JZ,
	.JNZ       = arm32_inst_JNZ,
	.JGT       = arm32_inst_JGT,
	.JGE       = arm32_inst_JGE,
	.JLT       = arm32_inst_JLT,
	.JLE       = arm32_inst_JLE,
	.JAE       = arm32_inst_JAE,
	.JBE       = arm32_inst_JBE,
	.JMP       = arm32_inst_JMP,
	.RET       = arm32_inst_RET,

	.SETZ      = arm32_inst_SETZ,
	.SETNZ     = arm32_inst_SETNZ,
	.SETGT     = arm32_inst_SETGT,
	.SETGE     = arm32_inst_SETGE,
	.SETLT     = arm32_inst_SETLT,
	.SETLE     = arm32_inst_SETLE,

	.I2G       = arm32_inst_I2G,
	.M2G       = arm32_inst_M2G,
	.M2GF      = arm32_inst_M2GF,
	.G2M       = arm32_inst_G2M,
	.G2P       = arm32_inst_G2P,
	.P2G       = arm32_inst_P2G,
	.ISTR2G    = arm32_inst_ISTR2G,
	.SIB2G     = arm32_inst_SIB2G,
	.G2SIB     = arm32_inst_G2SIB,
	.ADR2G     = arm32_inst_ADR2G,
	.ADRP2G    = arm32_inst_ADRP2G,
	.ADRSIB2G  = arm32_inst_ADRSIB2G,

	.cmp_update     = arm32_cmp_update,
	.set_jmp_offset = arm32_set_jmp_offset,
	.set_rel_veneer = arm32_set_rel_veneer,
};

