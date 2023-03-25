#include"scf_elf_naja.h"
#include"scf_elf_link.h"

static uint32_t naja_plt_lazy[8] = {
	0xa9bf7bf0,  // stp  x16, x30, [sp, #-16]!
	0x90000010,  // adrp x16, 0
	0xf9400211,  // ldr  x17, [x16, #0]
	0x91000210,  // add  x16,  x16, #0

	0xd61f0220,  // br   x17
	0xd503201f,  // nop
	0xd503201f,  // nop
	0xd503201f,  // nop
};

static uint32_t naja_plt[4] = {
	0x90000010,  // adrp x16, 0
	0xf9400211,  // ldr  x17, [x16, #0]
	0x91000210,  // add  x16,  x16, #0
	0xd61f0220,  // br   x17
};


static uint32_t _naja_elf_hash(const uint8_t* p)
{
	uint32_t k = 0;
	uint32_t u = 0;

	while (*p) {
		k = (k << 4) + *p++;
		u = k & 0xf0000000;

		if (u)
			k ^= u >> 24;
		k &= ~u;
	}
	return k;
}

static int _naja_elf_add_interp(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".interp");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	char*  interp = "/lib/ld-linux-aarch64.so.1";
	size_t len    = strlen(interp);
	size_t align  = (len + 1 + 7) & ~0x7;

	s->data = calloc(1, align);
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	memcpy(s->data, interp, len);
	s->data_len  = align;

	s->index     = 1;

	s->sh.sh_type   = SHT_PROGBITS;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_addralign = 1;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_gnu_version(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".gnu.version");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = calloc(naja->dynsyms->size, sizeof(Elf64_Versym));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = naja->dynsyms->size * sizeof(Elf64_Versym);

	s->index     = 1;

	s->sh.sh_type   = SHT_GNU_versym;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_addralign = 8;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_gnu_version_r(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".gnu.version_r");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = calloc(1, sizeof(Elf64_Verneed) + sizeof(Elf64_Vernaux));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = sizeof(Elf64_Verneed) + sizeof(Elf64_Vernaux);

	s->index     = 1;

	s->sh.sh_type   = SHT_GNU_verneed;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_addralign = 8;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_dynsym(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".dynsym");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = calloc(naja->dynsyms->size + 1, sizeof(Elf64_Sym));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = (naja->dynsyms->size + 1) * sizeof(Elf64_Sym);

	s->index     = 1;

	s->sh.sh_type   = SHT_DYNSYM;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_info   = 1;
	s->sh.sh_addralign = 8;
	s->sh.sh_entsize   = sizeof(Elf64_Sym);

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_dynstr(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".dynstr");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->index     = 1;

	s->sh.sh_type   = SHT_STRTAB;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_addralign = 1;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_dynamic(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".dynamic");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	int nb_tags = naja->dyn_needs->size + 11 + 1;

	s->data = calloc(nb_tags, sizeof(Elf64_Dyn));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = nb_tags * sizeof(Elf64_Dyn);

	s->index     = 1;

	s->sh.sh_type   = SHT_PROGBITS;
	s->sh.sh_flags  = SHF_ALLOC | SHF_WRITE;
	s->sh.sh_addralign = 8;
	s->sh.sh_entsize   = sizeof(Elf64_Dyn);

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_got_plt(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".got.plt");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = calloc(naja->dynsyms->size + 3, sizeof(void*));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = (naja->dynsyms->size + 3) * sizeof(void*);

	s->index     = 1;

	s->sh.sh_type   = SHT_PROGBITS;
	s->sh.sh_flags  = SHF_ALLOC | SHF_WRITE;
	s->sh.sh_addralign = 8;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_rela_plt(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".rela.plt");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = calloc(naja->dynsyms->size, sizeof(Elf64_Rela));
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	s->data_len  = naja->dynsyms->size * sizeof(Elf64_Rela);

	s->index     = 1;

	s->sh.sh_type   = SHT_RELA;
	s->sh.sh_flags  = SHF_ALLOC | SHF_INFO_LINK;
	s->sh.sh_addralign = 8;
	s->sh.sh_entsize   = sizeof(Elf64_Rela);

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _naja_elf_add_plt(elf_native_t* naja, elf_section_t** ps)
{
	elf_section_t* s;

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(".plt");
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->data = malloc(sizeof(naja_plt_lazy) + sizeof(naja_plt) * naja->dynsyms->size);
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}

	memcpy(s->data, naja_plt_lazy, sizeof(naja_plt_lazy));
	s->data_len = sizeof(naja_plt_lazy);

	int i;
	for (i = 0; i < naja->dynsyms->size; i++) {

		memcpy(s->data + s->data_len, naja_plt, sizeof(naja_plt));

		s->data_len += sizeof(naja_plt);
	}

	s->index     = 1;

	s->sh.sh_type   = SHT_PROGBITS;
	s->sh.sh_flags  = SHF_ALLOC;
	s->sh.sh_addralign = 1;

	int ret = scf_vector_add(naja->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s->data);
		free(s);
		return -ENOMEM;
	}

	*ps = s;
	return 0;
}

