#include"scf_arm64.h"

scf_register_arm64_t	arm64_registers[] = {

	{0, 4, "w0",    ARM64_COLOR(0, 0, 0xf),  NULL, 0, 0},
	{0, 8, "x0",    ARM64_COLOR(0, 0, 0xff), NULL, 0, 0},

	{1, 4, "w1",    ARM64_COLOR(0, 1, 0xf),  NULL, 0, 0},
	{1, 8, "x1",    ARM64_COLOR(0, 1, 0xff), NULL, 0, 0},

	{2, 4, "w2",    ARM64_COLOR(0, 2, 0xf),  NULL, 0, 0},
	{2, 8, "x2",    ARM64_COLOR(0, 2, 0xff), NULL, 0, 0},

	{3, 4, "w3",    ARM64_COLOR(0, 3, 0xf),  NULL, 0, 0},
	{3, 8, "x3",    ARM64_COLOR(0, 3, 0xff), NULL, 0, 0},

	{4, 4, "w4",    ARM64_COLOR(0, 4, 0xf),  NULL, 0, 0},
	{4, 8, "x4",    ARM64_COLOR(0, 4, 0xff), NULL, 0, 0},

	{5, 4, "w5",    ARM64_COLOR(0, 5, 0xf),  NULL, 0, 0},
	{5, 8, "x5",    ARM64_COLOR(0, 5, 0xff), NULL, 0, 0},

	{6, 4, "w6",    ARM64_COLOR(0, 6, 0xf),  NULL, 0, 0},
	{6, 8, "x6",    ARM64_COLOR(0, 6, 0xff), NULL, 0, 0},

	{7, 4, "w7",    ARM64_COLOR(0, 7, 0xf),  NULL, 0, 0},
	{7, 8, "x7",    ARM64_COLOR(0, 7, 0xff), NULL, 0, 0},

// not use x8

//	{8, 4, "w8",    ARM64_COLOR(0, 8,  0xf),  NULL, 0},
//	{8, 8, "x8",    ARM64_COLOR(0, 8,  0xff), NULL, 0},

	{9, 4, "w9",    ARM64_COLOR(0, 9,  0xf),  NULL, 0, 0},
	{9, 8, "x9",    ARM64_COLOR(0, 9,  0xff), NULL, 0, 0},

	{10, 4, "w10",  ARM64_COLOR(0, 10, 0xf),  NULL, 0, 0},
	{10, 8, "x10",  ARM64_COLOR(0, 10, 0xff), NULL, 0, 0},

	{11, 4, "w11",  ARM64_COLOR(0, 11, 0xf),  NULL, 0, 0},
	{11, 8, "x11",  ARM64_COLOR(0, 11, 0xff), NULL, 0, 0},

	{12, 4, "w12",  ARM64_COLOR(0, 12, 0xf),  NULL, 0, 0},
	{12, 8, "x12",  ARM64_COLOR(0, 12, 0xff), NULL, 0, 0},

	{13, 4, "w13",  ARM64_COLOR(0, 13, 0xf),  NULL, 0, 0},
	{13, 8, "x13",  ARM64_COLOR(0, 13, 0xff), NULL, 0, 0},

	{14, 4, "w14",  ARM64_COLOR(0, 14, 0xf),  NULL, 0, 0},
	{14, 8, "x14",  ARM64_COLOR(0, 14, 0xff), NULL, 0, 0},

	{15, 4, "w15",  ARM64_COLOR(0, 15, 0xf),  NULL, 0, 0},
	{15, 8, "x15",  ARM64_COLOR(0, 15, 0xff), NULL, 0, 0},

// not use x16, x17, x18

//	{16, 4, "w16",  ARM64_COLOR(0, 16, 0xf),  NULL, 0, 0},
//	{16, 8, "x16",  ARM64_COLOR(0, 16, 0xff), NULL, 0, 0},

//	{17, 4, "w17",  ARM64_COLOR(0, 17, 0xf),  NULL, 0, 0},
//	{17, 8, "x17",  ARM64_COLOR(0, 17, 0xff), NULL, 0, 0},

//	{18, 4, "w18",  ARM64_COLOR(0, 18, 0xf),  NULL, 0, 0},
//	{18, 8, "x18",  ARM64_COLOR(0, 18, 0xff), NULL, 0, 0},

	{19, 4, "w19",  ARM64_COLOR(0, 19, 0xf),  NULL, 0, 0},
	{19, 8, "x19",  ARM64_COLOR(0, 19, 0xff), NULL, 0, 0},

	{20, 4, "w20",  ARM64_COLOR(0, 20, 0xf),  NULL, 0, 0},
	{20, 8, "x20",  ARM64_COLOR(0, 20, 0xff), NULL, 0, 0},

	{21, 4, "w21",  ARM64_COLOR(0, 21, 0xf),  NULL, 0, 0},
	{21, 8, "x21",  ARM64_COLOR(0, 21, 0xff), NULL, 0, 0},

	{22, 4, "w22",  ARM64_COLOR(0, 22, 0xf),  NULL, 0, 0},
	{22, 8, "x22",  ARM64_COLOR(0, 22, 0xff), NULL, 0, 0},

	{23, 4, "w23",  ARM64_COLOR(0, 23, 0xf),  NULL, 0, 0},
	{23, 8, "x23",  ARM64_COLOR(0, 23, 0xff), NULL, 0, 0},

	{24, 4, "w24",  ARM64_COLOR(0, 24, 0xf),  NULL, 0, 0},
	{24, 8, "x24",  ARM64_COLOR(0, 24, 0xff), NULL, 0, 0},

	{25, 4, "w25",  ARM64_COLOR(0, 25, 0xf),  NULL, 0, 0},
	{25, 8, "x25",  ARM64_COLOR(0, 25, 0xff), NULL, 0, 0},

	{26, 4, "w26",  ARM64_COLOR(0, 26, 0xf),  NULL, 0, 0},
	{26, 8, "x26",  ARM64_COLOR(0, 26, 0xff), NULL, 0, 0},

	{27, 4, "w27",  ARM64_COLOR(0, 27, 0xf),  NULL, 0, 0},
	{27, 8, "x27",  ARM64_COLOR(0, 27, 0xff), NULL, 0, 0},

	{28, 4, "w28",  ARM64_COLOR(0, 28, 0xf),  NULL, 0, 0},
	{28, 8, "x28",  ARM64_COLOR(0, 28, 0xff), NULL, 0, 0},

// fp = x29 = bp
	{29, 4, "w29",  ARM64_COLOR(0, 29, 0xf),  NULL, 0, 0},
	{29, 8, "fp",   ARM64_COLOR(0, 29, 0xff), NULL, 0, 0},
// lr = x30
	{30, 4, "w30",  ARM64_COLOR(0, 30, 0xf),  NULL, 0, 0},
	{30, 8, "lr",   ARM64_COLOR(0, 30, 0xff), NULL, 0, 0},
	{31, 8, "sp",   ARM64_COLOR(0, 31, 0xff), NULL, 0, 0},


	{0, 2, "h0",    ARM64_COLOR(1, 0, 0x3),  NULL, 0, 0},
	{0, 4, "s0",    ARM64_COLOR(1, 0, 0xf),  NULL, 0, 0},
	{0, 8, "d0",    ARM64_COLOR(1, 0, 0xff), NULL, 0, 0},

	{1, 2, "h1",    ARM64_COLOR(1, 1, 0x3),  NULL, 0, 0},
	{1, 4, "s1",    ARM64_COLOR(1, 1, 0xf),  NULL, 0, 0},
	{1, 8, "d1",    ARM64_COLOR(1, 1, 0xff), NULL, 0, 0},

	{2, 2, "h2",    ARM64_COLOR(1, 2, 0x3),  NULL, 0, 0},
	{2, 4, "s2",    ARM64_COLOR(1, 2, 0xf),  NULL, 0, 0},
	{2, 8, "d2",    ARM64_COLOR(1, 2, 0xff), NULL, 0, 0},

	{3, 2, "h3",    ARM64_COLOR(1, 3, 0x3),  NULL, 0, 0},
	{3, 4, "s3",    ARM64_COLOR(1, 3, 0xf),  NULL, 0, 0},
	{3, 8, "d3",    ARM64_COLOR(1, 3, 0xff), NULL, 0, 0},

	{4, 2, "h4",    ARM64_COLOR(1, 4, 0x3),  NULL, 0, 0},
	{4, 4, "s4",    ARM64_COLOR(1, 4, 0xf),  NULL, 0, 0},
	{4, 8, "d4",    ARM64_COLOR(1, 4, 0xff), NULL, 0, 0},

	{5, 2, "h5",    ARM64_COLOR(1, 5, 0x3),  NULL, 0, 0},
	{5, 4, "s5",    ARM64_COLOR(1, 5, 0xf),  NULL, 0, 0},
	{5, 8, "d5",    ARM64_COLOR(1, 5, 0xff), NULL, 0, 0},

	{6, 2, "h6",    ARM64_COLOR(1, 6, 0x3),  NULL, 0, 0},
	{6, 4, "s6",    ARM64_COLOR(1, 6, 0xf),  NULL, 0, 0},
	{6, 8, "d6",    ARM64_COLOR(1, 6, 0xff), NULL, 0, 0},

	{7, 2, "h7",    ARM64_COLOR(1, 7, 0x3),  NULL, 0, 0},
	{7, 4, "s7",    ARM64_COLOR(1, 7, 0xf),  NULL, 0, 0},
	{7, 8, "d7",    ARM64_COLOR(1, 7, 0xff), NULL, 0, 0},

	{8, 2, "h8",    ARM64_COLOR(1, 8, 0x3),  NULL, 0, 0},
	{8, 4, "s8",    ARM64_COLOR(1, 8, 0xf),  NULL, 0, 0},
	{8, 8, "d8",    ARM64_COLOR(1, 8, 0xff), NULL, 0, 0},

	{9, 2, "h9",    ARM64_COLOR(1, 9, 0x3),  NULL, 0, 0},
	{9, 4, "s9",    ARM64_COLOR(1, 9, 0xf),  NULL, 0, 0},
	{9, 8, "d9",    ARM64_COLOR(1, 9, 0xff), NULL, 0, 0},

	{10, 2, "h10",    ARM64_COLOR(1, 10, 0x3),  NULL, 0, 0},
	{10, 4, "s10",    ARM64_COLOR(1, 10, 0xf),  NULL, 0, 0},
	{10, 8, "d10",    ARM64_COLOR(1, 10, 0xff), NULL, 0, 0},

	{11, 2, "h11",    ARM64_COLOR(1, 11, 0x3),  NULL, 0, 0},
	{11, 4, "s11",    ARM64_COLOR(1, 11, 0xf),  NULL, 0, 0},
	{11, 8, "d11",    ARM64_COLOR(1, 11, 0xff), NULL, 0, 0},

	{12, 2, "h12",    ARM64_COLOR(1, 12, 0x3),  NULL, 0, 0},
	{12, 4, "s12",    ARM64_COLOR(1, 12, 0xf),  NULL, 0, 0},
	{12, 8, "d12",    ARM64_COLOR(1, 12, 0xff), NULL, 0, 0},

	{13, 2, "h13",    ARM64_COLOR(1, 13, 0x3),  NULL, 0, 0},
	{13, 4, "s13",    ARM64_COLOR(1, 13, 0xf),  NULL, 0, 0},
	{13, 8, "d13",    ARM64_COLOR(1, 13, 0xff), NULL, 0, 0},

	{14, 2, "h14",    ARM64_COLOR(1, 14, 0x3),  NULL, 0, 0},
	{14, 4, "s14",    ARM64_COLOR(1, 14, 0xf),  NULL, 0, 0},
	{14, 8, "d14",    ARM64_COLOR(1, 14, 0xff), NULL, 0, 0},

	{15, 2, "h15",    ARM64_COLOR(1, 15, 0x3),  NULL, 0, 0},
	{15, 4, "s15",    ARM64_COLOR(1, 15, 0xf),  NULL, 0, 0},
	{15, 8, "d15",    ARM64_COLOR(1, 15, 0xff), NULL, 0, 0},

	{16, 2, "h16",    ARM64_COLOR(1, 16, 0x3),  NULL, 0, 0},
	{16, 4, "s16",    ARM64_COLOR(1, 16, 0xf),  NULL, 0, 0},
	{16, 8, "d16",    ARM64_COLOR(1, 16, 0xff), NULL, 0, 0},

	{17, 2, "h17",    ARM64_COLOR(1, 17, 0x3),  NULL, 0, 0},
	{17, 4, "s17",    ARM64_COLOR(1, 17, 0xf),  NULL, 0, 0},
	{17, 8, "d17",    ARM64_COLOR(1, 17, 0xff), NULL, 0, 0},

	{18, 2, "h18",    ARM64_COLOR(1, 18, 0x3),  NULL, 0, 0},
	{18, 4, "s18",    ARM64_COLOR(1, 18, 0xf),  NULL, 0, 0},
	{18, 8, "d18",    ARM64_COLOR(1, 18, 0xff), NULL, 0, 0},

	{19, 2, "h19",    ARM64_COLOR(1, 19, 0x3),  NULL, 0, 0},
	{19, 4, "s19",    ARM64_COLOR(1, 19, 0xf),  NULL, 0, 0},
	{19, 8, "d19",    ARM64_COLOR(1, 19, 0xff), NULL, 0, 0},

	{20, 2, "h20",    ARM64_COLOR(1, 20, 0x3),  NULL, 0, 0},
	{20, 4, "s20",    ARM64_COLOR(1, 20, 0xf),  NULL, 0, 0},
	{20, 8, "d20",    ARM64_COLOR(1, 20, 0xff), NULL, 0, 0},

	{21, 2, "h21",    ARM64_COLOR(1, 21, 0x3),  NULL, 0, 0},
	{21, 4, "s21",    ARM64_COLOR(1, 21, 0xf),  NULL, 0, 0},
	{21, 8, "d21",    ARM64_COLOR(1, 21, 0xff), NULL, 0, 0},

	{22, 2, "h22",    ARM64_COLOR(1, 22, 0x3),  NULL, 0, 0},
	{22, 4, "s22",    ARM64_COLOR(1, 22, 0xf),  NULL, 0, 0},
	{22, 8, "d22",    ARM64_COLOR(1, 22, 0xff), NULL, 0, 0},

	{23, 2, "h23",    ARM64_COLOR(1, 23, 0x3),  NULL, 0, 0},
	{23, 4, "s23",    ARM64_COLOR(1, 23, 0xf),  NULL, 0, 0},
	{23, 8, "d23",    ARM64_COLOR(1, 23, 0xff), NULL, 0, 0},

	{24, 2, "h24",    ARM64_COLOR(1, 24, 0x3),  NULL, 0, 0},
	{24, 4, "s24",    ARM64_COLOR(1, 24, 0xf),  NULL, 0, 0},
	{24, 8, "d24",    ARM64_COLOR(1, 24, 0xff), NULL, 0, 0},

	{25, 2, "h25",    ARM64_COLOR(1, 25, 0x3),  NULL, 0, 0},
	{25, 4, "s25",    ARM64_COLOR(1, 25, 0xf),  NULL, 0, 0},
	{25, 8, "d25",    ARM64_COLOR(1, 25, 0xff), NULL, 0, 0},

	{26, 2, "h26",    ARM64_COLOR(1, 26, 0x3),  NULL, 0, 0},
	{26, 4, "s26",    ARM64_COLOR(1, 26, 0xf),  NULL, 0, 0},
	{26, 8, "d26",    ARM64_COLOR(1, 26, 0xff), NULL, 0, 0},

	{27, 2, "h27",    ARM64_COLOR(1, 27, 0x3),  NULL, 0, 0},
	{27, 4, "s27",    ARM64_COLOR(1, 27, 0xf),  NULL, 0, 0},
	{27, 8, "d27",    ARM64_COLOR(1, 27, 0xff), NULL, 0, 0},

	{28, 2, "h28",    ARM64_COLOR(1, 28, 0x3),  NULL, 0, 0},
	{28, 4, "s28",    ARM64_COLOR(1, 28, 0xf),  NULL, 0, 0},
	{28, 8, "d28",    ARM64_COLOR(1, 28, 0xff), NULL, 0, 0},

	{29, 2, "h29",    ARM64_COLOR(1, 29, 0x3),  NULL, 0, 0},
	{29, 4, "s29",    ARM64_COLOR(1, 29, 0xf),  NULL, 0, 0},
	{29, 8, "d29",    ARM64_COLOR(1, 29, 0xff), NULL, 0, 0},

	{30, 2, "h30",    ARM64_COLOR(1, 30, 0x3),  NULL, 0, 0},
	{30, 4, "s30",    ARM64_COLOR(1, 30, 0xf),  NULL, 0, 0},
	{30, 8, "d30",    ARM64_COLOR(1, 30, 0xff), NULL, 0, 0},

	{31, 2, "h31",    ARM64_COLOR(1, 31, 0x3),  NULL, 0, 0},
	{31, 4, "s31",    ARM64_COLOR(1, 31, 0xf),  NULL, 0, 0},
	{31, 8, "d31",    ARM64_COLOR(1, 31, 0xff), NULL, 0, 0},
};

