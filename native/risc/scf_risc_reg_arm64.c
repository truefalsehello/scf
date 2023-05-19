#include"scf_risc.h"

#define SCF_RISC_REG_FP 29
#define SCF_RISC_REG_LR 30
#define SCF_RISC_REG_SP 31

scf_register_t	arm64_registers[] = {

	{0, 4, "w0",    RISC_COLOR(0, 0, 0xf),  NULL, 0, 0},
	{0, 8, "x0",    RISC_COLOR(0, 0, 0xff), NULL, 0, 0},

	{1, 4, "w1",    RISC_COLOR(0, 1, 0xf),  NULL, 0, 0},
	{1, 8, "x1",    RISC_COLOR(0, 1, 0xff), NULL, 0, 0},

	{2, 4, "w2",    RISC_COLOR(0, 2, 0xf),  NULL, 0, 0},
	{2, 8, "x2",    RISC_COLOR(0, 2, 0xff), NULL, 0, 0},

	{3, 4, "w3",    RISC_COLOR(0, 3, 0xf),  NULL, 0, 0},
	{3, 8, "x3",    RISC_COLOR(0, 3, 0xff), NULL, 0, 0},

	{4, 4, "w4",    RISC_COLOR(0, 4, 0xf),  NULL, 0, 0},
	{4, 8, "x4",    RISC_COLOR(0, 4, 0xff), NULL, 0, 0},

	{5, 4, "w5",    RISC_COLOR(0, 5, 0xf),  NULL, 0, 0},
	{5, 8, "x5",    RISC_COLOR(0, 5, 0xff), NULL, 0, 0},

	{6, 4, "w6",    RISC_COLOR(0, 6, 0xf),  NULL, 0, 0},
	{6, 8, "x6",    RISC_COLOR(0, 6, 0xff), NULL, 0, 0},

	{7, 4, "w7",    RISC_COLOR(0, 7, 0xf),  NULL, 0, 0},
	{7, 8, "x7",    RISC_COLOR(0, 7, 0xff), NULL, 0, 0},

// not use x8

//	{8, 4, "w8",    RISC_COLOR(0, 8,  0xf),  NULL, 0},
//	{8, 8, "x8",    RISC_COLOR(0, 8,  0xff), NULL, 0},

	{9, 4, "w9",    RISC_COLOR(0, 9,  0xf),  NULL, 0, 0},
	{9, 8, "x9",    RISC_COLOR(0, 9,  0xff), NULL, 0, 0},

	{10, 4, "w10",  RISC_COLOR(0, 10, 0xf),  NULL, 0, 0},
	{10, 8, "x10",  RISC_COLOR(0, 10, 0xff), NULL, 0, 0},

	{11, 4, "w11",  RISC_COLOR(0, 11, 0xf),  NULL, 0, 0},
	{11, 8, "x11",  RISC_COLOR(0, 11, 0xff), NULL, 0, 0},

	{12, 4, "w12",  RISC_COLOR(0, 12, 0xf),  NULL, 0, 0},
	{12, 8, "x12",  RISC_COLOR(0, 12, 0xff), NULL, 0, 0},

	{13, 4, "w13",  RISC_COLOR(0, 13, 0xf),  NULL, 0, 0},
	{13, 8, "x13",  RISC_COLOR(0, 13, 0xff), NULL, 0, 0},

	{14, 4, "w14",  RISC_COLOR(0, 14, 0xf),  NULL, 0, 0},
	{14, 8, "x14",  RISC_COLOR(0, 14, 0xff), NULL, 0, 0},

	{15, 4, "w15",  RISC_COLOR(0, 15, 0xf),  NULL, 0, 0},
	{15, 8, "x15",  RISC_COLOR(0, 15, 0xff), NULL, 0, 0},

// not use x16, x17, x18

	{16, 4, "w16",  RISC_COLOR(0, 16, 0xf),  NULL, 0, 0},
	{16, 8, "x16",  RISC_COLOR(0, 16, 0xff), NULL, 0, 0},

	{17, 4, "w17",  RISC_COLOR(0, 17, 0xf),  NULL, 0, 0},
	{17, 8, "x17",  RISC_COLOR(0, 17, 0xff), NULL, 0, 0},

//	{18, 4, "w18",  RISC_COLOR(0, 18, 0xf),  NULL, 0, 0},
//	{18, 8, "x18",  RISC_COLOR(0, 18, 0xff), NULL, 0, 0},

	{19, 4, "w19",  RISC_COLOR(0, 19, 0xf),  NULL, 0, 0},
	{19, 8, "x19",  RISC_COLOR(0, 19, 0xff), NULL, 0, 0},

	{20, 4, "w20",  RISC_COLOR(0, 20, 0xf),  NULL, 0, 0},
	{20, 8, "x20",  RISC_COLOR(0, 20, 0xff), NULL, 0, 0},

	{21, 4, "w21",  RISC_COLOR(0, 21, 0xf),  NULL, 0, 0},
	{21, 8, "x21",  RISC_COLOR(0, 21, 0xff), NULL, 0, 0},

	{22, 4, "w22",  RISC_COLOR(0, 22, 0xf),  NULL, 0, 0},
	{22, 8, "x22",  RISC_COLOR(0, 22, 0xff), NULL, 0, 0},

	{23, 4, "w23",  RISC_COLOR(0, 23, 0xf),  NULL, 0, 0},
	{23, 8, "x23",  RISC_COLOR(0, 23, 0xff), NULL, 0, 0},

	{24, 4, "w24",  RISC_COLOR(0, 24, 0xf),  NULL, 0, 0},
	{24, 8, "x24",  RISC_COLOR(0, 24, 0xff), NULL, 0, 0},

	{25, 4, "w25",  RISC_COLOR(0, 25, 0xf),  NULL, 0, 0},
	{25, 8, "x25",  RISC_COLOR(0, 25, 0xff), NULL, 0, 0},

	{26, 4, "w26",  RISC_COLOR(0, 26, 0xf),  NULL, 0, 0},
	{26, 8, "x26",  RISC_COLOR(0, 26, 0xff), NULL, 0, 0},

	{27, 4, "w27",  RISC_COLOR(0, 27, 0xf),  NULL, 0, 0},
	{27, 8, "x27",  RISC_COLOR(0, 27, 0xff), NULL, 0, 0},

	{28, 4, "w28",  RISC_COLOR(0, 28, 0xf),  NULL, 0, 0},
	{28, 8, "x28",  RISC_COLOR(0, 28, 0xff), NULL, 0, 0},

// fp = x29 = bp
	{29, 4, "w29",  RISC_COLOR(0, 29, 0xf),  NULL, 0, 0},
	{29, 8, "fp",   RISC_COLOR(0, 29, 0xff), NULL, 0, 0},
// lr = x30
	{30, 4, "w30",  RISC_COLOR(0, 30, 0xf),  NULL, 0, 0},
	{30, 8, "lr",   RISC_COLOR(0, 30, 0xff), NULL, 0, 0},
	{31, 8, "sp",   RISC_COLOR(0, 31, 0xff), NULL, 0, 0},


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
};

