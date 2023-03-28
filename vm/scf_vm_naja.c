#include"scf_vm.h"

#define NAJA_REG_FP   29
#define NAJA_REG_LR   30
#define NAJA_REG_SP   31

static const char* somaps[][3] =
{
	{"x64", "/lib/ld-linux-aarch64.so.1",       "/lib64/ld-linux-x86-64.so.2"},
	{"x64", "/lib/aarch64-linux-gnu/libc.so.6", "/lib/x86_64-linux-gnu/libc.so.6"},
};

typedef int64_t (*naja_dyn_func_pt)(uint64_t r0,
		uint64_t r1,
		uint64_t r2,
		uint64_t r3,
		uint64_t r4,
		uint64_t r5,
		uint64_t r6,
		uint64_t r7);

int naja_vm_open(scf_vm_t* vm)
{
	if (!vm)
		return -EINVAL;

	scf_vm_naja_t* naja = calloc(1, sizeof(scf_vm_naja_t));
	if (!naja)
		return -ENOMEM;

	vm->priv = naja;
	return 0;
}

int naja_vm_close(scf_vm_t* vm)
{
	if (vm) {
		if (vm->priv) {
			free(vm->priv);
			vm->priv = NULL;
		}
	}

	return 0;
}

static int naja_vm_dynamic_link(scf_vm_t* vm)
{
	naja_dyn_func_pt  f    = NULL;
	scf_vm_naja_t*    naja = vm->priv;

	int64_t  sp = naja->regs[NAJA_REG_SP];

	uint64_t r30 = *(uint64_t*)(naja->stack - (sp +  8));
	uint64_t r16 = *(uint64_t*)(naja->stack - (sp + 16));

	scf_logw("sp: %ld, r16: %#lx, r30: %#lx, vm->jmprel_size: %ld\n", sp, r16, r30, vm->jmprel_size);

	if (r16  > (uint64_t)vm->data->data) {
		r16 -= (uint64_t)vm->data->data;
		r16 +=           vm->data->addr;
	}

	scf_logw("r16: %#lx, text: %p, rodata: %p, data: %p\n", r16, vm->text->data, vm->rodata->data, vm->data->data);

	int i;
	for (i = 0; i < vm->jmprel_size / sizeof(Elf64_Rela); i++) {

		if (r16  == vm->jmprel[i].r_offset) {

			int   j     = ELF64_R_SYM(vm->jmprel[i].r_info);

			char* fname = vm->dynstr + vm->dynsym[j].st_name;

			scf_logw("j: %d, %s\n", j, fname);

			int k;
			for (k = 0; k < vm->sofiles->size; k++) {

				f = dlsym(vm->sofiles->data[k], fname);
				if (f)
					break;
			}

			if (f) {
				int64_t offset = vm->jmprel[i].r_offset - vm->data->addr;

				if (offset < 0 || offset > vm->data->len) {
					scf_loge("\n");
					return -1;
				}

				*(void**)(vm->data->data + offset) = f;

				naja->regs[0] = f(naja->regs[0],
						naja->regs[1],
						naja->regs[2],
						naja->regs[3],
						naja->regs[4],
						naja->regs[5],
						naja->regs[6],
						naja->regs[7]);

				naja->regs[NAJA_REG_SP] += 16;
				return 0;
			}
			break;
		}
	}

	return -1;
}