int arm64_reg_cached_vars(scf_register_arm64_t* r)
{
	int nb_vars = 0;
	int i;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r2 = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r2->id
         || SCF_ARM64_REG_FP == r2->id
         || SCF_ARM64_REG_LR == r2->id)
			continue;

		if (!ARM64_COLOR_CONFLICT(r->color, r2->color))
			continue;

		nb_vars += r2->dag_nodes->size;
	}

	return nb_vars;
}

int arm64_registers_init()
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
         || SCF_ARM64_REG_FP == r->id
         || SCF_ARM64_REG_LR == r->id)
			continue;

		assert(!r->dag_nodes);

		r->dag_nodes = scf_vector_alloc();
		if (!r->dag_nodes)
			return -ENOMEM;

		r->used = 0;
	}

	return 0;
}

void arm64_registers_clear()
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
         || SCF_ARM64_REG_FP == r->id
         || SCF_ARM64_REG_LR == r->id)
			continue;

		if (r->dag_nodes) {
			scf_vector_free(r->dag_nodes);
			r->dag_nodes = NULL;
		}

		r->used = 0;
	}
}

int arm64_caller_save_regs(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_arm64_t** saved_regs)
{
	int i;
	int j;
	scf_register_arm64_t* r;
	scf_register_arm64_t* r2;
	scf_instruction_t*    inst;
	scf_register_arm64_t* sp   = arm64_find_register("sp");

	uint32_t opcode;

	int ret;
	int size = 0;
	int k    = 0;

	for (j = 0; j < nb_regs; j++) {
		r2 = arm64_find_register_type_id_bytes(0, regs[j], 8);

		for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {
			r  = &(arm64_registers[i]);

			if (SCF_ARM64_REG_SP == r->id
             || SCF_ARM64_REG_FP == r->id
             || SCF_ARM64_REG_LR == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (ARM64_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm64_registers) / sizeof(arm64_registers[0]))
			continue;

		if (stack_size > 0) {
			ret = arm64_make_inst_G2P(c, f, r2, sp, size + stack_size, 8);
			if (ret < 0)
				return ret;
		} else {
			opcode = (0xf8 << 24) | (0x1f8 << 12) | (0x3 << 10) | (sp->id << 5) | r2->id;
			inst   = arm64_make_inst(NULL, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		saved_regs[k++] = r2;
		size += 8;
	}

	if (size & 0xf) {
		r2 = saved_regs[k - 1];

		if (stack_size > 0) {
			ret = arm64_make_inst_G2P(c, f, r2, sp, size + stack_size, 8);
			if (ret < 0)
				return ret;
		} else {
			opcode = (0xf8 << 24) | (0x1f8 << 12) | (0x3 << 10) | (sp->id << 5) | r2->id;
			inst   = arm64_make_inst(NULL, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		saved_regs[k++] = r2;
		size += 8;
	}

	if (stack_size > 0) {
		for (j = 0; j < k / 2; j++) {

			i  = k - 1 - j;
			SCF_XCHG(saved_regs[i], saved_regs[j]);
		}
	}

	return size;
}

int arm64_push_regs(scf_vector_t* instructions, uint32_t* regs, int nb_regs)
{
#if 0
	int i;
	int j;
	scf_register_arm64_t* r;
	scf_register_arm64_t* r2;
	scf_instruction_t*    inst;
	scf_arm64_OpCode_t*   push = arm64_find_OpCode(SCF_ARM64_PUSH, 8,8, SCF_ARM64_G);

	for (j = 0; j < nb_regs; j++) {
		r2 = arm64_find_register_type_id_bytes(0, regs[j], 8);

		for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {
			r  = &(arm64_registers[i]);

			if (SCF_ARM64_REG_SP == r->id
             || SCF_ARM64_REG_FP == r->id
             || SCF_ARM64_REG_LR == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (ARM64_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm64_registers) / sizeof(arm64_registers[0]))
			continue;

		inst = arm64_make_inst_G(push, r2);
		ARM64_INST_ADD_CHECK(instructions, inst);
	}
	return 0;
#endif
	return -1;
}

int arm64_pop_regs(scf_3ac_code_t* c, scf_register_arm64_t** regs, int nb_regs, scf_register_arm64_t** updated_regs, int nb_updated)
{
	int i;
	int j;

	scf_register_arm64_t* sp = arm64_find_register("sp");
	scf_register_arm64_t* r;
	scf_register_arm64_t* r2;
	scf_instruction_t*    inst;

	for (j = nb_regs - 1; j >= 0; j--) {
		r2 = regs[j];

		for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {
			r  = &(arm64_registers[i]);

			if (SCF_ARM64_REG_SP == r->id
             || SCF_ARM64_REG_FP == r->id
             || SCF_ARM64_REG_LR == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (ARM64_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm64_registers) / sizeof(arm64_registers[0]))
			continue;

		for (i = 0; i < nb_updated; i++) {

			r  = updated_regs[i];

			if (ARM64_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == nb_updated) {
			uint32_t pop = (0xf8 << 24) | (0x1 << 22) | (0x8 << 12) | (0x1 << 10) | (0x1f << 5) | r2->id;

			inst = arm64_make_inst(c, pop);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		} else {
			uint32_t add8 = (0x91 << 24) | (0x8 << 10) | (sp->id << 5) | sp->id;
			inst = arm64_make_inst(c, add8);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}
	}
	return 0;
}

int arm64_registers_reset()
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
				|| SCF_ARM64_REG_FP == r->id
				|| SCF_ARM64_REG_LR == r->id)
			continue;

		if (!r->dag_nodes)
			continue;

		int j = 0;
		while (j < r->dag_nodes->size) {
			scf_dag_node_t* dn = r->dag_nodes->data[j];

			if (dn->var->w)
				scf_logw("drop: v_%d_%d/%s\n", dn->var->w->line, dn->var->w->pos, dn->var->w->text->data);
			else
				scf_logw("drop: v_%#lx\n", 0xffff & (uintptr_t)dn->var);

			int ret = scf_vector_del(r->dag_nodes, dn);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}

			dn->loaded     = 0;
			dn->color      = 0;
		}
	}

	return 0;
}

scf_register_arm64_t* arm64_find_register(const char* name)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (!strcmp(r->name, name))
			return r;
	}
	return NULL;
}

scf_register_arm64_t* arm64_find_register_type_id_bytes(uint32_t type, uint32_t id, int bytes)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (ARM64_COLOR_TYPE(r->color) == type && r->id == id && r->bytes == bytes)
			return r;
	}
	return NULL;
}