static uint32_t arm64_abi_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
	SCF_RISC_REG_X4,
	SCF_RISC_REG_X5,
	SCF_RISC_REG_X6,
	SCF_RISC_REG_X7,
};

static uint32_t arm64_abi_float_regs[] =
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

static uint32_t arm64_abi_ret_regs[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
};

static uint32_t arm64_abi_caller_saves[] =
{
	SCF_RISC_REG_X0,
	SCF_RISC_REG_X1,
	SCF_RISC_REG_X2,
	SCF_RISC_REG_X3,
	SCF_RISC_REG_X4,
	SCF_RISC_REG_X5,
	SCF_RISC_REG_X6,
	SCF_RISC_REG_X7,

	SCF_RISC_REG_X9,
	SCF_RISC_REG_X10,
	SCF_RISC_REG_X11,
	SCF_RISC_REG_X12,
	SCF_RISC_REG_X13,
	SCF_RISC_REG_X14,
	SCF_RISC_REG_X15,
};

static uint32_t arm64_abi_callee_saves[] =
{
	SCF_RISC_REG_X19,
	SCF_RISC_REG_X20,
	SCF_RISC_REG_X21,
	SCF_RISC_REG_X22,
	SCF_RISC_REG_X23,
	SCF_RISC_REG_X24,
	SCF_RISC_REG_X25,
	SCF_RISC_REG_X26,
	SCF_RISC_REG_X27,
	SCF_RISC_REG_X28,
	SCF_RISC_REG_X29,
	SCF_RISC_REG_X30,
};

static int arm64_color_conflict(intptr_t c0, intptr_t c1)
{
	intptr_t id0 = c0 >> 16;
	intptr_t id1 = c1 >> 16;

	return id0 == id1 && (c0 & c1 & 0xffff);
}