int naja_vm_init(scf_vm_t* vm, const char* path, const char* sys)
{
	if (!vm || !path)
		return -EINVAL;

	if (vm->elf)
		scf_vm_clear(vm);

	if (vm->priv)
		memset(vm->priv, 0, sizeof(scf_vm_naja_t));
	else {
		vm->priv = calloc(1, sizeof(scf_vm_naja_t));
		if (!vm->priv)
			return -ENOMEM;
	}

	if (vm->phdrs)
		scf_vector_clear(vm->phdrs, (void (*)(void*) )free);
	else {
		vm->phdrs = scf_vector_alloc();
		if (!vm->phdrs)
			return -ENOMEM;
	}

	if (vm->sofiles)
		scf_vector_clear(vm->phdrs, (void (*)(void*) )dlclose);
	else {
		vm->sofiles = scf_vector_alloc();
		if (!vm->sofiles)
			return -ENOMEM;
	}

	int ret = scf_elf_open(&vm->elf, "naja", path, "rb");
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ret = scf_elf_read_phdrs(vm->elf, vm->phdrs);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	scf_elf_phdr_t* ph;
	int i;
	for (i = 0; i < vm->phdrs->size; i++) {
		ph =        vm->phdrs->data[i];

		if (PT_LOAD ==  ph->ph.p_type) {

			ph->addr = (ph->ph.p_vaddr + ph->ph.p_memsz) & ~(ph->ph.p_align - 1);
			ph->len  = (ph->ph.p_vaddr + ph->ph.p_memsz) - ph->addr;

			ph->data = calloc(1, ph->len);
			if (!ph->data)
				return -ENOMEM;

			fseek(vm->elf->fp, 0, SEEK_SET);

			ret = fread(ph->data, ph->len, 1, vm->elf->fp);
			if (1 != ret) {
				scf_loge("\n");
				return -1;
			}

			scf_loge("i: %d, ph->p_offset: %#lx, ph->p_filesz: %#lx\n", i, ph->ph.p_offset, ph->ph.p_filesz);

			scf_loge("i: %d, ph->addr: %#lx, ph->len: %#lx, %#lx, ph->flags: %#x\n", i, ph->addr, ph->len, ph->ph.p_memsz, ph->ph.p_flags);

			if ((PF_X | PF_R) == ph->ph.p_flags)
				vm->text   =  ph;

			else if ((PF_W | PF_R) == ph->ph.p_flags)
				vm->data   = ph;

			else if (PF_R == ph->ph.p_flags)
				vm->rodata =  ph;
			else {
				scf_loge("\n");
				return -1;
			}

		} else if (PT_DYNAMIC == ph->ph.p_type) {
			ph->addr = ph->ph.p_vaddr;
			ph->len  = ph->ph.p_memsz;

			vm->dynamic = ph;

			scf_loge("ph->addr: %#lx, ph->len: %#lx, %#lx, ph->p_offset: %#lx\n", ph->addr, ph->len, ph->ph.p_memsz, ph->ph.p_offset);
		}
	}

	scf_loge("\n\n");

	if (vm->dynamic) {
		Elf64_Dyn* d = (Elf64_Dyn*)(vm->data->data + vm->dynamic->ph.p_offset);

		vm->jmprel = NULL;
		for (i = 0; i < vm->dynamic->ph.p_filesz / sizeof(Elf64_Dyn); i++) {

			switch (d[i].d_tag) {

				case DT_STRTAB:
					scf_loge("dynstr: %#lx\n", d[i].d_un.d_ptr);
					vm->dynstr = d[i].d_un.d_ptr - vm->text->addr + vm->text->data;
					break;

				case DT_SYMTAB:
					scf_loge("dynsym: %#lx\n", d[i].d_un.d_ptr);
					vm->dynsym = (Elf64_Sym*)(d[i].d_un.d_ptr - vm->text->addr + vm->text->data);
					break;

				case DT_JMPREL:
					scf_loge("JMPREL: %#lx\n", d[i].d_un.d_ptr);
					vm->jmprel      = (Elf64_Rela*)(d[i].d_un.d_ptr - vm->text->addr + vm->text->data);
					vm->jmprel_addr = d[i].d_un.d_ptr;
					break;

				case DT_PLTGOT:
					scf_loge("PLTGOT: %#lx\n", d[i].d_un.d_ptr);
					vm->pltgot = (uint64_t*)(d[i].d_un.d_ptr - vm->data->addr + vm->data->data);
					break;

				default:
					break;
			};
		}

		for (i = 0; i < vm->dynamic->ph.p_filesz / sizeof(Elf64_Dyn); i++) {

			if (DT_NEEDED == d[i].d_tag) {

				uint8_t* name = d[i].d_un.d_ptr + vm->dynstr;

				int j;
				for (j = 0; j < sizeof(somaps) / sizeof(somaps[0]); j++) {

					if (!strcmp(somaps[j][0], sys)
							&& !strcmp(somaps[j][1], name)) {
						name  = somaps[j][2];
						break;
					}
				}

				scf_loge("needed: %s\n", name);

				void* so = dlopen(name, RTLD_LAZY);
				if (!so) {
					scf_loge("dlopen error, so: %s\n", name);
					return -1;
				}

				if (scf_vector_add(vm->sofiles, so) < 0) {
					dlclose(so);
					return -ENOMEM;
				}
			}
		}

		vm->pltgot[2] = (uint64_t)naja_vm_dynamic_link;
	}

	return 0;
}