scf_register_arm64_t* arm64_find_register_color(intptr_t color)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (r->color == color)
			return r;
	}
	return NULL;
}

scf_register_arm64_t* arm64_find_register_color_bytes(intptr_t color, int bytes)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (ARM64_COLOR_CONFLICT(r->color, color) && r->bytes == bytes)
			return r;
	}
	return NULL;
}

scf_vector_t* arm64_register_colors()
{
	scf_vector_t* colors = scf_vector_alloc();
	if (!colors)
		return NULL;

	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
				|| SCF_ARM64_REG_FP == r->id
				|| SCF_ARM64_REG_LR == r->id)
			continue;

		int ret = scf_vector_add(colors, (void*)r->color);
		if (ret < 0) {
			scf_vector_free(colors);
			return NULL;
		}
	}
#if 0
	srand(time(NULL));
	for (i = 0; i < colors->size; i++) {
		int j = rand() % colors->size;

		void* t = colors->data[i];
		colors->data[i] = colors->data[j];
		colors->data[j] = t;
	}
#endif
	return colors;
}

int arm64_save_var2(scf_dag_node_t* dn, scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_variable_t*     v    = dn->var;
	scf_rela_t*         rela = NULL;
	scf_arm64_OpCode_t*   mov;
	scf_instruction_t*  inst;

	int var_size = arm64_variable_size(v);
	int is_float = scf_variable_float(v);

	assert(var_size == r->bytes);

	if (v->const_literal_flag) {
		scf_logw("const literal var: v_%s_%d_%d not save\n", v->w->text->data, v->w->line, v->w->pos);
		goto end;
	}

	// if temp var in register, alloc it in stack
	if (0 == v->bp_offset && !v->global_flag && !v->local_flag) {

		int local_vars_size  = f->local_vars_size;
		local_vars_size     += var_size;

		if (local_vars_size & 0x7)
			local_vars_size = (local_vars_size + 7) >> 3 << 3;

		v->bp_offset  = -local_vars_size;
		v->tmp_flag   = 1;

		f->local_vars_size = local_vars_size;

		scf_logw("r: %s, temp var, ", r->name);
		if (v->w)
			printf("v_%d_%d/%s, bp_offset: %d\n", v->w->line, v->w->pos, v->w->text->data, v->bp_offset);
		else
			printf("v_%#lx, bp_offset: %d\n", 0xffff & (uintptr_t)v, v->bp_offset);

	} else {
#if 1
		if (v->w)
			scf_logw("save var: v_%d_%d/%s, ", v->w->line, v->w->pos, v->w->text->data);
		else
			scf_logw("save var: v_%#lx, ", 0xffff & (uintptr_t)v);
		printf("size: %d, bp_offset: %d, r: %s\n", var_size, v->bp_offset, r->name);
#endif
	}

	if (is_float) {
		scf_loge("\n");
		return -EINVAL;
	}

	scf_loge("v->size: %d\n", v->size);

	int ret = arm64_make_inst_G2M(c, f, r, NULL, v);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

end:
	// if this var is function argment, it become a normal local var
	v->arg_flag =  0;
	dn->color   = -1;
	dn->loaded  =  0;

	scf_vector_del(r->dag_nodes, dn);
	return 0;
}

