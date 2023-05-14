#include"scf_risc.h"

#define SCF_RISC_REG_FP 11
#define SCF_RISC_REG_SP 13
#define SCF_RISC_REG_LR 14
#define SCF_RISC_REG_PC 15

scf_register_t	arm32_registers[] = {

	{0, 1, "b0",    RISC_COLOR(0, 0, 0x1),  NULL, 0, 0},
	{0, 2, "h0",    RISC_COLOR(0, 0, 0x3),  NULL, 0, 0},
	{0, 4, "x0",    RISC_COLOR(0, 0, 0xf),  NULL, 0, 0},

	{1, 1, "b1",    RISC_COLOR(0, 1, 0x1),  NULL, 0, 0},
	{1, 2, "h1",    RISC_COLOR(0, 1, 0x3),  NULL, 0, 0},
	{1, 4, "x1",    RISC_COLOR(0, 1, 0xf),  NULL, 0, 0},

	{2, 1, "b2",    RISC_COLOR(0, 2, 0x1),  NULL, 0, 0},
	{2, 2, "h2",    RISC_COLOR(0, 2, 0x3),  NULL, 0, 0},
	{2, 4, "x2",    RISC_COLOR(0, 2, 0xf),  NULL, 0, 0},

	{3, 1, "b3",    RISC_COLOR(0, 3, 0x1),  NULL, 0, 0},
	{3, 2, "h3",    RISC_COLOR(0, 3, 0x3),  NULL, 0, 0},
	{3, 4, "x3",    RISC_COLOR(0, 3, 0xf),  NULL, 0, 0},

	{4, 1, "b4",    RISC_COLOR(0, 4, 0x1),  NULL, 0, 0},
	{4, 2, "h4",    RISC_COLOR(0, 4, 0x3),  NULL, 0, 0},
	{4, 4, "x4",    RISC_COLOR(0, 4, 0xf),  NULL, 0, 0},

	{5, 1, "b5",    RISC_COLOR(0, 5, 0x1),  NULL, 0, 0},
	{5, 2, "h5",    RISC_COLOR(0, 5, 0x3),  NULL, 0, 0},
	{5, 4, "x5",    RISC_COLOR(0, 5, 0xf),  NULL, 0, 0},

	{6, 1, "b6",    RISC_COLOR(0, 6, 0x1),  NULL, 0, 0},
	{6, 2, "h6",    RISC_COLOR(0, 6, 0x3),  NULL, 0, 0},
	{6, 4, "x6",    RISC_COLOR(0, 6, 0xf),  NULL, 0, 0},

	{7, 1, "b7",    RISC_COLOR(0, 7, 0x1),  NULL, 0, 0},
	{7, 2, "h7",    RISC_COLOR(0, 7, 0x3),  NULL, 0, 0},
	{7, 4, "x7",    RISC_COLOR(0, 7, 0xf),  NULL, 0, 0},

	{8, 1, "b8",    RISC_COLOR(0, 8, 0x1),  NULL, 0, 0},
	{8, 2, "h8",    RISC_COLOR(0, 8, 0x3),  NULL, 0, 0},
	{8, 4, "x8",    RISC_COLOR(0, 8, 0xf),  NULL, 0, 0},

	{9, 1, "b9",    RISC_COLOR(0, 9, 0x1),  NULL, 0, 0},
	{9, 2, "h9",    RISC_COLOR(0, 9, 0x3),  NULL, 0, 0},
	{9, 4, "x9",    RISC_COLOR(0, 9,  0xf),  NULL, 0, 0},

	{10, 1, "b10",  RISC_COLOR(0, 10, 0x1),  NULL, 0, 0},
	{10, 2, "h10",  RISC_COLOR(0, 10, 0x3),  NULL, 0, 0},
	{10, 4, "x10",  RISC_COLOR(0, 10, 0xf),  NULL, 0, 0},

	{11, 4, "fp",   RISC_COLOR(0, 11, 0xf),  NULL, 0, 0},

	{12, 4, "x12",  RISC_COLOR(0, 12, 0xf),  NULL, 0, 0},

	{13, 4, "sp",   RISC_COLOR(0, 13, 0xf),  NULL, 0, 0},

	{14, 4, "lr",   RISC_COLOR(0, 14, 0xf),  NULL, 0, 0},

//	{15, 4, "pc",   RISC_COLOR(0, 15, 0xf),  NULL, 0, 0},

#if 0
	{0, 2, "h0",    RISC_COLOR(1, 0, 0x3),  NULL, 0, 0},
	{0, 4, "s0",    RISC_COLOR(1, 0, 0xf),  NULL, 0, 0},
	{0, 8, "d0",    RISC_COLOR(1, 0, 0xff), NULL, 0, 0},

	{1, 2, "h1",    RISC_COLOR(1, 1, 0x3),  NULL, 0, 0},
	{1, 4, "s1",    RISC_COLOR(1, 1, 0xf),  NULL, 0, 0},
	{1, 8, "d1",    RISC_COLOR(1, 1, 0xff), NULL, 0, 0},

	{2, 2, "h2",    RISC_COLOR(1, 2, 0x3),  NULL, 0, 0},
	{2, 4, "s2",    RISC_COLOR(1, 2, 0xf),  NULL, 0, 0},
	{2, 8, "d2",    RISC_COLOR(1, 2, 0xff), NULL, 0, 0},

	{3, 2, "h3",    RISC_COLOR(1, 3, 0x3),  NULL, 0, 0},
	{3, 4, "s3",    RISC_COLOR(1, 3, 0xf),  NULL, 0, 0},
	{3, 8, "d3",    RISC_COLOR(1, 3, 0xff), NULL, 0, 0},

	{4, 2, "h4",    RISC_COLOR(1, 4, 0x3),  NULL, 0, 0},
	{4, 4, "s4",    RISC_COLOR(1, 4, 0xf),  NULL, 0, 0},
	{4, 8, "d4",    RISC_COLOR(1, 4, 0xff), NULL, 0, 0},

	{5, 2, "h5",    RISC_COLOR(1, 5, 0x3),  NULL, 0, 0},
	{5, 4, "s5",    RISC_COLOR(1, 5, 0xf),  NULL, 0, 0},
	{5, 8, "d5",    RISC_COLOR(1, 5, 0xff), NULL, 0, 0},

	{6, 2, "h6",    RISC_COLOR(1, 6, 0x3),  NULL, 0, 0},
	{6, 4, "s6",    RISC_COLOR(1, 6, 0xf),  NULL, 0, 0},
	{6, 8, "d6",    RISC_COLOR(1, 6, 0xff), NULL, 0, 0},

	{7, 2, "h7",    RISC_COLOR(1, 7, 0x3),  NULL, 0, 0},
	{7, 4, "s7",    RISC_COLOR(1, 7, 0xf),  NULL, 0, 0},
	{7, 8, "d7",    RISC_COLOR(1, 7, 0xff), NULL, 0, 0},

	{8, 2, "h8",    RISC_COLOR(1, 8, 0x3),  NULL, 0, 0},
	{8, 4, "s8",    RISC_COLOR(1, 8, 0xf),  NULL, 0, 0},
	{8, 8, "d8",    RISC_COLOR(1, 8, 0xff), NULL, 0, 0},

	{9, 2, "h9",    RISC_COLOR(1, 9, 0x3),  NULL, 0, 0},
	{9, 4, "s9",    RISC_COLOR(1, 9, 0xf),  NULL, 0, 0},
	{9, 8, "d9",    RISC_COLOR(1, 9, 0xff), NULL, 0, 0},

	{10, 2, "h10",    RISC_COLOR(1, 10, 0x3),  NULL, 0, 0},
	{10, 4, "s10",    RISC_COLOR(1, 10, 0xf),  NULL, 0, 0},
	{10, 8, "d10",    RISC_COLOR(1, 10, 0xff), NULL, 0, 0},

	{11, 2, "h11",    RISC_COLOR(1, 11, 0x3),  NULL, 0, 0},
	{11, 4, "s11",    RISC_COLOR(1, 11, 0xf),  NULL, 0, 0},
	{11, 8, "d11",    RISC_COLOR(1, 11, 0xff), NULL, 0, 0},

	{12, 2, "h12",    RISC_COLOR(1, 12, 0x3),  NULL, 0, 0},
	{12, 4, "s12",    RISC_COLOR(1, 12, 0xf),  NULL, 0, 0},
	{12, 8, "d12",    RISC_COLOR(1, 12, 0xff), NULL, 0, 0},

	{13, 2, "h13",    RISC_COLOR(1, 13, 0x3),  NULL, 0, 0},
	{13, 4, "s13",    RISC_COLOR(1, 13, 0xf),  NULL, 0, 0},
	{13, 8, "d13",    RISC_COLOR(1, 13, 0xff), NULL, 0, 0},

	{14, 2, "h14",    RISC_COLOR(1, 14, 0x3),  NULL, 0, 0},
	{14, 4, "s14",    RISC_COLOR(1, 14, 0xf),  NULL, 0, 0},
	{14, 8, "d14",    RISC_COLOR(1, 14, 0xff), NULL, 0, 0},

	{15, 2, "h15",    RISC_COLOR(1, 15, 0x3),  NULL, 0, 0},
	{15, 4, "s15",    RISC_COLOR(1, 15, 0xf),  NULL, 0, 0},
	{15, 8, "d15",    RISC_COLOR(1, 15, 0xff), NULL, 0, 0},

	{16, 2, "h16",    RISC_COLOR(1, 16, 0x3),  NULL, 0, 0},
	{16, 4, "s16",    RISC_COLOR(1, 16, 0xf),  NULL, 0, 0},
	{16, 8, "d16",    RISC_COLOR(1, 16, 0xff), NULL, 0, 0},

	{17, 2, "h17",    RISC_COLOR(1, 17, 0x3),  NULL, 0, 0},
	{17, 4, "s17",    RISC_COLOR(1, 17, 0xf),  NULL, 0, 0},
	{17, 8, "d17",    RISC_COLOR(1, 17, 0xff), NULL, 0, 0},

	{18, 2, "h18",    RISC_COLOR(1, 18, 0x3),  NULL, 0, 0},
	{18, 4, "s18",    RISC_COLOR(1, 18, 0xf),  NULL, 0, 0},
	{18, 8, "d18",    RISC_COLOR(1, 18, 0xff), NULL, 0, 0},

	{19, 2, "h19",    RISC_COLOR(1, 19, 0x3),  NULL, 0, 0},
	{19, 4, "s19",    RISC_COLOR(1, 19, 0xf),  NULL, 0, 0},
	{19, 8, "d19",    RISC_COLOR(1, 19, 0xff), NULL, 0, 0},

	{20, 2, "h20",    RISC_COLOR(1, 20, 0x3),  NULL, 0, 0},
	{20, 4, "s20",    RISC_COLOR(1, 20, 0xf),  NULL, 0, 0},
	{20, 8, "d20",    RISC_COLOR(1, 20, 0xff), NULL, 0, 0},

	{21, 2, "h21",    RISC_COLOR(1, 21, 0x3),  NULL, 0, 0},
	{21, 4, "s21",    RISC_COLOR(1, 21, 0xf),  NULL, 0, 0},
	{21, 8, "d21",    RISC_COLOR(1, 21, 0xff), NULL, 0, 0},

	{22, 2, "h22",    RISC_COLOR(1, 22, 0x3),  NULL, 0, 0},
	{22, 4, "s22",    RISC_COLOR(1, 22, 0xf),  NULL, 0, 0},
	{22, 8, "d22",    RISC_COLOR(1, 22, 0xff), NULL, 0, 0},

	{23, 2, "h23",    RISC_COLOR(1, 23, 0x3),  NULL, 0, 0},
	{23, 4, "s23",    RISC_COLOR(1, 23, 0xf),  NULL, 0, 0},
	{23, 8, "d23",    RISC_COLOR(1, 23, 0xff), NULL, 0, 0},

	{24, 2, "h24",    RISC_COLOR(1, 24, 0x3),  NULL, 0, 0},
	{24, 4, "s24",    RISC_COLOR(1, 24, 0xf),  NULL, 0, 0},
	{24, 8, "d24",    RISC_COLOR(1, 24, 0xff), NULL, 0, 0},

	{25, 2, "h25",    RISC_COLOR(1, 25, 0x3),  NULL, 0, 0},
	{25, 4, "s25",    RISC_COLOR(1, 25, 0xf),  NULL, 0, 0},
	{25, 8, "d25",    RISC_COLOR(1, 25, 0xff), NULL, 0, 0},

	{26, 2, "h26",    RISC_COLOR(1, 26, 0x3),  NULL, 0, 0},
	{26, 4, "s26",    RISC_COLOR(1, 26, 0xf),  NULL, 0, 0},
	{26, 8, "d26",    RISC_COLOR(1, 26, 0xff), NULL, 0, 0},

	{27, 2, "h27",    RISC_COLOR(1, 27, 0x3),  NULL, 0, 0},
	{27, 4, "s27",    RISC_COLOR(1, 27, 0xf),  NULL, 0, 0},
	{27, 8, "d27",    RISC_COLOR(1, 27, 0xff), NULL, 0, 0},

	{28, 2, "h28",    RISC_COLOR(1, 28, 0x3),  NULL, 0, 0},
	{28, 4, "s28",    RISC_COLOR(1, 28, 0xf),  NULL, 0, 0},
	{28, 8, "d28",    RISC_COLOR(1, 28, 0xff), NULL, 0, 0},

	{29, 2, "h29",    RISC_COLOR(1, 29, 0x3),  NULL, 0, 0},
	{29, 4, "s29",    RISC_COLOR(1, 29, 0xf),  NULL, 0, 0},
	{29, 8, "d29",    RISC_COLOR(1, 29, 0xff), NULL, 0, 0},

	{30, 2, "h30",    RISC_COLOR(1, 30, 0x3),  NULL, 0, 0},
	{30, 4, "s30",    RISC_COLOR(1, 30, 0xf),  NULL, 0, 0},
	{30, 8, "d30",    RISC_COLOR(1, 30, 0xff), NULL, 0, 0},

	{31, 2, "h31",    RISC_COLOR(1, 31, 0x3),  NULL, 0, 0},
	{31, 4, "s31",    RISC_COLOR(1, 31, 0xf),  NULL, 0, 0},
	{31, 8, "d31",    RISC_COLOR(1, 31, 0xff), NULL, 0, 0},
#endif
};