static int _section_cmp(const void* v0, const void* v1)
{
	const elf_section_t* s0 = *(const elf_section_t**)v0;
	const elf_section_t* s1 = *(const elf_section_t**)v1;

	if (s0->index < s1->index)
		return -1;
	else if (s0->index > s1->index)
		return 1;
	return 0;
}

int __naja_elf_add_dyn (elf_native_t* naja)
{
	elf_section_t* s;
	elf_sym_t*     sym;
	Elf64_Rela*            rela;

	int i;
	for (i  = naja->symbols->size - 1; i >= 0; i--) {
		sym = naja->symbols->data[i];

		uint16_t shndx = sym->sym.st_shndx;

		if (STT_SECTION == ELF64_ST_TYPE(sym->sym.st_info)) {
			if (shndx > 0) {
				assert(shndx - 1 < naja->sections->size);
				sym->section = naja->sections->data[shndx - 1];
			}
		} else if (0 != shndx) {
			if (shndx - 1 < naja->sections->size)
				sym->section = naja->sections->data[shndx - 1];
		}
	}

	char* sh_names[] = {
		".interp",
		".dynsym",
		".dynstr",
//		".gnu.version_r",
		".rela.plt",
		".plt",

		".text",
		".rodata",

		".dynamic",
		".got.plt",
		".data",
	};

	for (i = 0; i < naja->sections->size; i++) {
		s  =        naja->sections->data[i];

		s->index = naja->sections->size + 1 + sizeof(sh_names) / sizeof(sh_names[0]);

		scf_logw("s: %s, link: %d, info: %d\n", s->name->data, s->sh.sh_link, s->sh.sh_info);

		if (s->sh.sh_link > 0) {
			assert(s->sh.sh_link - 1 < naja->sections->size);

			s->link = naja->sections->data[s->sh.sh_link - 1];
		}

		if (s->sh.sh_info > 0) {
			assert(s->sh.sh_info - 1 < naja->sections->size);

			s->info = naja->sections->data[s->sh.sh_info - 1];
		}
	}

	_naja_elf_add_interp(naja, &naja->interp);
	_naja_elf_add_dynsym(naja, &naja->dynsym);
	_naja_elf_add_dynstr(naja, &naja->dynstr);

//	_naja_elf_add_gnu_version_r(naja, &naja->gnu_version_r);

	_naja_elf_add_rela_plt(naja, &naja->rela_plt);
	_naja_elf_add_plt(naja, &naja->plt);

	_naja_elf_add_dynamic(naja, &naja->dynamic);
	_naja_elf_add_got_plt(naja, &naja->got_plt);

	scf_string_t* str = scf_string_alloc();

	char c = '\0';
	scf_string_cat_cstr_len(str, &c, 1);

	Elf64_Sym*    syms    = (Elf64_Sym*   )naja->dynsym->data;
	Elf64_Sym     sym0    = {0};

	sym0.st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE);
	memcpy(&syms[0], &sym0, sizeof(Elf64_Sym));

	for (i = 0; i < naja->dynsyms->size; i++) {
		elf_sym_t* xsym = naja->dynsyms->data[i];

		memcpy(&syms[i + 1], &xsym->sym, sizeof(Elf64_Sym));

		syms[i + 1].st_name = str->len;

		scf_loge("i: %d, st_value: %#lx\n", i, syms[i + 1].st_value);

		scf_string_cat_cstr_len(str, xsym->name->data, xsym->name->len + 1);
	}