int arm64_save_var(scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	if (dn->color <= 0)
		return -EINVAL;

	scf_register_arm64_t* r = arm64_find_register_color(dn->color);

	return arm64_save_var2(dn, r, c, f);
}

int arm64_save_reg(scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f)
{
	int i = 0;
	while (i < r->dag_nodes->size) {

		scf_dag_node_t* dn = r->dag_nodes->data[i];

		int ret = arm64_save_var(dn, c, f);
		if (ret < 0) {
			scf_loge("i: %d, size: %d, r: %s, dn->var: %s\n", i, r->dag_nodes->size, r->name, dn->var->w->text->data);
			return ret;
		}
	}

	return 0;
}

int arm64_overflow_reg(scf_register_arm64_t* r, scf_3ac_code_t* c, scf_function_t* f)
{
	int i;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r2 = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r2->id
				|| SCF_ARM64_REG_FP == r2->id
				|| SCF_ARM64_REG_LR == r2->id)
			continue;

		if (!ARM64_COLOR_CONFLICT(r->color, r2->color))
			continue;

		int ret = arm64_save_reg(r2, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	r->used = 1;
	return 0;
}

int arm64_overflow_reg2(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_arm64_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r2->id
				|| SCF_ARM64_REG_FP == r2->id
				|| SCF_ARM64_REG_LR == r2->id)
			continue;

		if (!ARM64_COLOR_CONFLICT(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; ) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 == dn) {
				j++;
				continue;
			}

			int ret = arm64_save_var(dn2, c, f);
			if (ret < 0)
				return ret;
		}
	}

	r->used = 1;
	return 0;
}

