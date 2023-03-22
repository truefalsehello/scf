#include"scf_risc.h"

int arm64_inst_I2G(scf_3ac_code_t* c, scf_register_t* rd, uint64_t imm, int bytes)
{
	scf_instruction_t* inst;

	uint64_t invert = ~imm;
	uint32_t opcode;

	if (0 == (invert >> 32)) {

		// movn rd, invert[15:0]
		opcode = (0x92 << 24) | (0x1 << 23) | ((invert & 0xffff) << 5) | rd->id;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

		if (invert >> 16) {
			// movk rd, imm[31:16]
			opcode = (0xf2 << 24) | (0x1 << 23) | (0x1 << 21) | (((imm >> 16) & 0xffff) << 5) | rd->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}

		return 0;
	}

	// mov rd, imm[15:0]
	opcode = (0xd2 << 24) | (0x1 << 23) | ((imm & 0xffff) << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	if ((imm >> 16) & 0xffff) {

		// movk rd, imm[31:16]
		opcode = (0xf2 << 24) | (0x1 << 23) | (0x1 << 21) | (((imm >> 16) & 0xffff) << 5) | rd->id;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);
	}

	if (8 == bytes) {

		if ((imm >> 32) & 0xffff) {

			// movk rd, imm[47:32]
			opcode = (0xf2 << 24) | (0x1 << 23) | (0x2 << 21) | (((imm >> 32) & 0xffff) << 5) | rd->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}

		if ((imm >> 48) & 0xffff) {

			// movk rd, imm[63:48]
			opcode = (0xf2 << 24) | (0x1 << 23) | (0x3 << 21) | (((imm >> 48) & 0xffff) << 5) | rd->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}
	}

	return 0;
}

int arm64_inst_ADR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* vs)
{
	scf_register_t* fp   = risc_find_register("fp");
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int64_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = risc_variable_size(vs);

	if (vs->local_flag || vs->tmp_flag) {

		offset = vs->bp_offset;

		if (offset >= 0 && offset <= 0xfff)

			opcode = (0x91 << 24) | (offset << 10) | (fp->id << 5) | rd->id;

		else if (offset < 0 && -offset <= 0xfff)

			opcode = (0xd1 << 24) | ((-offset) << 10) | (fp->id << 5) | rd->id;

		else {
			int ret = arm64_inst_I2G(c, rd, offset, 8);
			if (ret < 0)
				return ret;

			opcode = (0x8b << 24) | (fp->id << 16) | (rd->id << 5) | rd->id;
		}

		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);

	} else if (vs->global_flag) {
		offset = 0;

		opcode = (0x90 << 24) | rd->id;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);
		RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
		rela->type = R_AARCH64_ADR_PREL_PG_HI21;

		opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
		inst   = risc_make_inst(c, opcode);
		RISC_INST_ADD_CHECK(c->instructions, inst);
		RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
		rela->type = R_AARCH64_ADD_ABS_LO12_NC;

	} else {
		scf_loge("temp var should give a register\n");
		return -EINVAL;
	}

	return 0;
}