#if 0
	Elf64_Verneed* verneeds = (Elf64_Verneed*) naja->gnu_version_r->data;
	Elf64_Vernaux* vernauxs = (Elf64_Vernaux*)(naja->gnu_version_r->data +sizeof(Elf64_Verneed));

	verneeds[0].vn_version = VER_NEED_CURRENT;
	verneeds[0].vn_file    = str->len;
	verneeds[0].vn_cnt     = 1;
	verneeds[0].vn_aux     = sizeof(Elf64_Verneed);
	verneeds[0].vn_next    = 0;

	scf_string_cat_cstr_len(str, "libc.so.6", strlen("libc.so.6") + 1);

	vernauxs[0].vna_hash   = _naja_elf_hash("GLIBC_2.4");
	vernauxs[0].vna_flags  = 0;
	vernauxs[0].vna_other  = 2;
	vernauxs[0].vna_name   = str->len;
	vernauxs[0].vna_next   = 0;

	scf_string_cat_cstr_len(str, "GLIBC_2.4", strlen("GLIBC_2.4") + 1);
#endif

	Elf64_Dyn* dyns = (Elf64_Dyn*)naja->dynamic->data;

	size_t prefix   = strlen("../lib/naja");

	for (i = 0; i < naja->dyn_needs->size; i++) {
		scf_string_t* needed = naja->dyn_needs->data[i];

		dyns[i].d_tag = DT_NEEDED;
		dyns[i].d_un.d_val = str->len;

		scf_logw("i: %d, %s, %s\n", i, needed->data, needed->data + prefix);

		scf_string_cat_cstr_len(str, needed->data + prefix, needed->len - prefix + 1);
	}

	dyns[i].d_tag     = DT_STRTAB;
	dyns[i + 1].d_tag = DT_SYMTAB;
	dyns[i + 2].d_tag = DT_STRSZ;
	dyns[i + 3].d_tag = DT_SYMENT;
	dyns[i + 4].d_tag = DT_PLTGOT;
	dyns[i + 5].d_tag = DT_PLTRELSZ;
	dyns[i + 6].d_tag = DT_PLTREL;
	dyns[i + 7].d_tag = DT_JMPREL;
//	dyns[i + 8].d_tag = DT_VERNEED;
//	dyns[i + 9].d_tag = DT_VERNEEDNUM;
//	dyns[i +10].d_tag = DT_VERSYM;
	dyns[i +8].d_tag = DT_NULL;

	dyns[i].d_un.d_ptr     = (uintptr_t)naja->dynstr;
	dyns[i + 1].d_un.d_ptr = (uintptr_t)naja->dynsym;
	dyns[i + 2].d_un.d_val = str->len;
	dyns[i + 3].d_un.d_val = sizeof(Elf64_Sym);
	dyns[i + 4].d_un.d_ptr = (uintptr_t)naja->got_plt;
	dyns[i + 5].d_un.d_ptr = sizeof(Elf64_Rela);
	dyns[i + 6].d_un.d_ptr = DT_RELA;
	dyns[i + 7].d_un.d_ptr = (uintptr_t)naja->rela_plt;