static int _arm64_overflow_reg3(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_arm64_t*	r2;
	scf_dn_status_t*    ds2;
	scf_dag_node_t*     dn2;

	int i;
	int j;
	int ret;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r2->id
				|| SCF_ARM64_REG_FP == r2->id
				|| SCF_ARM64_REG_LR == r2->id)
			continue;

		if (!ARM64_COLOR_CONFLICT(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; ) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 == dn) {
				j++;
				continue;
			}

			ds2 = scf_vector_find_cmp(c->active_vars, dn2, scf_dn_status_cmp);
			if (!ds2) {
				j++;
				continue;
			}

			if (!ds2->active) {
				j++;
				continue;
			}
#if 1
			scf_variable_t* v  = dn->var;
			scf_variable_t* v2 = dn2->var;
			if (v->w)
				scf_loge("v_%d_%d/%s, bp_offset: %d\n", v->w->line, v->w->pos, v->w->text->data, v->bp_offset);
			else
				scf_loge("v_%#lx, bp_offset: %d\n", 0xffff & (uintptr_t)v, v->bp_offset);

			if (v2->w)
				scf_loge("v2_%d_%d/%s, bp_offset: %d\n", v2->w->line, v2->w->pos, v2->w->text->data, v2->bp_offset);
			else
				scf_loge("v2_%#lx, bp_offset: %d\n", 0xffff & (uintptr_t)v2, v2->bp_offset);