static int __naja_add(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		naja->regs[rd]  = naja->regs[rs0] + uimm15;

		printf("add    r%d, r%d, %lu\n", rd, rs0, uimm15);
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		int      rs1    = (inst >>  5) & 0x1f;

		if (0 == sh) {
			naja->regs[rd]  = naja->regs[rs0] + (naja->regs[rs1] << uimm8);

			printf("add    r%d, r%d, r%d << %lu\n", rd, rs0, rs1, uimm8);

		} else if (1 == sh) {
			naja->regs[rd]  = naja->regs[rs0] + (naja->regs[rs1] >> uimm8);

			printf("add    r%d, r%d, r%d LSR %lu\n", rd, rs0, rs1, uimm8);

		} else {
			naja->regs[rd]  = naja->regs[rs0] + (((int64_t)naja->regs[rs1]) >> uimm8);

			printf("add    r%d, r%d, r%d ASR %lu\n", rd, rs0, rs1, uimm8);
		}
	}

	naja->ip += 4;
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

		naja->regs[rd]  = naja->regs[rs0] - uimm15;

		printf("sub    r%d, r%d, %lu\n", rd, rs0, uimm15);
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		int      rs1    = (inst >>  5) & 0x1f;

		if (0 == sh) {
			naja->regs[rd]  = naja->regs[rs0] - (naja->regs[rs1] << uimm8);

			printf("sub    r%d, r%d, r%d << %lu\n", rd, rs0, rs1, uimm8);

		} else if (1 == sh) {
			naja->regs[rd]  = naja->regs[rs0] - (naja->regs[rs1] >> uimm8);

			printf("sub    r%d, r%d, r%d LSR %lu\n", rd, rs0, rs1, uimm8);

		} else {
			naja->regs[rd]  = naja->regs[rs0] - (((int64_t)naja->regs[rs1]) >> uimm8);

			printf("sub    r%d, r%d, r%d ASR %lu\n", rd, rs0, rs1, uimm8);
		}
	}

	naja->ip += 4;
	return 0;
}