//	dyns[i + 8].d_un.d_ptr = (uintptr_t)naja->gnu_version_r;
//	dyns[i + 9].d_un.d_ptr = 1;
//	dyns[i +10].d_un.d_ptr = (uintptr_t)naja->gnu_version;
	dyns[i +8].d_un.d_ptr = 0;

	int fill = 8 - (str->len & 0x7);
	if (fill > 0)
		scf_string_fill_zero(str, fill);

	naja->dynstr->data     = str->data;
	naja->dynstr->data_len = str->len;

	str->data = NULL;
	str->len  = 0;
	str->capacity = 0;
	scf_string_free(str);
	str = NULL;

	naja->rela_plt->link = naja->dynsym;
	naja->rela_plt->info = naja->got_plt;
	naja->dynsym  ->link = naja->dynstr;
#if 0
	naja->gnu_version_r->link = naja->dynstr;
	naja->gnu_version_r->info = naja->interp;
#endif

	for (i = 0; i < naja->sections->size; i++) {
		s  =        naja->sections->data[i];

		int j;
		for (j = 0; j < sizeof(sh_names) / sizeof(sh_names[0]); j++) {
			if (!strcmp(s->name->data, sh_names[j]))
				break;
		}

		if (j < sizeof(sh_names) / sizeof(sh_names[0]))
			s->index = j + 1;

		scf_logd("i: %d, s: %s, index: %d\n", i, s->name->data, s->index);
	}

	qsort(naja->sections->data, naja->sections->size, sizeof(void*), _section_cmp);

	int j = sizeof(sh_names) / sizeof(sh_names[0]);

	for (i = j; i < naja->sections->size; i++) {
		s  =        naja->sections->data[i];

		s->index = i + 1;
	}

	for (i = 0; i < naja->sections->size; i++) {
		s  =        naja->sections->data[i];

		scf_loge("i: %d, s: %s, index: %d\n", i, s->name->data, s->index);

		if (s->link) {
			scf_logd("link: %s, index: %d\n", s->link->name->data, s->link->index);
			s->sh.sh_link = s->link->index;
		}

		if (s->info) {
			scf_logd("info: %s, index: %d\n", s->info->name->data, s->info->index);
			s->sh.sh_info = s->info->index;
		}
	}

#if 1
	for (i  = 0; i < naja->symbols->size; i++) {
		sym =        naja->symbols->data[i];

		if (sym->section) {
			scf_logw("sym: %s, index: %d->%d\n", sym->name->data, sym->sym.st_shndx, sym->section->index);
			sym->sym.st_shndx = sym->section->index;
		}
	}
#endif
	return 0;
}