int arm64_inst_M2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t* fp   = risc_find_register("fp");
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = risc_variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			opcode = (0x90 << 24) | rd->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADR_PREL_PG_HI21;

			opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADD_ABS_LO12_NC;

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
		S = 0;
		SIZE = 0;

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 1;
		SIZE = 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 2;
		SIZE = 2;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 3;
		SIZE = 3;
	} else
		return -EINVAL;


	if (offset >= 0 && offset < 0xfff)
		opcode = (0x39 << 24) | (offset << 10);

	else if (offset < 0 && offset >= -0xff)
		opcode = (0x38 << 24) | ((offset & 0x1ff) << 12);

	else {
		int ret = risc_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		ret = arm64_inst_I2G(c, ri, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10);
	}

	if (rd->bytes > size && scf_variable_signed(vs))
		opcode |= 0x2 << 22;
	else
		opcode |= 0x1 << 22;

	opcode |= (SIZE << 30) | (rb->id << 5) | rd->id;

	opcode |= RISC_COLOR_TYPE(rd->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_inst_G2M(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t* fp   = risc_find_register("fp");
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = risc_variable_size(vs);

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

			ret = arm64_inst_ADR2G(c, f, rb, vs);
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
		S = 0;
		SIZE = 0;

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 1;
		SIZE = 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 2;
		SIZE = 2;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 3;
		SIZE = 3;

	} else
		return -EINVAL;

	if (offset >= 0 && offset < 0xfff)

		opcode = (SIZE << 30) | (0x39 << 24) | (offset << 10) | (rb->id << 5) | rs->id;

	else if (offset < 0 && offset >= -0xff)

		opcode = (SIZE << 30) | (0x38 << 24) | ((offset & 0x1ff) << 12) | (rb->id << 5) | rs->id;

	else {
		int ret = risc_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		ret = arm64_inst_I2G(c, ri, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rs->id;
	}

	opcode |= RISC_COLOR_TYPE(rs->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_inst_ISTR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_variable_t* v)
{
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int size1 = risc_variable_size(v);

	assert(8 == rd->bytes);
	assert(8 == size1);

	v->global_flag = 1;
	v->local_flag  = 0;
	v->tmp_flag    = 0;

	uint32_t opcode;

	opcode = (0x90 << 24) | rd->id;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	RISC_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	rela->type = R_AARCH64_ADR_PREL_PG_HI21;

	opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	RISC_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	rela->type = R_AARCH64_ADD_ABS_LO12_NC;

	return 0;
}

int arm64_inst_G2P(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_register_t* rb, int32_t offset, int size)
{
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	if (!rb)
		return -EINVAL;

	if (1 == size) {
		S = 0;
		SIZE = 0;

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 1;
		SIZE = 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 2;
		SIZE = 2;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 3;
		SIZE = 3;

	} else
		return -EINVAL;

	if (offset >= 0 && offset < 0xfff)

		opcode = (SIZE << 30) | (0x39 << 24) | (offset << 10) | (rb->id << 5) | rs->id;

	else if (offset < 0 && offset >= -0xff)

		opcode = (SIZE << 30) | (0x38 << 24) | ((offset & 0x1ff) << 12) | (rb->id << 5) | rs->id;

	else {
		int ret = risc_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		ret = arm64_inst_I2G(c, ri, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rs->id;
	}

	opcode |= RISC_COLOR_TYPE(rs->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_inst_P2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset, int size)
{
	scf_register_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	if (!rb)
		return -EINVAL;

	if (1 == size) {
		S = 0;
		SIZE = 0;

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 1;
		SIZE = 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 2;
		SIZE = 2;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 3;
		SIZE = 3;

	} else
		return -EINVAL;

	if (offset >= 0 && offset < 0xfff)

		opcode = (SIZE << 30) | (0x39 << 24) | (0x1 << 22) | (offset << 10) | (rb->id << 5) | rd->id;

	else if (offset < 0 && offset >= -0xff)

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 22) | ((offset & 0x1ff) << 12) | (rb->id << 5) | rd->id;

	else {
		int ret = risc_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		ret = arm64_inst_I2G(c, ri, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 22) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rd->id;
	}

	opcode |= RISC_COLOR_TYPE(rd->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_inst_ADRP2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, int32_t offset)
{
	scf_register_t* r    = NULL;
	scf_instruction_t*    inst = NULL;

	uint32_t opcode = 0;
	uint32_t u24i   = 0x91;
	uint32_t u24r   = 0x8b;

	if (offset < 0) {
		offset = -offset;
		u24i   = 0xd1;
		u24r   = 0xcb;
	}

	if (offset <= 0xfff)
		opcode = (u24i << 24) | (offset << 10) | (rd->id << 5) | rd->id;

	else if (0 == (offset & 0xfff) && (offset >> 12) <= 0xfff)

		opcode = (u24i << 24) | (0x1 << 22) | ((offset >> 12) << 10) | (rd->id << 5) | rd->id;

	else {
		int ret = risc_select_free_reg(&r, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_inst_I2G(c, r, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (u24r << 24) | (r->id << 16) | (rd->id << 5) | rd->id;
	}

	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_inst_ADRSIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib)
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

	else if (8 == sib->scale)
		SH = 3;
	else
		return -EINVAL;

	opcode = (0x8b << 24) | (ri->id << 16) | (SH << 10) | (rb->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_inst_SIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_sib_t* sib)
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
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	if (1 == scale)
		S  = 0;


	if (1 == size)
		SIZE = 0;

	else if (2 == size)
		SIZE = 1;

	else if (4 == size)
		SIZE = 2;

	else if (8 == size)
		SIZE = 3;
	else
		return -EINVAL;

	opcode  = (SIZE << 30) | (0x38 << 24) | (0x1 << 22) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rd->id;

	opcode |= RISC_COLOR_TYPE(rd->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_inst_G2SIB(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rs, scf_sib_t* sib)
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
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	if (1 == scale)
		S  = 0;


	if (1 == size)
		SIZE = 0;

	else if (2 == size)
		SIZE = 1;

	else if (4 == size)
		SIZE = 2;

	else if (8 == size)
		SIZE = 3;
	else
		return -EINVAL;

	opcode  = (SIZE << 30) | (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rs->id;

	opcode |= RISC_COLOR_TYPE(rs->color) << 26;

	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_inst_M2GF(scf_3ac_code_t* c, scf_function_t* f, scf_register_t* rd, scf_register_t* rb, scf_variable_t* vs)
{
	scf_register_t* fp   = risc_find_register("fp");
	scf_register_t* ro   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = risc_variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			int ret = risc_select_free_reg(&rb, c, f, 0);
			if (ret < 0)
				return ret;

			opcode = (0x90 << 24) | rb->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADR_PREL_PG_HI21;

			opcode = (0x91 << 24) | (rb->id << 5) | rb->id;
			inst   = risc_make_inst(c, opcode);
			RISC_INST_ADD_CHECK(c->instructions, inst);
			RISC_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADD_ABS_LO12_NC;

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
		S = 0;
		SIZE = 0;

	} else if (2 == size) {

		if (offset & 0x1) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 1;
		SIZE = 1;

	} else if (4 == size) {

		if (offset & 0x3) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 2;
		SIZE = 2;

	} else if (8 == size) {

		if (offset & 0x7) {
			scf_loge("memory align error\n");
			return -EINVAL;
		}

		offset >> 3;
		SIZE = 3;
	} else
		return -EINVAL;


	if (offset >= 0 && offset < 0xfff)
		opcode = (0x3d << 24) | (0x1 << 22) | (offset << 10);

	else if (offset < 0 && offset >= -0xff)
		opcode = (0x3c << 24) | (0x1 << 22) | ((offset & 0x1ff) << 12);

	else {
		int ret = risc_select_free_reg(&ro, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_inst_I2G(c, ro, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (0x3c << 24) | (0x1 << 22) | (0x1 << 21) | (ro->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10);
	}

	opcode |= (SIZE << 30) | (rb->id << 5) | rd->id;
	inst    = risc_make_inst(c, opcode);
	RISC_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

scf_instruction_t* arm64_inst_PUSH(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xf8 << 24) | (0x1f8 << 12) | (0x3 << 10) | (0x1f << 5) | r->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_POP(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xf8 << 24) | (0x1 << 22) | (0x8 << 12) | (0x1 << 10) | (0x1f << 5) | r->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_RET(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0xd65f03c0;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MOV_SP(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x91 << 24) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MOV_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xaa << 24) | (rs->id << 16) | (0x1f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MVN(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xaa << 24) | (0x1 << 21) | (rs->id << 16) | (0x1f << 10) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FMOV_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x1e << 24) | ((8 == rd->bytes) << 22) | (0x1 << 21) | (0x1 << 14) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MOVSX(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	if (1 == size)
		S =  0x7;
	else if (2 == size)
		S = 0xf;
	else if (4 == size)
		S = 0x1f;
	else
		return NULL;

	opcode = (0x93 << 24) | (0x1 << 22) | (S << 10) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MOVZX(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, int size)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	if (1 == size)
		S =  0x7;
	else if (2 == size)
		S = 0xf;
	else
		return NULL;

	opcode = (0x53 << 24) | (S << 10) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTSS2SD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (0x3 << 14) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTSD2SS(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode = (0x1e << 24) | (0x1 << 22) | (0x1 << 21) | (0x1 << 17) | (0x1 << 14) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTF2SI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode  = ((8 == rd->bytes) << 31) | (0x1e << 24) | ((8 == rs->bytes) << 22) | (0x7 << 19) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTF2UI(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode  = ((8 == rd->bytes) << 31) | (0x1e << 24) | ((8 == rs->bytes) << 22) | (0x7 << 19) | (rs->id << 5) | rd->id;
	opcode |= 1 << 16;

	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTSI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode  = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (rs->id << 5) | rd->id;
	opcode |= ((8 == rs->bytes) << 31) | ((8 == rd->bytes) << 22);

	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CVTUI2F(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           S;

	opcode  = (0x1e << 24) | (0x1 << 21) | (0x1 << 17) | (rs->id << 5) | rd->id;
	opcode |= ((8 == rs->bytes) << 31) | ((8 == rd->bytes) << 22);
	opcode |= 1 << 16;

	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SUB_IMM(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xfff) {

		if (0 == (imm & 0xfff) && (imm >> 12) <= 0xfff) {
			imm >>= 12;
			sh = 1;
		} else {
			scf_loge("NOT support too big imm: %#lx\n", imm);
			return NULL;
		}
	}

	opcode  = (0x51 << 24) | (sh << 22) | (imm << 10) | (rs->id << 5) | rd->id;
	opcode |= (8 == rd->bytes) << 31;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CMP_IMM(scf_3ac_code_t* c, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xfff) {

		if (0 == (imm & 0xfff) && (imm >> 12) <= 0xfff) {
			imm >>= 12;
			sh = 1;
		} else {
			scf_loge("NOT support too big imm: %#lx\n", imm);
			return NULL;
		}
	}

	opcode  = (0x71 << 24) | (sh << 22) | (imm << 10) | (rs->id << 5) | 0x1f;
	opcode |= (8 == rs->bytes) << 31;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_ADD_IMM(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs, uint64_t imm)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           sh = 0;

	if (imm > 0xfff) {

		if (0 == (imm & 0xfff) && (imm >> 12) <= 0xfff) {
			imm >>= 12;
			sh = 1;
		} else {
			scf_loge("NOT support too big imm: %#lx\n", imm);
			return NULL;
		}
	}

	opcode = (0x91 << 24) | (sh << 22) | (imm << 10) | (rs->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_ADD_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x8b << 24) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SHL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x9a << 24) | (0x3 << 22) | (0x1 << 13) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SHR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x9a << 24) | (0x3 << 22) | (0x1 << 13) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	opcode |= 0x1 << 10;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_ASR(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x9a << 24) | (0x3 << 22) | (0x1 << 13) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	opcode |= 0x1 << 11;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_AND_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x8a << 24) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_OR_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xaa << 24) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SUB_G(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rs0->bytes) << 31) | (0x4b << 24) | (rs1->id << 16) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_CMP_G(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rs0->bytes) << 31) | (0x6b << 24) | (rs1->id << 16) | (rs0->id << 5) | 0x1f;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FCMP(scf_3ac_code_t* c, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x1e   << 24)  | (0x1    << 21) | (0x1 << 13);
	opcode |= (rs1->id << 16) | (rs0->id << 5);
	opcode |= ((8 == rs0->bytes) << 22);
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_NEG(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rd->bytes) << 31) | (0x4b << 24) | (rs->id << 16) | (0x1f << 10) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_TEQ(scf_3ac_code_t* c, scf_register_t* rs)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rs->bytes) << 31) | (0x6a << 24) | (rs->id << 16) | (rs->id << 5) | 0x1f;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FADD(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x1e    << 24) | (0x1     << 21) | (0x1 << 13) | (0x1 << 11);
	opcode |= (rs1->id << 16) | (rs0->id <<  5) | rd->id;
	opcode |= (8 == rd->bytes) << 22;
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FSUB(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x1e    << 24) | (0x1     << 21) | (0x7 << 11);
	opcode |= (rs1->id << 16) | (rs0->id << 5)  | rd->id;
	opcode |= ((8 == rd->bytes) << 22);
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MUL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0x9b << 24) | (rs1->id << 16) | (0x1f << 10) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FMUL(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x1e   << 24) | (0x1    << 21) | (0x1 << 11);
	opcode |= (rs1->id << 16) | (rs0->id << 5)  | rd->id;
	opcode |= ((8 == rd->bytes) << 22);
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_FDIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode  = (0x1e   << 24)  | (0x1    << 21) | (0x3 << 11);
	opcode |= (rs1->id << 16) | (rs0->id << 5) | rd->id;
	opcode |= ((8 == rd->bytes) << 22);
	inst    = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_DIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rs0->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs1->id << 16) | (0x2 << 10) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SDIV(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rs0, scf_register_t* rs1)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == rs0->bytes) << 31) | (0x1a << 24) | (0x3 << 22) | (rs1->id << 16) | (0x3 << 10) | (rs0->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_MSUB(scf_3ac_code_t* c, scf_register_t* rd, scf_register_t* rm, scf_register_t* rn, scf_register_t* ra)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = ((8 == ra->bytes) << 31) | (0x1b << 24) | (rm->id << 16) | (0x1 << 15) | (ra->id << 10) | (rn->id << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_BL(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x25 << 26;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_BLR(scf_3ac_code_t* c, scf_register_t* r)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = (0xd63f << 16) | (r->id << 5);
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_SETZ(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 1;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm64_inst_SETNZ(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm64_inst_SETGT(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xd;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm64_inst_SETGE(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xb;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm64_inst_SETLT(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xa;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}
scf_instruction_t* arm64_inst_SETLE(scf_3ac_code_t* c, scf_register_t* rd)
{
	scf_instruction_t* inst;
	uint32_t           opcode;
	uint32_t           cc = 0xc;

	opcode = ((8 == rd->bytes) << 1) | (0x1a << 24) | (0x1 << 23) | (0x1f << 16) | (cc << 12) | (0x3f << 5) | rd->id;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JMP(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x14000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JZ(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x54000000;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JNZ(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x54000001;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JGT(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x5400000c;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JGE(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x5400000a;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JLT(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x5400000b;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JLE(scf_3ac_code_t* c)
{
	scf_instruction_t* inst;
	uint32_t           opcode;

	opcode = 0x5400000d;
	inst   = risc_make_inst(c, opcode);

	return inst;
}

scf_instruction_t* arm64_inst_JA(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm64_inst_JB(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm64_inst_JAE(scf_3ac_code_t* c)
{
	return NULL;
}
scf_instruction_t* arm64_inst_JBE(scf_3ac_code_t* c)
{
	return NULL;
}

void risc_set_jmp_offset(scf_instruction_t* inst, int32_t bytes)
{
	if (0x54 == inst->code[3]) {

		if (bytes  >= 0 && bytes < (0x1 << 20)) {
			bytes >>= 2;
			bytes <<= 5;

		} else if (bytes < 0 && bytes > -(0x1 << 20)) {

			bytes >>= 2;
			bytes  &= 0x7ffff;
			bytes <<= 5;
		} else
			assert(0);

		inst->code[0] |= 0xff &  bytes;
		inst->code[1] |= 0xff & (bytes >>  8);
		inst->code[2] |= 0xff & (bytes >> 16);

	} else {
		assert(0x14 == inst->code[3]);

		bytes >>= 2;

		assert(bytes < (0x1 << 26) && bytes > -(0x1 << 26));

		inst->code[0] |= 0xff &  bytes;
		inst->code[1] |= 0xff & (bytes >>  8);
		inst->code[2] |= 0xff & (bytes >> 16);
		inst->code[3] |= 0x3  & (bytes >> 24);
	}
}

scf_inst_ops_t  inst_ops_arm64 =
{
	.name      = "arm64",

	.BL        = arm64_inst_BL,
	.BLR       = arm64_inst_BLR,
	.PUSH      = arm64_inst_PUSH,
	.POP       = arm64_inst_POP,
	.TEQ       = arm64_inst_TEQ,
	.NEG       = arm64_inst_NEG,

	.MOVZX     = arm64_inst_MOVZX,
	.MOVSX     = arm64_inst_MOVSX,
	.MVN       = arm64_inst_MVN,
	.MOV_G     = arm64_inst_MOV_G,
	.MOV_SP    = arm64_inst_MOV_SP,

	.ADD_G     = arm64_inst_ADD_G,
	.ADD_IMM   = arm64_inst_ADD_IMM,
	.SUB_G     = arm64_inst_SUB_G,
	.SUB_IMM   = arm64_inst_SUB_IMM,
	.CMP_G     = arm64_inst_CMP_G,
	.CMP_IMM   = arm64_inst_CMP_IMM,
	.AND_G     = arm64_inst_AND_G,
	.OR_G      = arm64_inst_OR_G,

	.MUL       = arm64_inst_MUL,
	.DIV       = arm64_inst_DIV,
	.SDIV      = arm64_inst_SDIV,
	.MSUB      = arm64_inst_MSUB,

	.SHL       = arm64_inst_SHL,
	.SHR       = arm64_inst_SHR,
	.ASR       = arm64_inst_ASR,

	.CVTSS2SD  = arm64_inst_CVTSS2SD,
	.CVTSD2SS  = arm64_inst_CVTSD2SS,
	.CVTF2SI   = arm64_inst_CVTF2SI,
	.CVTF2UI   = arm64_inst_CVTF2UI,
	.CVTSI2F   = arm64_inst_CVTSI2F,
	.CVTUI2F   = arm64_inst_CVTUI2F,

	.FCMP      = arm64_inst_FCMP,
	.FADD      = arm64_inst_FADD,
	.FSUB      = arm64_inst_FSUB,
	.FMUL      = arm64_inst_FMUL,
	.FDIV      = arm64_inst_FDIV,
	.FMOV_G    = arm64_inst_FMOV_G,

	.JA        = arm64_inst_JA,
	.JB        = arm64_inst_JB,
	.JZ        = arm64_inst_JZ,
	.JNZ       = arm64_inst_JNZ,
	.JGT       = arm64_inst_JGT,
	.JGE       = arm64_inst_JGE,
	.JLT       = arm64_inst_JLT,
	.JLE       = arm64_inst_JLE,
	.JAE       = arm64_inst_JAE,
	.JBE       = arm64_inst_JBE,
	.JMP       = arm64_inst_JMP,
	.RET       = arm64_inst_RET,

	.SETZ      = arm64_inst_SETZ,
	.SETNZ     = arm64_inst_SETNZ,
	.SETGT     = arm64_inst_SETGT,
	.SETGE     = arm64_inst_SETGE,
	.SETLT     = arm64_inst_SETLT,
	.SETLE     = arm64_inst_SETLE,

	.I2G       = arm64_inst_I2G,
	.M2G       = arm64_inst_M2G,
	.M2GF      = arm64_inst_M2GF,
	.G2M       = arm64_inst_G2M,
	.G2P       = arm64_inst_G2P,
	.P2G       = arm64_inst_P2G,
	.ISTR2G    = arm64_inst_ISTR2G,
	.SIB2G     = arm64_inst_SIB2G,
	.G2SIB     = arm64_inst_G2SIB,
	.ADR2G     = arm64_inst_ADR2G,
	.ADRP2G    = arm64_inst_ADRP2G,
	.ADRSIB2G  = arm64_inst_ADRSIB2G,

	.set_jmp_offset = risc_set_jmp_offset,
};