static int arm64_variable_size(scf_variable_t* v)
{
	if (v->nb_dimentions > 0)
		return 8;

	if (v->type >= SCF_STRUCT && 0 == v->nb_pointers)
		return 8;

	return v->size < 4 ? 4 : v->size;
}

scf_register_t* arm64_find_register(const char* name)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (!strcmp(r->name, name))
			return r;
	}
	return NULL;
}

scf_register_t* arm64_find_register_type_id_bytes(uint32_t type, uint32_t id, int bytes)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (RISC_COLOR_TYPE(r->color) == type && r->id == id && r->bytes == bytes)
			return r;
	}
	return NULL;
}

scf_register_t* arm64_find_register_color(intptr_t color)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (r->color == color)
			return r;
	}
	return NULL;
}

scf_register_t* arm64_find_register_color_bytes(intptr_t color, int bytes)
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (arm64_color_conflict(r->color, color) && r->bytes == bytes)
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

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X16 == r->id
				|| SCF_RISC_REG_X17 == r->id)
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

int arm64_reg_cached_vars(scf_register_t* r)
{
	int nb_vars = 0;
	int i;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r2 = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
         || SCF_RISC_REG_FP == r2->id
         || SCF_RISC_REG_LR == r2->id
		 || SCF_RISC_REG_X16 == r2->id
		 || SCF_RISC_REG_X17 == r2->id)
			continue;

		if (!arm64_color_conflict(r->color, r2->color))
			continue;

		nb_vars += r2->dag_nodes->size;
	}

	return nb_vars;
}

int arm64_registers_init()
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
         || SCF_RISC_REG_FP == r->id
         || SCF_RISC_REG_LR == r->id
		 || SCF_RISC_REG_X16 == r->id
		 || SCF_RISC_REG_X17 == r->id)
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

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
         || SCF_RISC_REG_FP == r->id
         || SCF_RISC_REG_LR == r->id
		 || SCF_RISC_REG_X16 == r->id
		 || SCF_RISC_REG_X17 == r->id)
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