#endif
			int ret = arm64_save_var(dn2, c, f);
			if (ret < 0)
				return ret;
		}
	}

	r->used = 1;
	return 0;
}

int arm64_reg_used(scf_register_arm64_t* r, scf_dag_node_t* dn)
{
	scf_register_arm64_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r2->id
				|| SCF_ARM64_REG_FP == r2->id
				|| SCF_ARM64_REG_LR == r2->id)
			continue;

		if (!ARM64_COLOR_CONFLICT(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; j++) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 != dn)
				return 1;
		}
	}
	return 0;
}

static scf_register_arm64_t* _arm64_reg_cached_min_vars(scf_register_arm64_t** regs, int nb_regs)
{
	scf_register_arm64_t* r_min = NULL;

	int min = 0;
	int i;

	for (i = 0; i < nb_regs; i++) {
		scf_register_arm64_t*	r = regs[i];

		int nb_vars = arm64_reg_cached_vars(r);

		if (!r_min) {
			r_min = r;
			min   = nb_vars;
			continue;
		}

		if (min > nb_vars) {
			r_min = r;
			min   = nb_vars;
		}
	}

	return r_min;
}

scf_register_arm64_t* arm64_select_overflowed_reg(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float)
{
	scf_vector_t*       neighbors = NULL;
	scf_graph_node_t*   gn        = NULL;

	scf_register_arm64_t* free_regs[sizeof(arm64_registers) / sizeof(arm64_registers[0])];

	int nb_free_regs = 0;
	int bytes        = 8;
	int ret;
	int i;
	int j;

	assert(c->rcg);

	if (dn) {
		is_float =   scf_variable_float(dn->var);
		bytes    = arm64_variable_size (dn->var);
	}

	ret = arm64_rcg_find_node(&gn, c->rcg, dn, NULL);
	if (ret < 0)
		neighbors = c->rcg->nodes;
	else
		neighbors = gn->neighbors;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
				|| SCF_ARM64_REG_FP == r->id
				|| SCF_ARM64_REG_LR == r->id)
			continue;

		if (r->bytes < bytes || ARM64_COLOR_TYPE(r->color) != is_float)
			continue;

		for (j = 0; j < neighbors->size; j++) {

			scf_graph_node_t* neighbor = neighbors->data[j];
			arm64_rcg_node_t* rn       = neighbor->data;

			if (rn->dag_node) {
				if (rn->dag_node->color <= 0)
					continue;

				if (ARM64_COLOR_CONFLICT(r->color, rn->dag_node->color))
					break;
			} else {
				assert(rn->reg);

				if (ARM64_COLOR_CONFLICT(r->color, rn->reg->color))
					break;
			}
		}

		if (j == neighbors->size)
			free_regs[nb_free_regs++] = r;
	}

	if (nb_free_regs > 0)
		return _arm64_reg_cached_min_vars(free_regs, nb_free_regs);

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_arm64_t*	r = &(arm64_registers[i]);

		if (SCF_ARM64_REG_SP == r->id
				|| SCF_ARM64_REG_FP == r->id
				|| SCF_ARM64_REG_LR == r->id)
			continue;

		if (r->bytes < bytes || ARM64_COLOR_TYPE(r->color) != is_float)
			continue;

		if (c->dsts) {
			scf_3ac_operand_t* dst;

			for (j  = 0; j < c->dsts->size; j++) {
				dst =        c->dsts->data[j];

				if (dst->dag_node && dst->dag_node->color > 0 
						&& ARM64_COLOR_CONFLICT(r->color, dst->dag_node->color))
					break;
			}

			if (j < c->dsts->size)
				continue;
		}

		if (c->srcs) {
			scf_3ac_operand_t* src;

			for (j  = 0; j < c->srcs->size; j++) {
				src =        c->srcs->data[j];

				if (src->dag_node && src->dag_node->color > 0
						&& ARM64_COLOR_CONFLICT(r->color, src->dag_node->color))
					break;
			}

			if (j < c->srcs->size)
				continue;
		}

		return r;
	}

	return NULL;
}

static int _arm64_load_reg_const(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_instruction_t* inst;
	scf_arm64_OpCode_t*  lea;
	scf_arm64_OpCode_t*  mov;

	scf_variable_t*    v = dn->var;

	r->used = 1;

	int size = arm64_variable_size(v);

	if (SCF_FUNCTION_PTR == v->type) {

		assert(v->func_ptr);
		assert(v->const_literal_flag);

		v->global_flag = 1;
		v->local_flag  = 0;
		v->tmp_flag    = 0;

		return arm64_make_inst_ADR2G(c, f, r, v);

	} else if (v->nb_dimentions > 0) {
		assert(v->const_literal_flag);

		return arm64_make_inst_ADR2G(c, f, r, v);
	}

	return arm64_make_inst_I2G(c, r, v->data.u64, size);
}