int __naja_elf_post_dyn(elf_native_t* naja, uint64_t rx_base, uint64_t rw_base, elf_section_t* cs)
{
	uint64_t cs_base   = rx_base + cs->offset;

//	naja->gnu_version_r->sh.sh_addr = rx_base + naja->gnu_version_r->offset;

	naja->rela_plt->sh.sh_addr = rx_base + naja->rela_plt->offset;
	naja->dynamic->sh.sh_addr  = rw_base + naja->dynamic->offset;
	naja->got_plt->sh.sh_addr  = rw_base + naja->got_plt->offset;
	naja->interp->sh.sh_addr   = rx_base + naja->interp->offset;
	naja->plt->sh.sh_addr      = rx_base + naja->plt->offset;

	scf_loge("rw_base: %#lx, offset: %#lx\n", rw_base, naja->got_plt->offset);
	scf_loge("got_addr: %#lx\n", naja->got_plt->sh.sh_addr);

	Elf64_Rela* rela_plt = (Elf64_Rela*)naja->rela_plt->data;
	Elf64_Sym*  dynsym   = (Elf64_Sym* )naja->dynsym->data;
	uint64_t*   got_plt  = (uint64_t*  )naja->got_plt->data;
	uint32_t*   plt      = (uint32_t*  )naja->plt->data;

	uint64_t   got_addr = naja->got_plt->sh.sh_addr;
	uint64_t   plt_addr = naja->plt->sh.sh_addr;
	int32_t    offset   = got_addr - plt_addr;

	got_plt[0] = naja->dynamic->sh.sh_addr;
	got_addr  += 8;

	got_plt[1] = 0;
	got_plt[2] = 0;
	got_plt   += 3;
	got_addr  += 8;

	scf_loge("got_addr: %#lx, plt_addr: %#lx, offset: %d, %#x\n", got_addr, plt_addr, offset, offset);

	plt[1] |= (((offset >> 12) & 0x3) << 29) | (((offset >> 14) & 0x7ffff) << 5);
	plt[2] |=  ((got_addr & 0xfff) >> 3) << 10;
	plt[3] |=   (got_addr & 0xfff) << 10;

	got_addr += 8;
	plt_addr += sizeof(naja_plt_lazy);
	plt      += sizeof(naja_plt_lazy) / sizeof(naja_plt_lazy[0]);

	int i;
	for (i = 0; i < naja->dynsyms->size; i++) {
		rela_plt[i].r_offset   = got_addr;
		rela_plt[i].r_addend   = 0;
		rela_plt[i].r_info     = ELF64_R_INFO(i + 1, R_AARCH64_JUMP_SLOT);

		*got_plt = naja->plt->sh.sh_addr;

		offset = got_addr - plt_addr;

		scf_loge("i: %d, got_addr: %#lx, plt_addr: %#lx, offset: %d, %#x\n", i, got_addr, plt_addr, offset, offset);

		plt[0] |= (((offset >> 12) & 0x3) << 29) | (((offset >> 14) & 0x7ffff) << 5);
		plt[1] |=  ((got_addr & 0xfff) >> 3) << 10;
		plt[2] |=   (got_addr & 0xfff) << 10;

		plt += sizeof(naja_plt) / sizeof(naja_plt[0]);
		plt_addr += sizeof(naja_plt);
		got_addr += 8;
		got_plt++;
	}

	for (i = 0; i < naja->dyn_relas->size; i++) {
		Elf64_Rela* r = naja->dyn_relas->data[i];

		int sym_idx = ELF64_R_SYM(r->r_info);
		assert(sym_idx > 0);

		assert(ELF64_R_TYPE(r->r_info) == R_AARCH64_CALL26);

		uint64_t plt_addr = naja->plt->sh.sh_addr + sizeof(naja_plt_lazy) + (sym_idx - 1) * sizeof(naja_plt);

		int32_t offset = plt_addr - (cs_base + r->r_offset) + r->r_addend;

		assert(0 == (offset & 0x3));

		offset >>= 2;

		if (offset > 0x1ffffff || offset < -0x1ffffff) {
			scf_loge("\n");
			return -EINVAL;
		}

		offset &= 0x3ffffff;
		offset |= (0x25 << 26);

		*(uint32_t*)(cs->data + r->r_offset) = offset;
	}

	Elf64_Dyn* dtags = (Elf64_Dyn*)naja->dynamic->data;

	for (i  = naja->dyn_needs->size; i < naja->dynamic->data_len / sizeof(Elf64_Dyn); i++) {

		elf_section_t* s = (elf_section_t*)dtags[i].d_un.d_ptr;

		switch (dtags[i].d_tag) {

			case DT_SYMTAB:
			case DT_STRTAB:
			case DT_JMPREL:
			case DT_VERNEED:
			case DT_VERSYM:
				dtags[i].d_un.d_ptr = s->offset + rx_base;
				s->sh.sh_addr       = s->offset + rx_base;
				break;

			case DT_PLTGOT:
				dtags[i].d_un.d_ptr = s->offset + rw_base;
				s->sh.sh_addr       = s->offset + rw_base;
				break;
			default:
				break;
		};
	}

	return 0;
}

int __naja_elf_write_phdr(scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint32_t nb_phdrs)
{
	// write program header

	Elf64_Phdr ph_phdr = {0};

	ph_phdr.p_type     = PT_PHDR;
	ph_phdr.p_flags    = PF_R;
	ph_phdr.p_offset   = offset;
	ph_phdr.p_vaddr    = rx_base + offset;
	ph_phdr.p_paddr    = ph_phdr.p_vaddr;
	ph_phdr.p_filesz   = sizeof(Elf64_Phdr) * nb_phdrs;
	ph_phdr.p_memsz    = ph_phdr.p_filesz;
	ph_phdr.p_align    = 0x8;

	fwrite(&ph_phdr,  sizeof(ph_phdr),  1, elf->fp);
	return 0;
}