static uint32_t arm32_abi_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
};

static uint32_t arm32_abi_float_regs[] =
{
	SCF_RISC_REG_D0,
	SCF_RISC_REG_D1,
	SCF_RISC_REG_D2,
	SCF_RISC_REG_D3,
	SCF_RISC_REG_D4,
	SCF_RISC_REG_D5,
	SCF_RISC_REG_D6,
	SCF_RISC_REG_D7,
};

static uint32_t arm32_abi_ret_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
};

static uint32_t arm32_abi_caller_saves[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
};

static uint32_t arm32_abi_callee_saves[] =
{
	SCF_RISC_REG_X4,
	SCF_RISC_REG_X5,
	SCF_RISC_REG_X6,
	SCF_RISC_REG_X7,
	SCF_RISC_REG_X8,
	SCF_RISC_REG_X9,
	SCF_RISC_REG_X10,
	SCF_RISC_REG_X11,
	SCF_RISC_REG_X13,
	SCF_RISC_REG_X14,
};

static int arm32_variable_size(scf_variable_t* v)
{
	if (v->nb_dimentions > 0)
		return 4;

	if (v->nb_pointers > 0)
		return 4;

	if (v->type >= SCF_STRUCT)
		return 4;

	return v->size;
}

scf_register_t* arm32_find_register(const char* name)
{
	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (!strcmp(r->name, name))
			return r;
	}
	return NULL;
}