int arm64_load_reg(scf_register_arm64_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	if (dn->loaded)
		return 0;

	scf_arm64_OpCode_t* mov;
	scf_instruction_t*  inst;
	scf_rela_t*         rela = NULL;

	int is_float = scf_variable_float(dn->var);
	int var_size = arm64_variable_size(dn->var);

	r->used = 1;

	if (!is_float) {

		if (scf_variable_const(dn->var)) {

			int ret = _arm64_load_reg_const(r, dn, c, f);
			if (ret < 0)
				return ret;

			dn->loaded = 1;
			return 0;
		}

		if (!dn->var->global_flag && !dn->var->local_flag && !dn->var->tmp_flag)
			return 0;

		if (scf_variable_const_string(dn->var)) {

			int ret = arm64_make_inst_ISTR2G(c, f, r, dn->var);
			if (ret < 0)
				return ret;

			dn->loaded = 1;
			return 0;
		}

		if (0 == dn->var->bp_offset && !dn->var->global_flag) {
			scf_loge("\n");
			return -EINVAL;
		}

		if ((dn->var->nb_dimentions > 0 && dn->var->const_literal_flag)
				|| (dn->var->type >= SCF_STRUCT && 0 == dn->var->nb_pointers)) {

			int ret = arm64_make_inst_ADR2G(c, f, r, dn->var);
			if (ret < 0)
				return ret;

			dn->loaded = 1;
			return 0;
		}

		int ret = arm64_make_inst_M2G(c, f, r, NULL, dn->var);
		if (ret < 0)
			return ret;

		dn->loaded = 1;
		return 0;
	}

	if (!dn->var->global_flag && !dn->var->local_flag && !dn->var->tmp_flag)
		return 0;

	if (0 == dn->var->bp_offset && !dn->var->global_flag) {
		scf_loge("\n");
		return -EINVAL;
	}

	int ret = arm64_make_inst_M2GF(c, f, r, NULL, dn->var);
	if (ret < 0)
		return ret;

	dn->loaded = 1;
	return 0;
}

int arm64_select_reg(scf_register_arm64_t** preg, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f, int load_flag)
{
	if (0 == dn->color)
		return -EINVAL;

	scf_register_arm64_t* r;

	int ret;
	int is_float = scf_variable_float(dn->var);
	int var_size = arm64_variable_size(dn->var);

	if (dn->color > 0) {
		r   = arm64_find_register_color(dn->color);
#if 1
		ret = _arm64_overflow_reg3(r, dn, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return -1;
		}
#endif
	} else {
		r   = arm64_select_overflowed_reg(dn, c, is_float);
		if (!r) {
			scf_loge("\n");
			return -1;
		}

		ret = arm64_overflow_reg(r, c, f);
		if (ret < 0) {
			scf_loge("overflow reg failed\n");
			return ret;
		}
		assert(0 == r->dag_nodes->size);

		r = arm64_find_register_type_id_bytes(is_float, r->id, var_size);
		assert(0 == r->dag_nodes->size);

		dn->color = r->color;
	}

	ret = scf_vector_add_unique(r->dag_nodes, dn);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	if (load_flag) {
		ret = arm64_load_reg(r, dn, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	} else
		dn->loaded = 1;

	r->used = 1;
	*preg = r;
	return 0;
}

int arm64_select_free_reg(scf_register_arm64_t** preg, scf_3ac_code_t* c, scf_function_t* f, int is_float)
{
	scf_register_arm64_t* r;

	r   = arm64_select_overflowed_reg(NULL, c, is_float);
	if (!r) {
		scf_loge("\n");
		return -1;
	}

	int ret = arm64_overflow_reg(r, c, f);
	if (ret < 0) {
		scf_loge("overflow reg failed\n");
		return ret;
	}
	assert(0 == r->dag_nodes->size);

	r = arm64_find_register_type_id_bytes(0, r->id, 8);
	assert(0 == r->dag_nodes->size);

	ret = arm64_rcg_make(c, c->rcg, NULL, r);
	if (ret < 0)
		return ret;

	r->used = 1;

	*preg = r;
	return 0;
}

int arm64_dereference_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_arm64_t* rb = NULL;
	scf_variable_t*     vb = base->var;

	scf_logw("base->color: %ld\n", base->color);

	int ret  = arm64_select_reg(&rb, base, c, f, 1);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}
	scf_logw("base->color: %ld\n", base->color);

	if (vb->nb_pointers + vb->nb_dimentions > 1 || vb->type >= SCF_STRUCT)
		sib->size = 8;
	else {
		sib->size = vb->data_size;
		assert(8 >= vb->data_size);
	}

	sib->base  = rb;
	sib->index = NULL;
	sib->scale = 0;
	sib->disp  = 0;
	return 0;
}

int arm64_pointer_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* member, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_variable_t*       vb = base  ->var;
	scf_variable_t*       vm = member->var;
	scf_register_arm64_t* rb = NULL;
	scf_instruction_t*    inst;

	int     ret;
	int32_t disp = 0;

	if (vb->nb_pointers > 0 && 0 == vb->nb_dimentions) {

		ret = arm64_select_reg(&rb, base, c, f, 1);
		if (ret < 0)
			return ret;

	} else if (vb->local_flag) {

		rb   = arm64_find_register("fp");
		disp = vb->bp_offset;

	} else if (vb->global_flag) {

		ret  = arm64_select_reg(&rb, base, c, f, 0);
		if (ret < 0)
			return ret;

		ret = arm64_make_inst_ADR2G(c, f, rb, vb);
		if (ret < 0)
			return ret;

	} else {
		ret = arm64_select_reg(&rb, base, c, f, 0);
		if (ret < 0)
			return ret;
	}

	disp += vm->offset;

	sib->base  = rb;
	sib->index = NULL;
	sib->scale = 0;
	sib->disp  = disp;
	sib->size  = arm64_variable_size(vm);
	return 0;
}