int __naja_elf_write_interp(scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint64_t len)
{
	Elf64_Phdr ph_interp  = {0};

	ph_interp.p_type   = PT_INTERP;
	ph_interp.p_flags  = PF_R;
	ph_interp.p_offset = offset;
	ph_interp.p_vaddr  = rx_base + offset;
	ph_interp.p_paddr  = ph_interp.p_vaddr;
	ph_interp.p_filesz = len;
	ph_interp.p_memsz  = ph_interp.p_filesz;
	ph_interp.p_align  = 0x1;

	fwrite(&ph_interp, sizeof(ph_interp),  1, elf->fp);
	return 0;
}

int __naja_elf_write_text(scf_elf_context_t* elf, uint64_t rx_base, uint64_t offset, uint64_t len)
{
	Elf64_Phdr ph_text = {0};

	ph_text.p_type     = PT_LOAD;
	ph_text.p_flags    = PF_R | PF_X;
	ph_text.p_offset   = 0;
	ph_text.p_vaddr    = rx_base + offset;
	ph_text.p_paddr    = ph_text.p_vaddr;
	ph_text.p_filesz   = len;
	ph_text.p_memsz    = ph_text.p_filesz;
	ph_text.p_align    = 0x200000;

	fwrite(&ph_text,  sizeof(ph_text),  1, elf->fp);
	return 0;
}

int __naja_elf_write_rodata(scf_elf_context_t* elf, uint64_t r_base, uint64_t offset, uint64_t len)
{
	Elf64_Phdr ph_rodata  = {0};

	ph_rodata.p_type   = PT_LOAD;
	ph_rodata.p_flags  = PF_R;
	ph_rodata.p_offset = offset;
	ph_rodata.p_vaddr  = r_base + offset;
	ph_rodata.p_paddr  = ph_rodata.p_vaddr;
	ph_rodata.p_filesz = len;
	ph_rodata.p_memsz  = ph_rodata.p_filesz;
	ph_rodata.p_align  = 0x200000;

	fwrite(&ph_rodata,  sizeof(ph_rodata),  1, elf->fp);
	return 0;
}

int __naja_elf_write_data(scf_elf_context_t* elf, uint64_t rw_base, uint64_t offset, uint64_t len)
{
	Elf64_Phdr ph_data    = {0};

	ph_data.p_type     = PT_LOAD;
	ph_data.p_flags    = PF_R | PF_W;
	ph_data.p_offset   = offset;
	ph_data.p_vaddr    = rw_base + offset;
	ph_data.p_paddr    = ph_data.p_vaddr;
	ph_data.p_filesz   = len;
	ph_data.p_memsz    = ph_data.p_filesz;
	ph_data.p_align    = 0x200000;

	fwrite(&ph_data,  sizeof(ph_data),  1, elf->fp);
	return 0;
}

int __naja_elf_write_dynamic(scf_elf_context_t* elf, uint64_t rw_base, uint64_t offset, uint64_t len)
{
	Elf64_Phdr ph_dynamic = {0};

	ph_dynamic.p_type     = PT_DYNAMIC;
	ph_dynamic.p_flags    = PF_R | PF_W;
	ph_dynamic.p_offset   = offset;
	ph_dynamic.p_vaddr    = rw_base + offset;
	ph_dynamic.p_paddr    = ph_dynamic.p_vaddr;
	ph_dynamic.p_filesz   = len;
	ph_dynamic.p_memsz    = ph_dynamic.p_filesz;
	ph_dynamic.p_align    = 0x8;

	fwrite(&ph_dynamic,  sizeof(Elf64_Phdr),  1, elf->fp);
	return 0;
}