scf_register_t* arm32_find_register_type_id_bytes(uint32_t type, uint32_t id, int bytes)
{
	int i;

	if (bytes > 4 && 0 == type)
		bytes = 4;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (RISC_COLOR_TYPE(r->color) == type && r->id == id && r->bytes == bytes)
			return r;
	}
	return NULL;
}

scf_register_t* arm32_find_register_color(intptr_t color)
{
	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (r->color == color)
			return r;
	}
	return NULL;
}

scf_register_t* arm32_find_register_color_bytes(intptr_t color, int bytes)
{
	int i;

	if (bytes > 4 && 0 == RISC_COLOR_TYPE(color))
		bytes = 4;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (RISC_COLOR_CONFLICT(r->color, color) && r->bytes == bytes)
			return r;
	}
	return NULL;
}

scf_vector_t* arm32_register_colors()
{
	scf_vector_t* colors = scf_vector_alloc();
	if (!colors)
		return NULL;

	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X12 == r->id)
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

int arm32_reg_cached_vars(scf_register_t* r)
{
	int nb_vars = 0;
	int i;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r2 = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
         || SCF_RISC_REG_FP == r2->id
         || SCF_RISC_REG_LR == r2->id
		 || SCF_RISC_REG_X12 == r2->id)
			continue;

		if (!RISC_COLOR_CONFLICT(r->color, r2->color))
			continue;

		nb_vars += r2->dag_nodes->size;
	}

	return nb_vars;
}

