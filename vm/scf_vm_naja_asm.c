#include"scf_vm.h"

static int __naja_add(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;
		printf("add      r%d, r%d, %lu\n", rd, rs0, uimm15);
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		int      rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("add      r%d, r%d, r%d LSL %lu\n", rd, rs0, rs1, uimm8);
		else if (1 == sh)
			printf("add      r%d, r%d, r%d LSR %lu\n", rd, rs0, rs1, uimm8);
		else
			printf("add      r%d, r%d, r%d ASR %lu\n", rd, rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_sub(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;
		printf("sub      r%d, r%d, %lu\n", rd, rs0, uimm15);
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		int      rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("sub      r%d, r%d, r%d << %lu\n", rd, rs0, rs1, uimm8);
		else if (1 == sh)
			printf("sub      r%d, r%d, r%d LSR %lu\n", rd, rs0, rs1, uimm8);
		else
			printf("sub      r%d, r%d, r%d ASR %lu\n", rd, rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_cmp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int I   = (inst >> 20) & 0x1;

	int ret = 0;

	if (I) {
		int uimm15 = (inst >> 5) & 0x7fff;
		printf("cmp      r%d, %d\n", rs0, uimm15);
	} else {
		int sh     = (inst >> 18) & 0x3;
		int uimm8  = (inst >> 10) & 0xff;
		int rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("cmp      r%d, r%d LSL %d\n", rs0, rs1, uimm8);
		else if (1 == sh)
			printf("cmp      r%d, r%d LSR %d\n", rs0, rs1, uimm8);
		else
			printf("cmp      r%d, r%d ASR %d\n", rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_mul(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rs2 = (inst >> 10) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int S   = (inst >> 20) & 0x1;
	int opt = (inst >> 15) & 0x3;

	if (S) {
		if (0 == opt)
			printf("smadd    r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else if (1 == opt)
			printf("smsub    r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else
			printf("smul     r%d, r%d, r%d\n", rd, rs0, rs1);
	} else {
		if (0 == opt)
			printf("madd     r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else if (1 == opt)
			printf("msub     r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else
			printf("mul      r%d, r%d, r%d\n", rd, rs0, rs1);
	}

	return 0;
}

static int __naja_div(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rs2 = (inst >> 10) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int S   = (inst >> 20) & 0x1;
	int opt = (inst >> 15) & 0x3;

	if (S) {
		if (0 == opt)
			printf("sdadd    r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else if (1 == opt)
			printf("sdsub    r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else
			printf("sdiv     r%d, r%d, r%d\n", rd, rs0, rs1);
	} else {
		if (0 == opt)
			printf("dadd     r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else if (1 == opt)
			printf("dsub     r%d, r%d, r%d, r%d\n", rd, rs2, rs0, rs1);
		else
			printf("div      r%d, r%d, r%d\n", rd, rs0, rs1);
	}

	return 0;
}

static int __naja_ldr_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int A   = (inst >> 20) & 0x1;
	int ext = (inst >> 17) & 0x7;
	int s12 = (inst >>  5) & 0xfff;

	if (s12  & 0x800)
		s12 |= 0xfffff000;

	switch (ext) {
		case 0:
			if (A)
				printf("ldrb     r%d, [r%d, %d]!\n", rd, rb, s12);
			else
				printf("ldrb     r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 1:
			if (A)
				printf("ldrw     r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			else
				printf("ldrw     r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 2:
			if (A)
				printf("ldrl     r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			else
				printf("ldrl     r%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;

		case 3:
			if (A)
				printf("ldr      r%d, [r%d, %d]!\n", rd, rb, s12 << 3);
			else
				printf("ldr      r%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		case 4:
			if (A)
				printf("ldrsb    r%d, [r%d, %d]!\n", rd, rb, s12);
			else
				printf("ldrsb    r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 5:
			if (A)
				printf("ldrsw    r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			else
				printf("ldrsw    r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 6:
			if (A)
				printf("ldrsl    r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			else
				printf("ldrsl    r%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	return 0;
}

static int __naja_ldr_sib(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int ri  = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int ext = (inst >> 17) & 0x7;
	int u7  = (inst >> 10) & 0x7f;

	switch (ext) {
		case 0:
			printf("ldrb    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 1:
			printf("ldrw    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 2:
			printf("ldrl    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 3:
			printf("ldr     r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 4:
			printf("ldrsb   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 5:
			printf("ldrsw   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 6:
			printf("ldrsl   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	return 0;
}

static int __naja_str_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int A   = (inst >> 20) & 0x1;
	int ext = (inst >> 17) & 0x3;
	int s12 = (inst >>  5) & 0xfff;

	if (s12  & 0x800)
		s12 |= 0xfffff000;

	switch (ext) {
		case 0:
			if (A)
				printf("strb     r%d, [r%d, %d]!\n", rd, rb, s12);
			else
				printf("strb     r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 1:
			if (A)
				printf("strw     r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			else
				printf("strw     r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 2:
			if (A)
				printf("strl     r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			else
				printf("strl     r%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;

		case 3:
			if (A)
				printf("str      r%d, [r%d, %d]!\n", rd, rb, s12 << 3);
			else
				printf("str      r%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		default:
			scf_loge("\n");
			return -1;
			break;
	};

	return 0;
}

static int __naja_str_sib(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int ri  = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int ext = (inst >> 17) & 0x7;
	int u7  = (inst >> 10) & 0x7f;

	switch (ext) {
		case 0:
			printf("strb    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 1:
			printf("strw    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 2:
			printf("strl    r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 3:
			printf("str     r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		default:
			scf_loge("\n");
			return -1;
			break;
	};

	return 0;
}

static int __naja_and(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		printf("and     r%d, r%d, %#lx\n", rd, rs0, uimm15);
	} else {
		int sh     = (inst >> 18) & 0x3;
		int uimm8  = (inst >> 10) & 0xff;
		int rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("and     r%d, r%d, r%d LSL %#x\n", rd, rs0, rs1, uimm8);
		else if (1 == sh)
			printf("and     r%d, r%d, r%d LSR %#x\n", rd, rs0, rs1, uimm8);
		else
			printf("and     r%d, r%d, r%d ASR %#x\n", rd, rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_teq(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		printf("teq     r%d, %#lx\n", rs0, uimm15);
	} else {
		int sh     = (inst >> 18) & 0x3;
		int uimm8  = (inst >> 10) & 0xff;
		int rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("teq     r%d, r%d LSL %#x\n", rs0, rs1, uimm8);
		else if (1 == sh)
			printf("teq     r%d, r%d LSR %#x\n", rs0, rs1, uimm8);
		else
			printf("teq     r%d, r%d ASR %#x\n", rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_or(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		printf("or      r%d, r%d, %#lx\n", rd, rs0, uimm15);
	} else {
		int sh     = (inst >> 18) & 0x3;
		int uimm8  = (inst >> 10) & 0xff;
		int rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			printf("or      r%d, r%d, r%d LSL %#x\n", rd, rs0, rs1, uimm8);
		else if (1 == sh)
			printf("or      r%d, r%d, r%d LSR %#x\n", rd, rs0, rs1, uimm8);
		else
			printf("or      r%d, r%d, r%d ASR %#x\n", rd, rs0, rs1, uimm8);
	}

	return 0;
}

static int __naja_jmp_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int simm26 = inst & 0x3ffffff;

	if (simm26  & 0x2000000)
		simm26 |= 0xfc000000;

	uint64_t ip = naja->ip + (simm26 << 2);
	printf("jmp    %#lx\n", ip);
	return 0;
}

static int __naja_call_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int simm26 = inst & 0x3ffffff;

	if (simm26  & 0x2000000)
		simm26 |= 0xfc000000;

	uint64_t ip = naja->ip + (simm26 << 2);
	printf("call     %#lx\n", ip);
	return 0;
}

static int __naja_jmp_reg(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	if (inst & 0x1) {

		int cc  = (inst >> 1) & 0xf;
		int s21 = (inst >> 5) & 0x1fffff;

		if (s21  & 0x100000)
			s21 |= 0xffe00000;

		s21 <<= 2;

		uint64_t ip = naja->ip + s21;

		if (0 == cc)
			printf("jz       %#lx\n", ip);

		else if (1 == cc)
			printf("jnz      %#lx\n", ip);

		else if (2 == cc)
			printf("jge      %#lx\n", ip);

		else if (3 == cc)
			printf("jgt      %#lx\n", ip);

		else if (4 == cc)
			printf("jle      %#lx\n", ip);

		else if (5 == cc)
			printf("jlt      %#lx\n", ip);
		else {
			scf_loge("\n");
			return -EINVAL;
		}

	} else {
		int rd = (inst >> 21) & 0x1f;

		printf("jmp      *r%d\n", rd);
	}
	return 0;
}


static int __naja_call_reg(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd = (inst >> 21) & 0x1f;

	printf("call    r%d\n", rd);

	return 0;
}

static int __naja_adrp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd  = (inst >> 21) & 0x1f;
	int s21 =  inst & 0x1fffff;

	if (s21  & 0x100000)
		s21 |= ~0x1fffff;

	printf("adrp     r%d, [rip, %d]\n", rd, s21);

	return 0;
}

static int __naja_ret(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	printf("ret\n");
	return 0;
}

static int __naja_setcc(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd = (inst >> 21) & 0x1f;
	int cc = (inst >> 17) & 0xf;

	if (SCF_VM_Z == cc)
		printf("setz     r%d\n", rd);

	else if (SCF_VM_NZ == cc)
		printf("setnz    r%d\n", rd);

	else if (SCF_VM_GE == cc)
		printf("setge    r%d\n", rd);

	else if (SCF_VM_GT == cc)
		printf("setgt    r%d\n", rd);

	else if (SCF_VM_LT == cc)
		printf("setlt    r%d\n", rd);

	else if (SCF_VM_LE == cc)
		printf("setle    r%d\n", rd);
	else {
		scf_loge("inst: %#x\n", inst);
		return -EINVAL;
	}

	return 0;
}

static int __naja_mov(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;
	int X   = (inst >> 19) & 0x1;
	int opt = (inst >> 16) & 0x7;

	if (I) {
		if (0 == opt) {
			if (X && (inst & 0x8000))
				printf("movsb    r%d, %d\n", rd, inst & 0xffff);
			else
				printf("mov      r%d, %d\n", rd, inst & 0xffff);

		} else if (1 == opt)
			printf("mov      r%d, %d << 16\n", rd, inst & 0xffff);
		else if (2 == opt)
			printf("mov      r%d, %d << 32\n", rd, inst & 0xffff);
		else if (3 == opt)
			printf("mov      r%d, %d << 48\n", rd, inst & 0xffff);
		else if (7 == opt)
			printf("mvn      r%d, %d\n", rd, inst & 0xffff);

	} else {
		int rs  =  inst & 0x1f;
		int rs1 = (inst >> 5) & 0x1f;
		int u11 = (inst >> 5) & 0x7ff;

		if (0 == opt) {
			if (X)
				printf("mov      r%d, r%d LSL r%d\n", rd, rs, rs1);
			else {
				if (0 == u11)
					printf("mov      r%d, r%d\n", rd, rs);
				else
					printf("mov      r%d, r%d LSL %d\n", rd, rs, u11);
			}
		} else if (1 == opt) {
			if (X)
				printf("mov      r%d, r%d LSR r%d\n", rd, rs, rs1);
			else {
				if (0 == u11)
					printf("mov      r%d, r%d\n", rd, rs);
				else
					printf("mov      r%d, r%d LSR %d\n", rd, rs, u11);
			}

		} else if (2 == opt) {
			if (X)
				printf("mov      r%d, r%d ASR r%d\n", rd, rs, rs1);
			else {
				if (0 == u11)
					printf("mov      r%d, r%d\n", rd, rs);
				else
					printf("mov      r%d, r%d ASR %d\n", rd, rs, u11);
			}

		} else if (3 == opt)
			printf("NOT      r%d, r%d\n", rd, rs);
		else if (4 == opt)
			printf("NEG      r%d, r%d\n", rd, rs);

		else if (5 == opt) {
			if (X)
				printf("movsb    r%d, r%d\n", rd, rs);
			else
				printf("movzb    r%d, r%d\n", rd, rs);

		} else if (6 == opt) {
			if (X)
				printf("movsw    r%d, r%d\n", rd, rs);
			else
				printf("movzw    r%d, r%d\n", rd, rs);

		} else if (7 == opt) {
			if (X)
				printf("movsl    r%d, r%d\n", rd, rs);
			else
				printf("movzl    r%d, r%d\n", rd, rs);
		}
	}

	return 0;
}

static int __naja_fadd(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;

	printf("fadd   r%d, r%d, r%d\n", rd, rs0, rs1);

	naja->ip += 4;
	return 0;
}

static int __naja_fsub(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;

	printf("fsub   r%d, r%d, r%d\n", rd, rs0, rs1);

	naja->ip += 4;
	return 0;
}

static int __naja_fcmp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0  =  inst        & 0x1f;
	int rs1  = (inst >>  5) & 0x1f;

	printf("fcmp   d%d, d%d\n", rs0, rs1);

	naja->ip += 4;
	return 0;
}

static int __naja_fmul(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rs2 = (inst >> 10) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int opt = (inst >> 18) & 0x3;

	if (0 == opt)
		printf("fmadd   d%d, d%d, d%d, d%d", rd, rs2, rs0, rs1);
	else if (1 == opt)
		printf("fmsub   d%d, d%d, d%d, d%d", rd, rs2, rs0, rs1);
	else
		printf("fmul    d%d, d%d, d%d", rd, rs0, rs1);

	naja->ip += 4;
	return 0;
}

static int __naja_fdiv(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rs1 = (inst >>  5) & 0x1f;
	int rs2 = (inst >> 10) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int opt = (inst >> 18) & 0x3;

	if (0 == opt)
		printf("fdadd   d%d, d%d, d%d, d%d", rd, rs2, rs0, rs1);
	else if (1 == opt)
		printf("fdsub   d%d, d%d, d%d, d%d", rd, rs2, rs0, rs1);
	else
		printf("fdiv    d%d, d%d, d%d", rd, rs0, rs1);

	naja->ip += 4;
	return 0;
}

static int __naja_fstr_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int A   = (inst >> 20) & 0x1;
	int ext = (inst >> 17) & 0x7;
	int s12 = (inst >>  5) & 0xfff;

	if (s12  & 0x800)
		s12 |= 0xfffff000;

	switch (ext) {
		case 3:
			if (A)
				printf("fstr     d%d, [r%d, %d]!\n", rd, rb, s12 << 3);
			else
				printf("fstr     d%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		case 6:
			if (A)
				printf("fstrf    f%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			else
				printf("fstrf    f%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;
		default:
			scf_loge("ext: %d\n", ext);
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_fldr_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int A   = (inst >> 20) & 0x1;
	int ext = (inst >> 17) & 0x7;
	int s12 = (inst >>  5) & 0xfff;

	if (s12  & 0x800)
		s12 |= 0xfffff000;

	switch (ext) {
		case 3:
			if (A)
				printf("fldr     d%d, [r%d, %d]!\n", rd, rb, s12 << 3);
			else
				printf("fldr     d%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		case 6:
			if (A)
				printf("fldrf    f%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			else
				printf("fldrf    f%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;
		default:
			scf_loge("ext: %d\n", ext);
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_fldr_sib(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int ri  = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int ext = (inst >> 17) & 0x7;
	int u7  = (inst >> 10) & 0x7f;

	switch (ext) {
		case 3:
			printf("fldr  d%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;
		case 6:
			printf("fldrf f%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_fstr_sib(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int ri  = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int ext = (inst >> 17) & 0x3;
	int u7  = (inst >> 10) & 0x7f;

	switch (ext) {
		case 3:
			printf("fstr  d%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;

		case 6:
			printf("fldrf f%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_fmov(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs  =  inst & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int opt = (inst >> 16) & 0xf;

	if (0 == opt)
		printf("fmov     d%d, d%d\n", rd, rs);

	else if (1 == opt)
		printf("fss2sd   d%d, f%d\n", rd, rs);

	else if (2 == opt)
		printf("fsd2ss   d%d, f%d\n", rd, rs);

	else if (3 == opt)
		printf("fneg     d%d, d%d\n", rd, rs);

	else if (4 == opt)
		printf("cvtss2si r%d, f%d\n", rd, rs);

	else if (5 == opt)
		printf("cvtsd2si r%d, d%d\n", rd, rs);

	else if (6 == opt)
		printf("cvtss2ui r%d, f%d\n", rd, rs);

	else if (7 == opt)
		printf("cvtsd2ui r%d, d%d\n", rd, rs);

	else if (0xc == opt)
		printf("cvtsi2ss f%d, r%d\n", rd, rs);

	else if (0xd == opt)
		printf("cvtsi2sd d%d, r%d\n", rd, rs);

	else if (0xe == opt)
		printf("cvtui2ss f%d, r%d\n", rd, rs);

	else if (0xf == opt)
		printf("cvtui2sd f%d, r%d\n", rd, rs);
	else {
		scf_loge("\n");
		return -EINVAL;
	}

	naja->ip += 4;
	return 0;
}


static naja_opcode_pt  naja_opcodes[64] =
{
	__naja_add,      // 0
	__naja_sub,      // 1
	__naja_mul,      // 2
	__naja_div,      // 3
	__naja_ldr_disp, // 4
	__naja_str_disp, // 5
	__naja_and,      // 6
	__naja_or,       // 7
	__naja_jmp_disp, // 8
	__naja_cmp,      // 9
	__naja_jmp_reg,  //10
	__naja_setcc,    //11
	__naja_ldr_sib,  //12
	__naja_str_sib,  //13
	__naja_teq,      //14
	__naja_mov,      //15

	__naja_fadd,     //16
	__naja_fsub,     //17
	__naja_fmul,     //18
	__naja_fdiv,     //19
	__naja_fldr_disp,//20
	__naja_fstr_disp,//21
	NULL,            //22
	NULL,            //23
	__naja_call_disp,//24
	__naja_fcmp,     //25
	__naja_call_reg, //26
	NULL,            //27
	__naja_fldr_sib, //28
	__naja_fstr_sib, //29
	NULL,            //30
	__naja_fmov,     //31

	NULL,            //32
	NULL,            //33
	NULL,            //34
	NULL,            //35
	NULL,            //36
	NULL,            //37
	NULL,            //38
	NULL,            //39
	NULL,            //40
	NULL,            //41
	__naja_adrp,     //42
	NULL,            //43
	NULL,            //44
	NULL,            //45
	NULL,            //46
	NULL,            //47

	NULL,            //48
	NULL,            //49
	NULL,            //50
	NULL,            //51
	NULL,            //52
	NULL,            //53
	NULL,            //54
	NULL,            //55
	__naja_ret,      //56
	NULL,            //57
	NULL,            //58
	NULL,            //59
	NULL,            //60
	NULL,            //61
	NULL,            //62
	NULL,            //63
};

static int __naja_vm_run(scf_vm_t* vm, const char* path, const char* sys)
{
	scf_elf_sym_t* s;
	scf_vm_naja_t* naja = vm->priv;
	scf_vector_t*  syms = scf_vector_alloc();
	if (!syms)
		return -ENOMEM;

	int ret = scf_elf_read_syms(vm->elf, syms, ".symtab");
	if (ret < 0)
		return ret;

	int i;
	for (i = 0; i < syms->size; i++) {
		s  =        syms->data[i];

		if (ELF64_ST_TYPE(s->st_info) != STT_FUNC)
			continue;

		int64_t offset = s->st_value - vm->text->addr;

		if (offset >= vm->text->len)
			return -1;

		printf("\n%s: \n", s->name);
		int j;
		for (j = 0; j < s->st_size; j+= 4) {

			uint32_t inst = *(uint32_t*)(vm->text->data + offset + j);

			naja_opcode_pt pt = naja_opcodes[(inst >> 26) & 0x3f];

			if (!pt) {
				scf_loge("inst: %d, %#x\n", (inst >> 26) & 0x3f, inst);
				continue;
			}

			naja->ip = vm->text->addr + offset + j;

			printf("%4d, %#lx: ", j, naja->ip);

			ret = pt(vm, inst);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}
		}
	}

	return 0;
}

static int naja_vm_run(scf_vm_t* vm, const char* path, const char* sys)
{
	int ret = naja_vm_init(vm, path, sys);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	return __naja_vm_run(vm, path, sys);
}

scf_vm_ops_t  vm_ops_naja_asm =
{
	.name  = "naja_asm",
	.open  = naja_vm_open,
	.close = naja_vm_close,
	.run   = naja_vm_run,
};