int arm64_array_index_reg(arm64_sib_t* sib, scf_dag_node_t* base, scf_dag_node_t* index, scf_dag_node_t* scale, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_variable_t*     vb   = base ->var;
	scf_variable_t*     vi   = index->var;

	scf_register_arm64_t* rb = NULL;
	scf_register_arm64_t* ri = NULL;
	scf_register_arm64_t* rs = NULL;
	scf_register_arm64_t* rd = NULL;

	scf_arm64_OpCode_t*   xor;
	scf_arm64_OpCode_t*   add;
	scf_arm64_OpCode_t*   shl;
	scf_arm64_OpCode_t*   lea;
	scf_arm64_OpCode_t*   mov;
	scf_instruction_t*  inst;

	int ret;
	int i;

	uint32_t opcode;
	int32_t  disp = 0;

	if (vb->nb_pointers > 0 && 0 == vb->nb_dimentions) {

		ret = arm64_select_reg(&rb, base, c, f, 1);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	} else if (vb->local_flag) {

		rb   = arm64_find_register("fp");
		disp = vb->bp_offset;

	} else if (vb->global_flag) {
		scf_rela_t* rela = NULL;

		if (0 == base->color)
			base->color = -1;

		ret  = arm64_select_reg(&rb, base, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}

		opcode = (0x90 << 24) | rb->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vb, NULL);
		rela->type = R_AARCH64_ADR_PREL_PG_HI21;

		opcode = (0x91 << 24) | (rb->id << 5) | rb->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);
		ARM64_RELA_ADD_CHECK(f->data_relas, rela, c, vb, NULL);
		rela->type = R_AARCH64_ADD_ABS_LO12_NC;

	} else {
		ret = arm64_select_reg(&rb, base, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	int32_t s = scale->var->data.i;
	assert(s > 0);

	if (vb->nb_pointers + vb->nb_dimentions > 1 || vb->type >= SCF_STRUCT)
		sib->size = 8;
	else {
		sib->size = vb->data_size;
		assert(8 >= vb->data_size);
	}

	if (0 == index->color) {
		disp += vi->data.i * s;

		sib->base  = rb;
		sib->index = NULL;
		sib->scale = 0;
		sib->disp  = disp;
		return 0;
	}

	ret = arm64_select_reg(&ri, index, c, f, 1);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	scf_register_arm64_t* ri2 = arm64_find_register_color_bytes(ri->color, 8);

	if (ri->bytes < ri2->bytes) {

		if (scf_variable_signed(index->var)) {

			opcode = (0x93 << 24) | (0x1 << 22) | (0x1f << 10) | (ri->id << 5) | ri->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);
		}

		ri = ri2;
	}

	if (1 != s
			&& 2 != s
			&& 4 != s
			&& 8 != s) {

		assert(8 == scale->var->size);

		ret = arm64_select_reg(&rs, scale, c, f, 0);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}

		opcode = (0xd2 << 24) | (0x1 << 23) | ((s & 0xffff) << 5) | rs->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		opcode = (0xf2 << 24) | (0x1 << 23) | (((s >> 16) & 0xffff) << 5) | rs->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		scf_logw("s: %d\n", s);

		opcode = (0x9b << 24) | (ri->id << 16) | (0x1f << 10) | (rs->id << 5) | rs->id;
		inst   = arm64_make_inst(c, opcode);
		ARM64_INST_ADD_CHECK(c->instructions, inst);

		ri = rs;
		s  = 1;
	}

	if (disp != 0) {

		if (!rs) {
			ret = arm64_select_reg(&rs, scale, c, f, 0);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}

			if (disp > 0 && disp <= 0xfff)
				opcode = (0x91 << 24) | (disp << 10) | (rb->id << 5) | rs->id;

			else if (disp < 0 && -disp <= 0xfff)
				opcode = (0xd1 << 24) | ((-disp) << 10) | (rb->id << 5) | rs->id;

			else {
				ret = arm64_make_inst_I2G(c, rs, disp, 4);
				if (ret < 0)
					return ret;

				opcode = (0x8b << 24) | (rb->id << 16) | (rs->id << 5) | rs->id;
			}

			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);

		} else {
			assert(1 == s);

			if (disp > 0 && disp <= 0xfff)
				opcode = (0x91 << 24) | (disp << 10) | (rs->id << 5) | rs->id;

			else if (disp < 0 && -disp <= 0xfff)
				opcode = (0xd1 << 24) | ((-disp) << 10) | (rs->id << 5) | rs->id;

			else {
				ret = arm64_select_free_reg(&rd, c, f, 0);
				if (ret < 0)
					return ret;

				ret = arm64_make_inst_I2G(c, rd, disp, 4);
				if (ret < 0)
					return ret;

				opcode = (0x8b << 24) | (rd->id << 16) | (rs->id << 5) | rs->id;
				inst   = arm64_make_inst(c, opcode);
				ARM64_INST_ADD_CHECK(c->instructions, inst);
			}

			opcode = (0x8b << 24) | (rb->id << 16) | (rs->id << 5) | rs->id;
			inst   = arm64_make_inst(c, opcode);
			ARM64_INST_ADD_CHECK(c->instructions, inst);

			ri = NULL;
		}

		rb = rs;
	}

	sib->base  = rb;
	sib->index = ri;
	sib->scale = s;
	sib->disp  = 0;
	return 0;
}

void arm64_call_rabi(int* p_nints, int* p_nfloats, scf_3ac_code_t* c)
{
	scf_3ac_operand_t* src = NULL;
	scf_dag_node_t*    dn  = NULL;

	int nfloats = 0;
	int nints   = 0;
	int i;

	for (i  = 1; i < c->srcs->size; i++) {
		src =        c->srcs->data[i];
		dn  =        src->dag_node;

		int is_float = scf_variable_float(dn->var);
		int size     = arm64_variable_size (dn->var);

		if (is_float) {
			if (nfloats < ARM64_ABI_NB)
				dn->rabi2 = arm64_find_register_type_id_bytes(is_float, arm64_abi_float_regs[nfloats++], size);
			else
				dn->rabi2 = NULL;
		} else {
			if (nints < ARM64_ABI_NB)
				dn->rabi2 = arm64_find_register_type_id_bytes(is_float, arm64_abi_regs[nints++], size);
			else
				dn->rabi2 = NULL;
		}

		src->rabi = dn->rabi2;
	}

	if (p_nints)
		*p_nints = nints;

	if (p_nfloats)
		*p_nfloats = nfloats;
}