int arm32_registers_init()
{
	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
         || SCF_RISC_REG_FP == r->id
         || SCF_RISC_REG_LR == r->id
		 || SCF_RISC_REG_X12 == r->id)
			continue;

		assert(!r->dag_nodes);

		r->dag_nodes = scf_vector_alloc();
		if (!r->dag_nodes)
			return -ENOMEM;

		r->used = 0;
	}

	return 0;
}

void arm32_registers_clear()
{
	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
         || SCF_RISC_REG_FP == r->id
         || SCF_RISC_REG_LR == r->id
		 || SCF_RISC_REG_X12 == r->id)
			continue;

		if (r->dag_nodes) {
			scf_vector_free(r->dag_nodes);
			r->dag_nodes = NULL;
		}

		r->used = 0;
	}
}

static scf_register_t* risc_reg_cached_min_vars(scf_register_t** regs, int nb_regs)
{
	scf_register_t* r_min = NULL;

	int min = 0;
	int i;

	for (i = 0; i < nb_regs; i++) {
		scf_register_t*	r = regs[i];

		int nb_vars = arm32_reg_cached_vars(r);

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

int arm32_caller_save_regs(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_t** saved_regs)
{
	int i;
	int j;
	scf_instruction_t* inst;
	scf_register_t*    r;
	scf_register_t*    r2;
	scf_register_t*    sp = arm32_find_register("sp");

	uint32_t opcode;

	int ret;
	int size = 0;
	int k    = 0;

	for (j = 0; j < nb_regs; j++) {
		r2 = arm32_find_register_type_id_bytes(0, regs[j], f->rops->MAX_BYTES);

		for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {
			r  = &(arm32_registers[i]);

			if (SCF_RISC_REG_SP == r->id
             || SCF_RISC_REG_FP == r->id
			 || SCF_RISC_REG_LR == r->id
			 || SCF_RISC_REG_X12 == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (RISC_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm32_registers) / sizeof(arm32_registers[0]))
			continue;

		if (stack_size > 0) {
			ret = f->iops->G2P(c, f, r2, sp, size + stack_size, r2->bytes);
			if (ret < 0)
				return ret;
		} else {
			inst   = f->iops->PUSH(NULL, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}

		saved_regs[k++] = r2;
		size += r2->bytes;
	}

	while (size & 0xf) {
		r2 = saved_regs[k - 1];

		if (stack_size > 0) {
			ret = f->iops->G2P(c, f, r2, sp, size + stack_size, r2->bytes);
			if (ret < 0)
				return ret;

		} else {
			inst   = f->iops->PUSH(NULL, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}

		saved_regs[k++] = r2;
		size += r2->bytes;
	}

	if (stack_size > 0) {
		for (j = 0; j < k / 2; j++) {

			i  = k - 1 - j;
			SCF_XCHG(saved_regs[i], saved_regs[j]);
		}
	}

	return size;
}

int arm32_pop_regs(scf_3ac_code_t* c, scf_function_t* f, scf_register_t** regs, int nb_regs, scf_register_t** updated_regs, int nb_updated)
{
	int i;
	int j;

	scf_register_t* sp = arm32_find_register("sp");
	scf_register_t* r;
	scf_register_t* r2;
	scf_instruction_t*    inst;

	for (j = nb_regs - 1; j >= 0; j--) {
		r2 = regs[j];

		for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {
			r  = &(arm32_registers[i]);

			if (SCF_RISC_REG_SP == r->id
             || SCF_RISC_REG_FP == r->id
             || SCF_RISC_REG_LR == r->id
			 || SCF_RISC_REG_X12 == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (RISC_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm32_registers) / sizeof(arm32_registers[0]))
			continue;

		for (i = 0; i < nb_updated; i++) {

			r  = updated_regs[i];

			if (RISC_COLOR_CONFLICT(r2->color, r->color))
				break;
		}

		if (i == nb_updated) {
			inst = f->iops->POP(c, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		} else {
			inst = f->iops->ADD_IMM(c, sp, sp, 4);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}
	}
	return 0;
}

int arm32_registers_reset()
{
	int i;
	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X12 == r->id)
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


int arm32_overflow_reg(scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f)
{
	int i;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r2 = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X12 == r2->id)
			continue;

		if (!RISC_COLOR_CONFLICT(r->color, r2->color))
			continue;

		int ret = risc_save_reg(r2, c, f);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}
	}

	r->used = 1;
	return 0;
}

int arm32_overflow_reg2(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		r2 = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X12 == r2->id)
			continue;

		if (!RISC_COLOR_CONFLICT(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; ) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 == dn) {
				j++;
				continue;
			}

			int ret = risc_save_var(dn2, c, f);
			if (ret < 0)
				return ret;
		}
	}

	r->used = 1;
	return 0;
}

int arm32_overflow_reg3(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_t*	r2;
	scf_dn_status_t*    ds2;
	scf_dag_node_t*     dn2;

	int i;
	int j;
	int ret;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		r2 = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X12 == r2->id)
			continue;

		if (!RISC_COLOR_CONFLICT(r->color, r2->color))
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
			int ret = risc_save_var(dn2, c, f);
			if (ret < 0)
				return ret;
		}
	}

	r->used = 1;
	return 0;
}