int arm64_caller_save_regs(scf_3ac_code_t* c, scf_function_t* f, uint32_t* regs, int nb_regs, int stack_size, scf_register_t** saved_regs)
{
	int i;
	int j;
	scf_register_t* r;
	scf_register_t* r2;
	scf_instruction_t*    inst;
	scf_register_t* sp   = arm64_find_register("sp");

	uint32_t opcode;

	int ret;
	int size = 0;
	int k    = 0;

	for (j = 0; j < nb_regs; j++) {
		r2 = arm64_find_register_type_id_bytes(0, regs[j], 8);

		for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {
			r  = &(arm64_registers[i]);

			if (SCF_RISC_REG_SP == r->id
             || SCF_RISC_REG_FP == r->id
			 || SCF_RISC_REG_LR == r->id
			 || SCF_RISC_REG_X16 == r->id
			 || SCF_RISC_REG_X17 == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (arm64_color_conflict(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm64_registers) / sizeof(arm64_registers[0]))
			continue;

		if (stack_size > 0) {
			ret = f->iops->G2P(c, f, r2, sp, size + stack_size, 8);
			if (ret < 0)
				return ret;
		} else {
			inst   = f->iops->PUSH(NULL, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}

		saved_regs[k++] = r2;
		size += 8;
	}

	if (size & 0xf) {
		r2 = saved_regs[k - 1];

		if (stack_size > 0) {
			ret = f->iops->G2P(c, f, r2, sp, size + stack_size, 8);
			if (ret < 0)
				return ret;
		} else {
			inst   = f->iops->PUSH(NULL, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
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

int arm64_pop_regs(scf_3ac_code_t* c, scf_function_t* f, scf_register_t** regs, int nb_regs, scf_register_t** updated_regs, int nb_updated)
{
	int i;
	int j;

	scf_register_t* sp = arm64_find_register("sp");
	scf_register_t* r;
	scf_register_t* r2;
	scf_instruction_t*    inst;

	for (j = nb_regs - 1; j >= 0; j--) {
		r2 = regs[j];

		for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {
			r  = &(arm64_registers[i]);

			if (SCF_RISC_REG_SP == r->id
             || SCF_RISC_REG_FP == r->id
             || SCF_RISC_REG_LR == r->id
			 || SCF_RISC_REG_X16 == r->id
			 || SCF_RISC_REG_X17 == r->id)
				continue;

			if (0 == r->dag_nodes->size)
				continue;

			if (arm64_color_conflict(r2->color, r->color))
				break;
		}

		if (i == sizeof(arm64_registers) / sizeof(arm64_registers[0]))
			continue;

		for (i = 0; i < nb_updated; i++) {

			r  = updated_regs[i];

			if (arm64_color_conflict(r2->color, r->color))
				break;
		}

		if (i == nb_updated) {
			inst = f->iops->POP(c, r2);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		} else {
			inst = f->iops->ADD_IMM(c, f, sp, sp, 8);
			RISC_INST_ADD_CHECK(c->instructions, inst);
		}
	}
	return 0;
}

int arm64_registers_reset()
{
	int i;
	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X16 == r->id
				|| SCF_RISC_REG_X17 == r->id)
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


int arm64_overflow_reg(scf_register_t* r, scf_3ac_code_t* c, scf_function_t* f)
{
	int i;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r2 = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X16 == r2->id
				|| SCF_RISC_REG_X17 == r2->id)
			continue;

		if (!arm64_color_conflict(r->color, r2->color))
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

int arm64_overflow_reg2(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X16 == r2->id
				|| SCF_RISC_REG_X17 == r2->id)
			continue;

		if (!arm64_color_conflict(r->color, r2->color))
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

int arm64_overflow_reg3(scf_register_t* r, scf_dag_node_t* dn, scf_3ac_code_t* c, scf_function_t* f)
{
	scf_register_t*	r2;
	scf_dn_status_t*    ds2;
	scf_dag_node_t*     dn2;

	int i;
	int j;
	int ret;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X16 == r2->id
				|| SCF_RISC_REG_X17 == r2->id)
			continue;

		if (!arm64_color_conflict(r->color, r2->color))
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

int arm64_reg_used(scf_register_t* r, scf_dag_node_t* dn)
{
	scf_register_t*	r2;
	scf_dag_node_t*     dn2;

	int i;
	int j;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		r2 = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r2->id
				|| SCF_RISC_REG_FP == r2->id
				|| SCF_RISC_REG_LR == r2->id
				|| SCF_RISC_REG_X16 == r2->id
				|| SCF_RISC_REG_X17 == r2->id)
			continue;

		if (!arm64_color_conflict(r->color, r2->color))
			continue;

		for (j  = 0; j < r2->dag_nodes->size; j++) {
			dn2 = r2->dag_nodes->data[j];

			if (dn2 != dn)
				return 1;
		}
	}
	return 0;
}

scf_register_t* arm64_select_overflowed_reg(scf_dag_node_t* dn, scf_3ac_code_t* c, int is_float)
{
	scf_vector_t*       neighbors = NULL;
	scf_graph_node_t*   gn        = NULL;

	scf_register_t* free_regs[sizeof(arm64_registers) / sizeof(arm64_registers[0])];

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

	ret = risc_rcg_find_node(&gn, c->rcg, dn, NULL);
	if (ret < 0)
		neighbors = c->rcg->nodes;
	else
		neighbors = gn->neighbors;

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X16 == r->id
				|| SCF_RISC_REG_X17 == r->id)
			continue;

		if (r->bytes < bytes || RISC_COLOR_TYPE(r->color) != is_float)
			continue;

		for (j = 0; j < neighbors->size; j++) {

			scf_graph_node_t* neighbor = neighbors->data[j];
			risc_rcg_node_t* rn       = neighbor->data;

			if (rn->dag_node) {
				if (rn->dag_node->color <= 0)
					continue;

				if (arm64_color_conflict(r->color, rn->dag_node->color))
					break;
			} else {
				assert(rn->reg);

				if (arm64_color_conflict(r->color, rn->reg->color))
					break;
			}
		}

		if (j == neighbors->size)
			free_regs[nb_free_regs++] = r;
	}

	if (nb_free_regs > 0)
		return risc_reg_cached_min_vars(free_regs, nb_free_regs);

	for (i = 0; i < sizeof(arm64_registers) / sizeof(arm64_registers[0]); i++) {

		scf_register_t*	r = &(arm64_registers[i]);

		if (SCF_RISC_REG_SP == r->id
				|| SCF_RISC_REG_FP == r->id
				|| SCF_RISC_REG_LR == r->id
				|| SCF_RISC_REG_X16 == r->id
				|| SCF_RISC_REG_X17 == r->id)
			continue;

		if (r->bytes < bytes || RISC_COLOR_TYPE(r->color) != is_float)
			continue;

		if (c->dsts) {
			scf_3ac_operand_t* dst;

			for (j  = 0; j < c->dsts->size; j++) {
				dst =        c->dsts->data[j];

				if (dst->dag_node && dst->dag_node->color > 0 
						&& arm64_color_conflict(r->color, dst->dag_node->color))
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
						&& arm64_color_conflict(r->color, src->dag_node->color))
					break;
			}

			if (j < c->srcs->size)
				continue;
		}

		return r;
	}

	return NULL;
}

static void arm64_argv_rabi(scf_function_t* f)
{
	scf_variable_t* v;

	f->args_int   = 0;
	f->args_float = 0;

	int bp_int    = -8;
	int bp_floats = -8 - (int)f->rops->ABI_NB * 8;
	int bp_others = 16;

	int i;
	for (i = 0; i < f->argv->size; i++) {
		v  =        f->argv->data[i];

		if (!v->arg_flag) {
			v ->arg_flag = 1;
			assert(f->inline_flag);
		}

		int is_float =      scf_variable_float(v);
		int size     = f->rops->variable_size (v);

		if (is_float) {

			if (f->args_float < f->rops->ABI_NB) {

				v->rabi       = f->rops->find_register_type_id_bytes(is_float, f->rops->abi_float_regs[f->args_float], size);
				v->bp_offset  = bp_floats;
				bp_floats    -= 8;
				f->args_float++;
				continue;
			}
		} else if (f->args_int < f->rops->ABI_NB) {

			v->rabi       = f->rops->find_register_type_id_bytes(is_float, f->rops->abi_regs[f->args_int], size);
			v->bp_offset  = bp_int;
			bp_int       -= 8;
			f->args_int++;
			continue;
		}

		v->rabi       = NULL;
		v->bp_offset  = bp_others;
		bp_others    += 8;
	}
}

void arm64_call_rabi(scf_3ac_code_t* c, scf_function_t* f, int* p_nints, int* p_nfloats, int* p_ndoubles)
{
	scf_3ac_operand_t* src = NULL;
	scf_dag_node_t*    dn  = NULL;

	int nfloats = 0;
	int nints   = 0;
	int i;

	for (i  = 1; i < c->srcs->size; i++) {
		src =        c->srcs->data[i];
		dn  =        src->dag_node;

		int is_float =      scf_variable_float(dn->var);
		int size     = f->rops->variable_size (dn->var);

		if (is_float) {
			if (nfloats   < f->rops->ABI_NB)
				dn->rabi2 = f->rops->find_register_type_id_bytes(is_float, f->rops->abi_float_regs[nfloats++], size);
			else
				dn->rabi2 = NULL;
		} else {
			if (nints     < f->rops->ABI_NB)
				dn->rabi2 = f->rops->find_register_type_id_bytes(is_float, f->rops->abi_regs[nints++], size);
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

int arm64_push_callee_regs(scf_3ac_code_t* c, scf_function_t* f)
{
	scf_instruction_t* inst;
	scf_register_t*    r;

	int i;
	for (i = 0; i < f->rops->ABI_CALLEE_SAVES_NB; i++) {

		r  = f->rops->find_register_type_id_bytes(0, f->rops->abi_callee_saves[i], 8);

		if (!r->used) {
			r  = f->rops->find_register_type_id_bytes(0, f->rops->abi_callee_saves[i], 4);

			if (!r->used)
				continue;
		}

		inst   = f->iops->PUSH(NULL, r);
		RISC_INST_ADD_CHECK(f->init_code->instructions, inst);

		f->init_code_bytes += inst->len;
	}

	return 0;
}

int arm64_pop_callee_regs(scf_3ac_code_t* c, scf_function_t* f)
{
	scf_instruction_t* inst;
	scf_register_t*    r;

	int i;
	for (i = f->rops->ABI_CALLEE_SAVES_NB - 1; i >= 0; i--) {

		r  = f->rops->find_register_type_id_bytes(0, f->rops->abi_callee_saves[i], 8);

		if (!r->used) {
			r  = f->rops->find_register_type_id_bytes(0, f->rops->abi_callee_saves[i], 4);

			if (!r->used)
				continue;
		}

		inst = f->iops->POP(c, r);
		RISC_INST_ADD_CHECK(c->instructions, inst);
	}

	return 0;
}

#define RISC_ABI_NB              (sizeof(arm64_abi_regs)        / sizeof(arm64_abi_regs[0]))
#define RISC_ABI_RET_NB          (sizeof(risc_abi_ret_regs)     / sizeof(risc_abi_ret_regs[0]))
#define RISC_ABI_CALLER_SAVES_NB (sizeof(risc_abi_caller_saves) / sizeof(risc_abi_caller_saves[0]))
#define RISC_ABI_CALLEE_SAVES_NB (sizeof(risc_abi_callee_saves) / sizeof(risc_abi_callee_saves[0]))

scf_regs_ops_t  regs_ops_arm64 =
{
	.name                        = "arm64",

	.abi_regs                    = arm64_abi_regs,
	.abi_float_regs              = arm64_abi_float_regs,
	.abi_ret_regs                = arm64_abi_ret_regs,
	.abi_caller_saves            = arm64_abi_caller_saves,
	.abi_callee_saves            = arm64_abi_callee_saves,

	.ABI_NB                      = sizeof(arm64_abi_regs)         / sizeof(uint32_t),
	.ABI_RET_NB                  = sizeof(arm64_abi_ret_regs)     / sizeof(uint32_t),
	.ABI_CALLER_SAVES_NB         = sizeof(arm64_abi_caller_saves) / sizeof(uint32_t),
	.ABI_CALLEE_SAVES_NB         = sizeof(arm64_abi_callee_saves) / sizeof(uint32_t),

	.MAX_BYTES                   = 8,

	.registers_init              = arm64_registers_init,
	.registers_reset             = arm64_registers_reset,
	.registers_clear             = arm64_registers_clear,
	.register_colors             = arm64_register_colors,

	.color_conflict              = arm64_color_conflict,

	.argv_rabi                   = arm64_argv_rabi,
	.call_rabi                   = arm64_call_rabi,

	.reg_used                    = arm64_reg_used,
	.reg_cached_vars             = arm64_reg_cached_vars,

	.variable_size               = arm64_variable_size,

	.caller_save_regs            = arm64_caller_save_regs,
	.pop_regs                    = arm64_pop_regs,

	.find_register               = arm64_find_register,
	.find_register_color         = arm64_find_register_color,
	.find_register_color_bytes   = arm64_find_register_color_bytes,
	.find_register_type_id_bytes = arm64_find_register_type_id_bytes,

	.select_overflowed_reg       = arm64_select_overflowed_reg,
	.overflow_reg                = arm64_overflow_reg,
	.overflow_reg2               = arm64_overflow_reg2,
	.overflow_reg3               = arm64_overflow_reg3,

	.push_callee_regs            = arm64_push_callee_regs,
	.pop_callee_regs             = arm64_pop_callee_regs,
};

scf_regs_ops_t  regs_ops_naja =
{
	.name                        = "naja",

	.abi_regs                    = arm64_abi_regs,
	.abi_float_regs              = arm64_abi_float_regs,
	.abi_ret_regs                = arm64_abi_ret_regs,
	.abi_caller_saves            = arm64_abi_caller_saves,
	.abi_callee_saves            = arm64_abi_callee_saves,

	.ABI_NB                      = sizeof(arm64_abi_regs)         / sizeof(uint32_t),
	.ABI_RET_NB                  = sizeof(arm64_abi_ret_regs)     / sizeof(uint32_t),
	.ABI_CALLER_SAVES_NB         = sizeof(arm64_abi_caller_saves) / sizeof(uint32_t),
	.ABI_CALLEE_SAVES_NB         = sizeof(arm64_abi_callee_saves) / sizeof(uint32_t),

	.MAX_BYTES                   = 8,

	.registers_init              = arm64_registers_init,
	.registers_reset             = arm64_registers_reset,
	.registers_clear             = arm64_registers_clear,
	.register_colors             = arm64_register_colors,

	.color_conflict              = arm64_color_conflict,

	.argv_rabi                   = arm64_argv_rabi,
	.call_rabi                   = arm64_call_rabi,

	.reg_used                    = arm64_reg_used,
	.reg_cached_vars             = arm64_reg_cached_vars,

	.variable_size               = arm64_variable_size,

	.caller_save_regs            = arm64_caller_save_regs,
	.pop_regs                    = arm64_pop_regs,

	.find_register               = arm64_find_register,
	.find_register_color         = arm64_find_register_color,
	.find_register_color_bytes   = arm64_find_register_color_bytes,
	.find_register_type_id_bytes = arm64_find_register_type_id_bytes,

	.select_overflowed_reg       = arm64_select_overflowed_reg,
	.overflow_reg                = arm64_overflow_reg,
	.overflow_reg2               = arm64_overflow_reg2,
	.overflow_reg3               = arm64_overflow_reg3,

	.push_callee_regs            = arm64_push_callee_regs,
	.pop_callee_regs             = arm64_pop_callee_regs,
};

