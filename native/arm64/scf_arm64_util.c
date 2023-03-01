#include"scf_arm64.h"

scf_instruction_t* arm64_make_inst(scf_3ac_code_t* c, uint32_t opcode)
{
	scf_instruction_t* inst;

	inst = calloc(1, sizeof(scf_instruction_t));
	if (!inst)
		return NULL;

	inst->c       = c;
	inst->code[0] = opcode & 0xff;
	inst->code[1] = (opcode >>  8) & 0xff;
	inst->code[2] = (opcode >> 16) & 0xff;
	inst->code[3] = (opcode >> 24) & 0xff;
	inst->len     = 4;

	return inst;
}

int arm64_make_inst_I2G(scf_3ac_code_t* c, scf_register_arm64_t* rd, uint64_t imm, int bytes)
{
	scf_instruction_t* inst;

	uint32_t opcode;

	// mov rd, imm[15:0]
	opcode = (0xd2 << 24) | (0x1 << 23) | ((imm & 0xffff) << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	if ((imm >> 16) & 0xffff) {

		// movk rd, imm[31:16]
		opcode = (0xf2 << 24) | (0x1 << 23) | (0x1 << 21) | (((imm >> 16) & 0xffff) << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
	}

	if (8 == bytes) {

		if ((imm >> 32) & 0xffff) {

			// movk rd, imm[47:32]
			opcode = (0xf2 << 24) | (0x1 << 23) | (0x2 << 21) | (((imm >> 32) & 0xffff) << 5) | rd->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		if ((imm >> 48) & 0xffff) {

			// movk rd, imm[63:48]
			opcode = (0xf2 << 24) | (0x1 << 23) | (0x3 << 21) | (((imm >> 48) & 0xffff) << 5) | rd->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}
	}

	return 0;
}

int arm64_make_inst_ADR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_variable_t* vs)
{
	scf_register_arm64_t* fp   = arm64_find_register("fp");
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int64_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = arm64_variable_size(vs);

	if (vs->local_flag || vs->tmp_flag) {

		offset = vs->bp_offset;

		if (offset >= 0 && offset <= 0xfff)

			opcode = (0x91 << 24) | (offset << 10) | (fp->id << 5) | rd->id;

		else if (offset < 0 && -offset <= 0xfff)

			opcode = (0xd1 << 24) | ((-offset) << 10) | (fp->id << 5) | rd->id;

		else {
			int ret = arm64_make_inst_I2G(c, rd, offset, 8);
			if (ret < 0)
				return ret;

			opcode = (0x8b << 24) | (fp->id << 16) | (rd->id << 5) | rd->id;
		}

		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

	} else if (vs->global_flag) {
		offset = 0;

		opcode = (0x90 << 24) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
		rela->type = R_AARCH64_ADR_PREL_PG_HI21;

		opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
		rela->type = R_AARCH64_ADD_ABS_LO12_NC;

	} else {
		scf_loge("temp var should give a register\n");
		return -EINVAL;
	}

	return 0;
}

int arm64_make_inst_M2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, scf_variable_t* vs)
{
	scf_register_arm64_t* fp   = arm64_find_register("fp");
	scf_register_arm64_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = arm64_variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			opcode = (0x90 << 24) | rd->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADR_PREL_PG_HI21;

			opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
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
		int ret = arm64_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		opcode = (0x52 << 24) | (0x1 << 23) | ((offset & 0xffff) << 5) | ri->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (offset >> 16) {
			opcode  = (0x72 << 24) | (0x1 << 23) | (((offset >> 16) & 0xffff) << 5) | ri->id;
			inst    = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		opcode = (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10);
	}

	if (rd->bytes > size && scf_variable_signed(vs))
		opcode |= 0x2 << 22;
	else
		opcode |= 0x1 << 22;

	opcode |= (SIZE << 30) | (rb->id << 5) | rd->id;

	opcode |= ARM64_COLOR_TYPE(rd->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_make_inst_G2M(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rs, scf_register_arm64_t* rb, scf_variable_t* vs)
{
	scf_register_arm64_t* fp   = arm64_find_register("fp");
	scf_register_arm64_t* ri   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = arm64_variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			int ret = arm64_select_free_reg(&rb, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return -EINVAL;
			}

			opcode = (0x90 << 24) | rb->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADR_PREL_PG_HI21;

			opcode = (0x91 << 24) | (rb->id << 5) | rb->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
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

		opcode = (SIZE << 30) | (0x39 << 24) | (offset << 10) | (rb->id << 5) | rs->id;

	else if (offset < 0 && offset >= -0xff)

		opcode = (SIZE << 30) | (0x38 << 24) | ((offset & 0x1ff) << 12) | (rb->id << 5) | rs->id;

	else {
		int ret = arm64_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		opcode = (0x52 << 24) | (0x1 << 23) | ((offset & 0xffff) << 5) | ri->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (offset >> 16) {
			opcode  = (0x72 << 24) | (0x1 << 23) | (((offset >> 16) & 0xffff) << 5) | ri->id;
			inst    = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rs->id;
	}

	opcode |= ARM64_COLOR_TYPE(rs->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_make_inst_ISTR2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_variable_t* v)
{
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int size1 = arm64_variable_size(v);

	assert(8 == rd->bytes);
	assert(8 == size1);

	v->global_flag = 1;
	v->local_flag  = 0;
	v->tmp_flag    = 0;

	uint32_t opcode;

	opcode = (0x90 << 24) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	rela->type = R_AARCH64_ADR_PREL_PG_HI21;

	opcode = (0x91 << 24) | (rd->id << 5) | rd->id;
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, v, NULL);
	rela->type = R_AARCH64_ADD_ABS_LO12_NC;

	return 0;
}

int arm64_make_inst_G2P(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rs, scf_register_arm64_t* rb, int32_t offset, int size)
{
	scf_register_arm64_t* ri   = NULL;
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
		int ret = arm64_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		opcode = (0x52 << 24) | (0x1 << 23) | ((offset & 0xffff) << 5) | ri->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (offset >> 16) {
			opcode  = (0x72 << 24) | (0x1 << 23) | (((offset >> 16) & 0xffff) << 5) | ri->id;
			inst    = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rs->id;
	}

	opcode |= ARM64_COLOR_TYPE(rs->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_make_inst_P2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, int32_t offset, int size)
{
	scf_register_arm64_t* ri   = NULL;
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
		int ret = arm64_select_free_reg(&ri, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return -EINVAL;
		}

		opcode = (0x52 << 24) | (0x1 << 23) | ((offset & 0xffff) << 5) | ri->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (offset >> 16) {
			opcode  = (0x72 << 24) | (0x1 << 23) | (((offset >> 16) & 0xffff) << 5) | ri->id;
			inst    = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		opcode = (SIZE << 30) | (0x38 << 24) | (0x1 << 22) | (0x1 << 21) | (ri->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10) | (rb->id << 5) | rd->id;
	}

	opcode |= ARM64_COLOR_TYPE(rd->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_make_inst_ADRP2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, int32_t offset)
{
	scf_register_arm64_t* r    = NULL;
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
		int ret = arm64_select_free_reg(&r, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_I2G(c, r, offset, 4);
		if (ret < 0)
			return ret;

		opcode = (u24r << 24) | (r->id << 16) | (rd->id << 5) | rd->id;
	}

	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_make_inst_ADRSIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, arm64_sib_t* sib)
{
	scf_register_arm64_t* rb   = sib->base;
	scf_register_arm64_t* ri   = sib->index;
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
	inst   = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);
	return 0;
}

int arm64_make_inst_SIB2G(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, arm64_sib_t* sib)
{
	scf_register_arm64_t* rb   = sib->base;
	scf_register_arm64_t* ri   = sib->index;
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

	opcode |= ARM64_COLOR_TYPE(rd->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_make_inst_G2SIB(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rs, arm64_sib_t* sib)
{
	scf_register_arm64_t* rb   = sib->base;
	scf_register_arm64_t* ri   = sib->index;
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

	opcode |= ARM64_COLOR_TYPE(rs->color) << 26;

	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}

int arm64_make_inst_M2GF(scf_3ac_code_t* c, scf_function_t* f, scf_register_arm64_t* rd, scf_register_arm64_t* rb, scf_variable_t* vs)
{
	scf_register_arm64_t* fp   = arm64_find_register("fp");
	scf_register_arm64_t* ro   = NULL;
	scf_instruction_t*    inst = NULL;
	scf_rela_t*           rela = NULL;

	int32_t  offset;
	uint32_t opcode;
	uint32_t SIZE = 0;
	uint32_t S    = 1;

	int size = arm64_variable_size(vs);

	if (!rb) {
		if (vs->local_flag || vs->tmp_flag) {

			offset = vs->bp_offset;
			rb     = fp;

		} else if (vs->global_flag) {
			offset = 0;

			int ret = arm64_select_free_reg(&rb, c, f, 0);
			if (ret < 0)
				return ret;

			opcode = (0x90 << 24) | rb->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
			rela->type = R_AARCH64_ADR_PREL_PG_HI21;

			opcode = (0x91 << 24) | (rb->id << 5) | rb->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
			ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vs, NULL);
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
		int ret = arm64_select_free_reg(&ro, c, f, 0);
		if (ret < 0)
			return ret;

		opcode = (0x52 << 24) | (0x1 << 23) | ((offset & 0xffff) << 5) | ro->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		if (offset >> 16) {
			opcode  = (0x72 << 24) | (0x1 << 23) | (((offset >> 16) & 0xffff) << 5) | ro->id;
			inst    = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		opcode = (0x3c << 24) | (0x1 << 22) | (0x1 << 21) | (ro->id << 16) | (0x3 << 13) | (S << 12) | (0x2 << 10);
	}

	opcode |= (SIZE << 30) | (rb->id << 5) | rd->id;
	inst    = arm64_make_inst(c, opcode);
	ARM64_INST_ADD_CHECK(c->instructions, inst);

	return 0;
}