int arm32_reg_used(scf_register_t* r, scf_dag_node_t* dn)
{
	scf_register_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		r2 = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X12 == r2->id)
			continue;

		if (!RISC_COLOR_CONFLICT(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; j++) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 != dn)
				return 1;
		}
	}
	return 0;
}

scf_register_t* arm32_select_overflowed_reg(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float)
{
	scf_vector_t*       neighbors = NULL;
	scf_graph_node_t*   gn        = NULL;
	scf_register_t*     free_regs[sizeof(arm32_registers) / sizeof(arm32_registers[0])];

	int nb_free_regs = 0;
	int bytes        = 4;
	int ret;
	int i;
	int j;

	assert(c->rcg);

	if (dn) {
		is_float =   scf_variable_float(dn->var);
		bytes    = arm32_variable_size (dn->var);
	}

	ret = risc_rcg_find_node(&gn, c->rcg, dn, NULL);
	if (ret < 0)
		neighbors = c->rcg->nodes;
	else
		neighbors = gn->neighbors;

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X12 == r->id)
			continue;

		if (r->bytes < bytes || RISC_COLOR_TYPE(r->color) != is_float)
			continue;

		for (j = 0; j < neighbors->size; j++) {

			scf_graph_node_t* neighbor = neighbors->data[j];
			risc_rcg_node_t* rn       = neighbor->data;

			if (rn->dag_node) {
				if (rn->dag_node->color <= 0)
					continue;

				if (RISC_COLOR_CONFLICT(r->color, rn->dag_node->color))
					break;
			} else {
				assert(rn->reg);

				if (RISC_COLOR_CONFLICT(r->color, rn->reg->color))
					break;
			}
		}

		if (j == neighbors->size)
			free_regs[nb_free_regs++] = r;
	}

	if (nb_free_regs > 0)
		return risc_reg_cached_min_vars(free_regs, nb_free_regs);

	for (i = 0; i < sizeof(arm32_registers) / sizeof(arm32_registers[0]); i++) {

		scf_register_t*	r = &(arm32_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X12 == r->id)
			continue;

		if (r->bytes < bytes || RISC_COLOR_TYPE(r->color) != is_float)
			continue;

		if (c->dsts) {
			scf_3ac_operand_t* dst;

			for (j  = 0; j < c->dsts->size; j++) {
				dst =        c->dsts->data[j];

				if (dst->dag_node && dst->dag_node->color > 0 
						&& RISC_COLOR_CONFLICT(r->color, dst->dag_node->color))
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
						&& RISC_COLOR_CONFLICT(r->color, src->dag_node->color))
					break;
			}

			if (j < c->srcs->size)
				continue;
		}

		return r;
	}

	return NULL;
}