static int __naja_cmp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	int ret = 0;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		ret = naja->regs[rs0] - uimm15;

		printf("cmp    r%d, %ld,  rs0: %lx, ret: %d\n", rs0, uimm15, naja->regs[rs0], ret);

	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		int      rs1    = (inst >>  5) & 0x1f;

		if (0 == sh) {
			ret = naja->regs[rs0] - (naja->regs[rs1] << uimm8);

			printf("cmp    r%d, r%d LSL %ld,  rs0: %#lx, rs1: %#lx, ret: %d\n", rs0, rs1, uimm8, naja->regs[rs0], naja->regs[rs1], ret);

		} else if (1 == sh) {
			ret = naja->regs[rs0] - (naja->regs[rs1] >> uimm8);

			printf("cmp    r%d, r%d LSR %ld,  rs0: %#lx, rs1: %#lx, ret: %d\n", rs0, rs1, uimm8, naja->regs[rs0], naja->regs[rs1], ret);

		} else {
			ret = naja->regs[rs0] - (((int64_t)naja->regs[rs1]) >> uimm8);

			printf("cmp    r%d, r%d ASR %ld,  rs0: %#lx, rs1: %ld, ret: %d\n", rs0, rs1, uimm8, naja->regs[rs0], naja->regs[rs1], ret);
		}
	}

	if (0 == ret)
		naja->flags = 0x1;
	else if (ret > 0)
		naja->flags = 0x4;
	else
		naja->flags = 0x2;

	naja->ip += 4;
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

	scf_logw("\n");
	if (S)
		naja->regs[rd] = (int64_t)naja->regs[rs0] * (int64_t)naja->regs[rs1];
	else
		naja->regs[rd] = naja->regs[rs0] * naja->regs[rs1];

	if (0 == opt)
		naja->regs[rd] += naja->regs[rs2];
	else if (1 == opt)
		naja->regs[rd]  = naja->regs[rs2] - naja->regs[rd];

	naja->ip += 4;
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

	scf_logw("\n");
	if (S)
		naja->regs[rd] = (int64_t)naja->regs[rs0] / (int64_t)naja->regs[rs1];
	else
		naja->regs[rd] = naja->regs[rs0] / naja->regs[rs1];

	if (0 == opt)
		naja->regs[rd] += naja->regs[rs2];
	else if (1 == opt)
		naja->regs[rd]  = naja->regs[rs2] - naja->regs[rd];

	naja->ip += 4;
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

	scf_logd("rd: %d, rb: %d, s12: %d, ext: %d\n", rd, rb, s12, ext);

	int64_t  addr   = naja->regs[rb];
	int64_t  offset = 0;
	uint8_t* data;

	if (addr >= (int64_t)vm->data->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->data->data + vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->rodata->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->rodata->data + vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->text->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->text->data + vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr  >= 0x800000) {
		data   = vm->data->data;
		offset = addr - vm->data->addr;

		if (offset >= vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x600000) {
		data   = vm->rodata->data;
		offset = addr - vm->rodata->addr;

		if (offset >= vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x400000) {
		data   = vm->text->data;
		offset = addr - vm->text->addr;

		if (offset >= vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else {
		data   = naja->stack;
		offset = addr;
	}

	if (!A)
		offset += s12 << (ext & 0x3);

	if (data   == naja->stack) {
		offset = -offset;

		scf_logd("offset0: %ld, size: %ld\n", offset, naja->size);
		assert(offset >= 0);

		if (naja->size < offset) {
			scf_loge("offset: %ld, size: %ld\n", offset, naja->size);
			return -EINVAL;
		}

		offset -= 1 << (ext & 0x3);
	}

	switch (ext) {
		case 0:
			naja->regs[rd] = *(uint8_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12;
				printf("ldrb   r%d, [r%d, %d]!\n", rd, rb, s12);
			} else
				printf("ldrb   r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 1:
			naja->regs[rd] = *(uint16_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12 << 1;
				printf("ldrw   r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			} else
				printf("ldrw   r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 2:
			naja->regs[rd] = *(uint32_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12 << 2;
				printf("ldrl   r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			} else
				printf("ldrl   r%d, [r%d, %d],  %ld, %p\n", rd, rb, s12 << 2, naja->regs[rd], data + offset);
			break;

		case 3:
			naja->regs[rd] = *(uint64_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12 << 3;
				printf("ldr    r%d, [r%d, %d]!, rd: %#lx, rb: %ld, %p\n", rd, rb, s12 << 3, naja->regs[rd], naja->regs[rb], data + offset);
			} else
				printf("ldr    r%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		case 4:
			naja->regs[rd] = *(int8_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12;
				printf("ldrsb  r%d, [r%d, %d]!\n", rd, rb, s12);
			} else
				printf("ldrsb  r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 5:
			naja->regs[rd] = *(int16_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12 << 1;
				printf("ldrsw  r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			} else
				printf("ldrsw  r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 6:
			naja->regs[rd] = *(int32_t*)(data + offset);
			if (A) {
				naja->regs[rb] += s12 << 2;
				printf("ldrsl  r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			} else
				printf("ldrsl  r%d, [r%d, %d]\n", rd, rb, s12 << 2);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
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

	int64_t  addr   = naja->regs[rb];
	uint64_t offset;
	uint8_t* data;

	scf_logd("ldr   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);
	scf_logd("rd: %#lx\n", naja->regs[rd]);
	scf_logd("rb: %#lx\n", naja->regs[rb]);
	scf_logd("ri: %#lx\n", naja->regs[ri]);

	if (addr >= (int64_t)vm->data->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->data->data + vm->data->len) {
			scf_loge("addr: %#lx, %#lx\n", addr, (int64_t)vm->data->data + vm->data->len);
			return -1;
		}

	} else if (addr >= (int64_t)vm->rodata->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->rodata->data + vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->text->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->text->data + vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr  >= 0x800000) {
		data   = vm->data->data;
		offset = addr - vm->data->addr;

		if (offset >= vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x600000) {
		data   = vm->rodata->data;
		offset = addr - vm->rodata->addr;

		if (offset >= vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x400000) {
		data   = vm->text->data;
		offset = addr - vm->text->addr;

		if (offset >= vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else {
		data   = naja->stack;
		offset = addr;
	}

	offset += (naja->regs[ri] << u7);

	if (data   == naja->stack) {
		offset = -offset;

		assert(offset >= 0);

		if (naja->size < offset) {
			scf_loge("\n");
			return -1;
		}

		offset -= 1 << (ext & 0x3);
	}

	switch (ext) {
		case 0:
			printf("ldrb  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(uint8_t*)(data + offset);
			break;

		case 1:
			printf("ldrw  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(uint16_t*)(data + offset);
			break;

		case 2:
			printf("ldrl  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(uint32_t*)(data + offset);
			break;

		case 3:
			printf("ldr   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(uint64_t*)(data + offset);
			break;

		case 4:
			printf("ldrsb r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(int8_t*)(data + offset);
			break;

		case 5:
			printf("ldrsw r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(int16_t*)(data + offset);
			break;

		case 6:
			printf("ldrsl r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			naja->regs[rd] = *(int32_t*)(data + offset);
			break;
		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
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

	int64_t  addr   = naja->regs[rb];
	int64_t  offset = 0;
	uint8_t* data;

	if (addr >= (int64_t)vm->data->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->data->data + vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->rodata->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->rodata->data + vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->text->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->text->data + vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr  >= 0x800000) {
		data   = vm->data->data;
		offset = addr - vm->data->addr;

		scf_loge("rb: %d, offset: %ld, addr: %#lx, %#lx\n", rb, offset, addr, vm->data->addr);

		if (offset >= vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x600000) {
		data   = vm->rodata->data;
		offset = addr - vm->rodata->addr;

		if (offset >= vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x400000) {
		data   = vm->text->data;
		offset = addr - vm->text->addr;

		if (offset >= vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else {
		data   = naja->stack;
		offset = addr;
	}

	offset += s12 << ext;

	if (data   == naja->stack) {
		offset = -offset;

		scf_logd("offset0: %ld, size: %ld\n", offset, naja->size);

		assert(offset > 0);

		if (naja->size < offset) {
			data = realloc(naja->stack, offset + STACK_INC);
			if (!data)
				return -ENOMEM;

			naja->stack = data;
			naja->size  = offset + STACK_INC;
		}

		offset -= 1 << ext;
	}

	switch (ext) {
		case 0:
			*(uint8_t*)(data + offset) = naja->regs[rd];
			if (A) {
				naja->regs[rb] += s12;
				printf("strb   r%d, [r%d, %d]!\n", rd, rb, s12);
			} else
				printf("strb   r%d, [r%d, %d]\n", rd, rb, s12);
			break;

		case 1:
			*(uint16_t*)(data + offset) = naja->regs[rd];
			if (A) {
				naja->regs[rb] += s12 << 1;
				printf("strw   r%d, [r%d, %d]!\n", rd, rb, s12 << 1);
			} else
				printf("strw   r%d, [r%d, %d]\n", rd, rb, s12 << 1);
			break;

		case 2:
			*(uint32_t*)(data + offset) = naja->regs[rd];
			if (A) {
				naja->regs[rb] += s12 << 2;
				printf("strl   r%d, [r%d, %d]!\n", rd, rb, s12 << 2);
			} else
				printf("strl   r%d, [r%d, %d],  %d, %p\n", rd, rb, s12 << 2, *(uint32_t*)(data + offset), data + offset);
			break;

		case 3:
			*(uint64_t*)(data + offset) = naja->regs[rd];
			if (A) {
				naja->regs[rb] += s12 << 3;
				printf("str    r%d, [r%d, %d]!, rd: %#lx, rb: %ld, %p\n", rd, rb, s12 << 3, naja->regs[rd], naja->regs[rb], data + offset);
			} else
				printf("str    r%d, [r%d, %d]\n", rd, rb, s12 << 3);
			break;

		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_str_sib(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rb  =  inst        & 0x1f;
	int ri  = (inst >>  5) & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int ext = (inst >> 17) & 0x3;
	int u7  = (inst >> 10) & 0x7f;

	uint64_t addr   = naja->regs[rb];
	uint64_t offset = 0;
	uint8_t* data;

	if (addr >= (int64_t)vm->data->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->data->data + vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->rodata->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->rodata->data + vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= (int64_t)vm->text->data) {
		data  = (uint8_t*)addr;

		if (addr >= (int64_t)vm->text->data + vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr  >= 0x800000) {
		data   = vm->data->data;
		offset = addr - vm->data->addr;

		if (offset >= vm->data->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x600000) {
		data   = vm->rodata->data;
		offset = addr - vm->rodata->addr;

		if (offset >= vm->rodata->len) {
			scf_loge("\n");
			return -1;
		}

	} else if (addr >= 0x400000) {
		data   = vm->text->data;
		offset = addr - vm->text->addr;

		if (offset >= vm->text->len) {
			scf_loge("\n");
			return -1;
		}

	} else {
		data   = naja->stack;
		offset = addr;
	}

	offset += naja->regs[ri] << u7;

	if (data   == naja->stack) {
		offset = -offset;

		assert(offset >= 0);

		if (naja->size < offset) {
			scf_loge("\n");
			return -1;
		}

		offset -= 1 << ext;
	}

	switch (ext) {
		case 0:
			printf("strb  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			*(uint8_t*)(data + offset) = naja->regs[rd];
			break;

		case 1:
			printf("strw  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			*(uint16_t*)(data + offset) = naja->regs[rd];
			break;

		case 2:
			printf("strl  r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			*(uint32_t*)(data + offset) = naja->regs[rd];
			break;

		case 3:
			printf("str   r%d, [r%d, r%d, %d]\n", rd, rb, ri, u7);

			*(uint64_t*)(data + offset) = naja->regs[rd];
			break;

		default:
			scf_loge("\n");
			return -1;
			break;
	};

	naja->ip += 4;
	return 0;
}

static int __naja_and(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	scf_logw("\n");
	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		naja->regs[rd]  = naja->regs[rs0] & uimm15;
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		uint64_t rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			naja->regs[rd]  = naja->regs[rs0] & (naja->regs[rs1] << uimm8);
		else if (1 == sh)
			naja->regs[rd]  = naja->regs[rs0] & (naja->regs[rs1] >> uimm8);
		else
			naja->regs[rd]  = naja->regs[rs0] & (((int64_t)naja->regs[rs1]) >> uimm8);
	}

	naja->ip += 4;
	return 0;
}

static int __naja_teq(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	scf_logw("\n");
	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		naja->flags = naja->regs[rs0] & uimm15;
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		uint64_t rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			naja->flags = naja->regs[rs0] & (naja->regs[rs1] << uimm8);
		else if (1 == sh)
			naja->flags = naja->regs[rs0] & (naja->regs[rs1] >> uimm8);
		else
			naja->flags = naja->regs[rs0] & (((int64_t)naja->regs[rs1]) >> uimm8);
	}

	naja->flags = !naja->flags;

	naja->ip += 4;
	return 0;
}

static int __naja_or(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rs0 =  inst        & 0x1f;
	int rd  = (inst >> 21) & 0x1f;
	int I   = (inst >> 20) & 0x1;

	scf_logw("\n");
	if (I) {
		uint64_t uimm15 = (inst >> 5) & 0x7fff;

		naja->regs[rd]  = naja->regs[rs0] | uimm15;
	} else {
		uint64_t sh     = (inst >> 18) & 0x3;
		uint64_t uimm8  = (inst >> 10) & 0xff;
		uint64_t rs1    = (inst >>  5) & 0x1f;

		if (0 == sh)
			naja->regs[rd]  = naja->regs[rs0] | (naja->regs[rs1] << uimm8);
		else if (1 == sh)
			naja->regs[rd]  = naja->regs[rs0] | (naja->regs[rs1] >> uimm8);
		else
			naja->regs[rd]  = naja->regs[rs0] | (((int64_t)naja->regs[rs1]) >> uimm8);
	}

	naja->ip += 4;
	return 0;
}

static int __naja_jmp_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int simm26 = inst & 0x3ffffff;

	if (simm26  & 0x2000000)
		simm26 |= 0xfc000000;

	naja->ip += simm26 << 2;
	printf("jmp    %#lx\n", naja->ip);
	return 0;
}

static int __naja_call_disp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int simm26 = inst & 0x3ffffff;

	if (simm26  & 0x2000000)
		simm26 |= 0xfc000000;

	naja->regs[NAJA_REG_LR] = naja->ip + 4;

	naja->ip += simm26 << 2;

	printf("call   %#lx\n", naja->ip);
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

		if (0 == (cc & naja->flags))
			naja->ip  += s21;
		else
			naja->ip  += 4;

		if (0 == cc)
			printf("jz     %#lx, flags: %#lx\n", naja->ip, naja->flags);

		else if (1 == cc)
			printf("jnz    %#lx, flags: %#lx\n", naja->ip, naja->flags);

		else if (2 == cc)
			printf("jge    %#lx, flags: %#lx\n", naja->ip, naja->flags);

		else if (3 == cc)
			printf("jgt    %#lx, flags: %#lx\n", naja->ip, naja->flags);

		else if (4 == cc)
			printf("jle    %#lx, flags: %#lx\n", naja->ip, naja->flags);

		else if (5 == cc)
			printf("jlt    %#lx, flags: %#lx\n", naja->ip, naja->flags);
		else {
			scf_loge("\n");
			return -EINVAL;
		}

	} else {
		int rd = (inst >> 21) & 0x1f;

		if (naja_vm_dynamic_link == (void*)naja->regs[rd]) {

			printf("\033[36mjmp    r%d, %#lx@plt\033[0m\n", rd, naja->regs[rd]);

			int ret = naja_vm_dynamic_link(vm);
			if (ret < 0) {
				scf_loge("\n");
				return ret;
			}

			naja->ip = naja->regs[NAJA_REG_LR];

		} else if (naja->regs[rd] < vm->text->addr
				|| naja->regs[rd] > vm->text->addr + vm->text->len) {

			printf("\033[36mjmp    r%d, %#lx@plt\033[0m\n", rd, naja->regs[rd]);

			naja_dyn_func_pt pt = (naja_dyn_func_pt) naja->regs[rd];

			naja->regs[0] = pt(naja->regs[0],
					naja->regs[1],
					naja->regs[2],
					naja->regs[3],
					naja->regs[4],
					naja->regs[5],
					naja->regs[6],
					naja->regs[7]);

			naja->ip = naja->regs[NAJA_REG_LR];
		} else {
			naja->ip = naja->regs[rd];

			printf("jmp    r%d, %#lx\n", rd, naja->regs[rd]);
		}
	}

	return 0;
}


static int __naja_call_reg(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd = (inst >> 21) & 0x1f;

	naja->regs[NAJA_REG_LR]   = naja->ip + 4;

	if (naja_vm_dynamic_link == (void*)naja->regs[rd]) {

		printf("\033[36mcall  r%d, %#lx@plt\033[0m\n", rd, naja->regs[rd]);

		int ret = naja_vm_dynamic_link(vm);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}

		naja->ip = naja->regs[NAJA_REG_LR];

	} else if (naja->regs[rd] < vm->text->addr
			|| naja->regs[rd] > vm->text->addr + vm->text->len) {

		printf("\033[36mcall  r%d, %#lx@plt\033[0m\n", rd, naja->regs[rd]);

		naja_dyn_func_pt pt = (naja_dyn_func_pt) naja->regs[rd];

		naja->regs[0] = pt(naja->regs[0],
				           naja->regs[1],
				           naja->regs[2],
				           naja->regs[3],
				           naja->regs[4],
				           naja->regs[5],
				           naja->regs[6],
				           naja->regs[7]);

		naja->ip = naja->regs[NAJA_REG_LR];
	} else {
		printf("call  r%d, %#lx\n", rd, naja->regs[rd]);
		naja->ip = naja->regs[rd];
	}

	return 0;
}

static int __naja_adrp(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd  = (inst >> 21) & 0x1f;
	int s21 =  inst & 0x1fffff;

	if (s21  & 0x100000)
		s21 |= ~0x1fffff;

	naja->regs[rd] = (naja->ip + ((int64_t)s21 << 15)) & ~0x7fffULL;

	if (naja->regs[rd] >= 0x800000)
		naja->regs[rd]  = naja->regs[rd] - vm->data->addr + (uint64_t)vm->data->data;

	else if (naja->regs[rd] >= 0x600000)
		naja->regs[rd]  = naja->regs[rd] - vm->rodata->addr + (uint64_t)vm->rodata->data;

	else if (naja->regs[rd] >= 0x400000)
		naja->regs[rd]  = naja->regs[rd] - vm->text->addr + (uint64_t)vm->text->data;

	printf("adrp   r%d, [rip, %d],  %#lx\n", rd, s21, naja->regs[rd]);

	naja->ip += 4;
	return 0;
}

static int __naja_ret(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	naja->ip   =  naja->regs[NAJA_REG_LR];
	int64_t sp = -naja->regs[NAJA_REG_SP];

	assert (sp >= 0);

	if (naja->size > sp + STACK_INC) {

		void* p = realloc(naja->stack, sp + STACK_INC);
		if (!p) {
			scf_loge("\n");
			return -ENOMEM;
		}

		naja->stack = p;
		naja->size  = sp + STACK_INC;
	}

	printf("ret,   %#lx, sp: %ld, stack->size: %ld\n", naja->ip, sp, naja->size);
	return 0;
}

static int __naja_setcc(scf_vm_t* vm, uint32_t inst)
{
	scf_vm_naja_t* naja = vm->priv;

	int rd = (inst >> 21) & 0x1f;
	int cc = (inst >> 17) & 0xf;

	naja->regs[rd] = 0 == (cc & naja->flags);

	if (SCF_VM_Z == cc)
		printf("setz   r%d\n", rd);

	else if (SCF_VM_NZ == cc)
		printf("setnz  r%d\n", rd);

	else if (SCF_VM_GE == cc)
		printf("setge  r%d\n", rd);

	else if (SCF_VM_GT == cc)
		printf("setgt  r%d\n", rd);

	else if (SCF_VM_LT == cc)
		printf("setlt  r%d\n", rd);

	else if (SCF_VM_LE == cc)
		printf("setle  r%d\n", rd);
	else {
		scf_loge("inst: %#x\n", inst);
		return -EINVAL;
	}

	naja->ip  += 4;
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
			naja->regs[rd]  = inst & 0xffff;

			if (X && (inst & 0x8000)) {
				naja->regs[rd] |= ~0xffffULL;
				printf("movsb  r%d, %d\n", rd, inst & 0xffff);
			} else {
				printf("mov    r%d, %d\n", rd, inst & 0xffff);
			}

		} else if (1 == opt) {
			naja->regs[rd] |= (inst & 0xffffULL) << 16;

			printf("mov    r%d, %d << 16\n", rd, inst & 0xffff);

		} else if (2 == opt) {
			naja->regs[rd] |= (inst & 0xffffULL) << 32;

			printf("mov    r%d, %d << 32\n", rd, inst & 0xffff);

		} else if (3 == opt) {
			naja->regs[rd] |= (inst & 0xffffULL) << 48;

			printf("mov    r%d, %d << 48\n", rd, inst & 0xffff);

		} else if (7 == opt) {
			naja->regs[rd] = ~(inst & 0xffffULL);

			printf("mvn    r%d, %d\n", rd, inst & 0xffff);
		}

	} else {
		int rs  =  inst & 0x1f;
		int rs1 = (inst >> 5) & 0x1f;
		int u11 = (inst >> 5) & 0x7ff;

		if (0 == opt) {
			if (X) {
				printf("mov    r%d, r%d LSL r%d\n", rd, rs, rs1);

				naja->regs[rd] = naja->regs[rs] << naja->regs[rs1];
			} else {
				naja->regs[rd] = naja->regs[rs] << u11;

				if (0 == u11)
					printf("mov    r%d, r%d\n", rd, rs);
				else
					printf("mov    r%d, r%d LSL %d\n", rd, rs, u11);
			}

		} else if (1 == opt) {
			if (X) {
				printf("mov    r%d, r%d LSR r%d\n", rd, rs, rs1);

				naja->regs[rd] = naja->regs[rs] >> naja->regs[rs1];
			} else {
				naja->regs[rd] = naja->regs[rs] >> u11;

				if (0 == u11)
					printf("mov    r%d, r%d\n", rd, rs);
				else
					printf("mov    r%d, r%d LSR %d\n", rd, rs, u11);
			}
		} else if (2 == opt) {
			if (X) {
				printf("mov    r%d, r%d ASR r%d\n", rd, rs, rs1);

				naja->regs[rd] = (int64_t)naja->regs[rs] >> naja->regs[rs1];
			} else {
				naja->regs[rd] = (int64_t)naja->regs[rs] >> u11;

				if (0 == u11)
					printf("mov    r%d, r%d\n", rd, rs);
				else
					printf("mov    r%d, r%d ASR %d\n", rd, rs, u11);
			}
		} else if (3 == opt) {
			printf("NOT    r%d, r%d\n", rd, rs);

			naja->regs[rd] = ~naja->regs[rs];
		} else if (4 == opt) {
			naja->regs[rd] = -naja->regs[rs];

			printf("NEG    r%d, r%d\n", rd, rs);

		} else if (5 == opt) {
			if (X) {
				printf("movsb  r%d, r%d\n", rd, rs);

				naja->regs[rd] = (int8_t)naja->regs[rs];
			} else {
				naja->regs[rd] = (uint8_t)naja->regs[rs];

				printf("movzb  r%d, r%d\n", rd, rs);
			}
		} else if (6 == opt) {
			if (X) {
				printf("movsw  r%d, r%d\n", rd, rs);

				naja->regs[rd] = (int16_t)naja->regs[rs];
			} else {
				naja->regs[rd] = (uint16_t)naja->regs[rs];

				printf("movzw  r%d, r%d\n", rd, rs);
			}
		} else if (7 == opt) {
			if (X) {
				printf("movsl  r%d, r%d\n", rd, rs);

				naja->regs[rd] = (int32_t)naja->regs[rs];
			} else {
				naja->regs[rd] = (uint32_t)naja->regs[rs];

				printf("movzl  r%d, r%d\n", rd, rs);
			}
		}
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

	NULL,            //16
	NULL,            //17
	NULL,            //18
	NULL,            //19
	NULL,            //20
	NULL,            //21
	NULL,            //22
	NULL,            //23
	__naja_call_disp,//24
	NULL,            //25
	__naja_call_reg, //26
	NULL,            //27
	NULL,            //28
	NULL,            //29
	NULL,            //30
	NULL,            //31

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

static void __naja_vm_exit()
{
}

static int __naja_vm_run(scf_vm_t* vm, const char* path, const char* sys)
{
	scf_vm_naja_t* naja = vm->priv;
	Elf64_Ehdr     eh;
	Elf64_Shdr     sh;

	fseek(vm->elf->fp, 0, SEEK_SET);

	int ret  = fread(&eh, sizeof(Elf64_Ehdr), 1, vm->elf->fp);
	if (ret != 1)
		return -1;

	if (vm->jmprel) {
		fseek(vm->elf->fp, eh.e_shoff, SEEK_SET);

		int i;
		for (i = 0; i < eh.e_shnum; i++) {

			ret = fread(&sh, sizeof(Elf64_Shdr), 1, vm->elf->fp);
			if (ret != 1)
				return -1;

			if (vm->jmprel_addr == sh.sh_addr) {
				vm->jmprel_size  = sh.sh_size;
				break;
			}
		}

		if (i == eh.e_shnum) {
			scf_loge("\n");
			return -1;
		}
	}

	naja->stack  = calloc(STACK_INC, sizeof(uint64_t));
	if (!naja->stack)
		return -ENOMEM;

	naja->size   = STACK_INC;
	naja->_start = eh.e_entry;
	naja->ip     = eh.e_entry;

	naja->regs[NAJA_REG_LR] = (uint64_t)__naja_vm_exit;

	int n = 0;
	while ((uint64_t)__naja_vm_exit != naja->ip) {

		int64_t offset = naja->ip - vm->text->addr;

		if (offset >= vm->text->len) {
			scf_loge("naja->ip: %#lx, %p\n", naja->ip, __naja_vm_exit);
			return -1;
		}

		uint32_t inst = *(uint32_t*)(vm->text->data + offset);

		naja_opcode_pt pt = naja_opcodes[(inst >> 26) & 0x3f];

		if (!pt) {
			scf_loge("inst: %d, %#x\n", (inst >> 26) & 0x3f, inst);
			return -EINVAL;
		}

		printf("%4d, %#lx: ", n++, naja->ip);
		ret = pt(vm, inst);
		if (ret < 0) {
			scf_loge("\n");
			return ret;
		}

		usleep(50 * 1000);
	}

	scf_logw("r0: %ld\n", naja->regs[0]);
	return naja->regs[0];
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

scf_vm_ops_t  vm_ops_naja =
{
	.name  = "naja",
	.open  = naja_vm_open,
	.close = naja_vm_close,
	.run   = naja_vm_run,
};