scf_regs_ops_t  regs_ops_arm32 =
{
	.name                        = "arm32",

	.abi_regs                    = arm32_abi_regs,
	.abi_float_regs              = arm32_abi_float_regs,
	.abi_ret_regs                = arm32_abi_ret_regs,
	.abi_caller_saves            = arm32_abi_caller_saves,
	.abi_callee_saves            = arm32_abi_callee_saves,

	.ABI_NB                      = sizeof(arm32_abi_regs)         / sizeof(uint32_t),
	.ABI_RET_NB                  = sizeof(arm32_abi_ret_regs)     / sizeof(uint32_t),
	.ABI_CALLER_SAVES_NB         = sizeof(arm32_abi_caller_saves) / sizeof(uint32_t),
	.ABI_CALLEE_SAVES_NB         = sizeof(arm32_abi_callee_saves) / sizeof(uint32_t),

	.MAX_BYTES                   = 4,

	.registers_init              = arm32_registers_init,
	.registers_reset             = arm32_registers_reset,
	.registers_clear             = arm32_registers_clear,
	.register_colors             = arm32_register_colors,

	.reg_used                    = arm32_reg_used,
	.reg_cached_vars             = arm32_reg_cached_vars,

	.variable_size               = arm32_variable_size,

	.caller_save_regs            = arm32_caller_save_regs,
	.pop_regs                    = arm32_pop_regs,

	.find_register               = arm32_find_register,
	.find_register_color         = arm32_find_register_color,
	.find_register_color_bytes   = arm32_find_register_color_bytes,
	.find_register_type_id_bytes = arm32_find_register_type_id_bytes,

	.select_overflowed_reg       = arm32_select_overflowed_reg,
	.overflow_reg                = arm32_overflow_reg,
	.overflow_reg2               = arm32_overflow_reg2,
	.overflow_reg3               = arm32_overflow_reg3,
};

