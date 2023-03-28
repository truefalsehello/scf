#include"scf_elf_native.h"
#include"scf_elf_link.h"

int elf_open(scf_elf_context_t* elf)
{
	if (!elf)
		return -EINVAL;

	elf_native_t* e = calloc(1, sizeof(elf_native_t));
	if (!e)
		return -ENOMEM;

	e->sh_null.sh_type          = SHT_NULL;

	e->sh_symtab.sh_type        = SHT_SYMTAB;
	e->sh_symtab.sh_flags       = 0;
	e->sh_symtab.sh_addralign   = 8;

	e->sh_strtab.sh_type        = SHT_STRTAB;
	e->sh_strtab.sh_flags       = 0;
	e->sh_strtab.sh_addralign   = 1;

	e->sh_shstrtab.sh_type      = SHT_STRTAB;
	e->sh_shstrtab.sh_flags     = 0;
	e->sh_shstrtab.sh_addralign = 1;

	e->sections = scf_vector_alloc();
	e->symbols  = scf_vector_alloc();

	elf->priv = e;
	return 0;
}

int elf_close(scf_elf_context_t* elf)
{
	elf_native_t* e = elf->priv;

	if (e) {
		free(e);
		e = NULL;
	}
	return 0;
}

int elf_add_sym(scf_elf_context_t* elf, const scf_elf_sym_t* sym, const char* sh_name)
{
	elf_sym_t*     xsym;
	elf_native_t*  e = elf->priv;
	scf_vector_t*  vec = NULL;

	if (!strcmp(sh_name, ".symtab"))
		vec = e->symbols;

	else if (!strcmp(sh_name, ".dynsym")) {

		if (!e->dynsyms) {
			e->dynsyms = scf_vector_alloc();
			if (!e->dynsyms)
				return -ENOMEM;
		}

		vec = e->dynsyms;
	} else
		return -EINVAL;

	xsym = calloc(1, sizeof(elf_sym_t));
	if (!xsym)
		return -ENOMEM;

	if (sym->name)
		xsym->name = scf_string_cstr(sym->name);
	else
		xsym->name = NULL;

	xsym->sym.st_size  = sym->st_size;
	xsym->sym.st_value = sym->st_value;
	xsym->sym.st_shndx = sym->st_shndx;
	xsym->sym.st_info  = sym->st_info;

	xsym->dyn_flag     = sym->dyn_flag;

	int ret = scf_vector_add(vec, xsym);
	if (ret < 0) {
		scf_string_free(xsym->name);
		free(xsym);
		return ret;
	}

	xsym->index = vec->size;
	return 0;
}

int elf_add_section(scf_elf_context_t* elf, const scf_elf_section_t* section)
{
	elf_native_t*  e = elf->priv;

	elf_section_t* s;
	elf_section_t* s2;
	int i;

	if (section->index > 0) {

		for (i = e->sections->size - 1; i >= 0; i--) {
			s  = e->sections->data[i];

			if (s->index == section->index) {
				scf_loge("s->index: %d\n", s->index);
				return -1;
			}
		}
	}

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(section->name);
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->sh.sh_type 		= section->sh_type;
	s->sh.sh_flags		= section->sh_flags;
	s->sh.sh_addralign	= section->sh_addralign;

	if (section->data && section->data_len > 0) {
		s->data = malloc(section->data_len);
		if (!s->data) {
			scf_string_free(s->name);
			free(s);
			return -ENOMEM;
		}

		memcpy(s->data, section->data, section->data_len);
		s->data_len = section->data_len;
	}

	if (scf_vector_add(e->sections, s) < 0) {
		if (s->data)
			free(s->data);
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}

	if (0 == section->index)
		s->index = e->sections->size;
	else {
		s->index = section->index;

		for (i = e->sections->size - 2; i >= 0; i--) {
			s2  = e->sections->data[i];

			if (s2->index < s->index)
				break;

			e->sections->data[i + 1] = s2;
		}

		e->sections->data[i + 1] = s;
	}

	return s->index;
}

int elf_add_rela_section(scf_elf_context_t* elf, const scf_elf_section_t* section, scf_vector_t* relas)
{
	if (relas->size <= 0) {
		scf_loge("\n");
		return -EINVAL;
	}

	elf_native_t*  e = elf->priv;

	elf_section_t* s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	s->name = scf_string_cstr(section->name);
	if (!s->name) {
		free(s);
		return -ENOMEM;
	}

	s->index            = e->sections->size + 1;
	s->sh.sh_type       = SHT_RELA;
	s->sh.sh_flags      = SHF_INFO_LINK;
	s->sh.sh_addralign  = section->sh_addralign;
	s->sh.sh_link       = section->sh_link;
	s->sh.sh_info       = section->sh_info;
	s->sh.sh_entsize    = sizeof(Elf64_Rela);

	s->data_len = sizeof(Elf64_Rela) * relas->size;

	s->data = malloc(s->data_len);
	if (!s->data) {
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}

	Elf64_Rela* pr = (Elf64_Rela*) s->data;

	int i;
	for (i = 0; i < relas->size; i++) {

		scf_elf_rela_t* r = relas->data[i];


		pr[i].r_offset = r->r_offset;
		pr[i].r_info   = r->r_info;
		pr[i].r_addend = r->r_addend;
	}

	if (scf_vector_add(e->sections, s) < 0) {
		free(s->data);
		scf_string_free(s->name);
		free(s);
		return -ENOMEM;
	}
	return s->index;
}

int elf_read_shstrtab(scf_elf_context_t* elf)
{
	elf_native_t* e = elf->priv;

	if (!elf->fp)
		return -EINVAL;

	int ret = fseek(elf->fp, elf->start, SEEK_SET);
	if (ret < 0)
		return ret;

	ret = fread(&e->eh, sizeof(Elf64_Ehdr), 1, elf->fp);
	if (ret != 1)
		return -1;

	if (ELFMAG0    != e->eh.e_ident[EI_MAG0]
		|| ELFMAG1 != e->eh.e_ident[EI_MAG1]
		|| ELFMAG2 != e->eh.e_ident[EI_MAG2]
		|| ELFMAG3 != e->eh.e_ident[EI_MAG3]) {

		scf_loge("not elf file\n");
		return -1;
	}

	long offset = e->eh.e_shoff + e->eh.e_shentsize * e->eh.e_shstrndx;
	fseek(elf->fp, elf->start + offset, SEEK_SET);

	ret = fread(&e->sh_shstrtab, sizeof(Elf64_Shdr), 1, elf->fp);
	if (ret != 1)
		return -1;

	if (!e->sh_shstrtab_data) {
		e->sh_shstrtab_data = scf_string_alloc();
		if (!e->sh_shstrtab_data)
			return -ENOMEM;
	}

	void* p = realloc(e->sh_shstrtab_data->data, e->sh_shstrtab.sh_size);
	if (!p)
		return -ENOMEM;
	e->sh_shstrtab_data->data     = p;
	e->sh_shstrtab_data->len      = e->sh_shstrtab.sh_size;
	e->sh_shstrtab_data->capacity = e->sh_shstrtab.sh_size;

	fseek(elf->fp, elf->start + e->sh_shstrtab.sh_offset, SEEK_SET);

	ret = fread(e->sh_shstrtab_data->data, e->sh_shstrtab.sh_size, 1, elf->fp);
	if (ret != 1)
		return -1;
#if 0
	int i;
	for (i = 0; i < e->sh_shstrtab.sh_size; i++) {

		unsigned char c = e->sh_shstrtab_data->data[i];
		if (c)
			printf("%c", c);
		else
			printf("\n");
	}
	printf("\n");
#endif
	return 0;
}

static int __elf_read_section_data(scf_elf_context_t* elf, elf_section_t* s)
{
	s->data_len = s->sh.sh_size;

	if (s->sh.sh_size > 0) {

		s->data = malloc(s->sh.sh_size);
		if (!s->data)
			return -1;

		fseek(elf->fp, elf->start + s->sh.sh_offset, SEEK_SET);

		int ret = fread(s->data, s->data_len, 1, elf->fp);
		if (ret != 1) {
			free(s->data);
			s->data = NULL;
			s->data_len = 0;
			return -1;
		}
	}

	return 0;
}

static int __elf_read_section_by_index(scf_elf_context_t* elf, elf_section_t** psection, const int index)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	if (!e->sh_shstrtab_data) {

		int ret = elf_read_shstrtab(elf);
		if (ret < 0)
			return ret;
	}

	if (index >= e->eh.e_shnum)
		return -EINVAL;

	elf_section_t* s;
	int i;
	for (i = 0; i < e->sections->size; i++) {
		s  =        e->sections->data[i];

		if (index == s->index) {

			if (s->data || __elf_read_section_data(elf, s) == 0) {
				*psection = s;
				return 0;
			}
			return -1;
		}
	}

	s = calloc(1, sizeof(elf_section_t));
	if (!s)
		return -ENOMEM;

	long offset = e->eh.e_shoff + e->eh.e_shentsize * index;
	fseek(elf->fp, elf->start + offset, SEEK_SET);

	int ret = fread(&s->sh, sizeof(Elf64_Shdr), 1, elf->fp);
	if (ret != 1) {
		free(s);
		return -1;
	}

	s->index = index;
	s->name  = scf_string_cstr(e->sh_shstrtab_data->data + s->sh.sh_name);
	if (!s->name) {
		free(s);
		return -1;
	}

	ret = scf_vector_add(e->sections, s);
	if (ret < 0) {
		scf_string_free(s->name);
		free(s);
		return -1;
	}

	if (__elf_read_section_data(elf, s) == 0) {
		*psection = s;
		return 0;
	}
	return -1;
}

int elf_read_phdrs(scf_elf_context_t* elf, scf_vector_t* phdrs)
{
	elf_native_t*   e = elf->priv;
	scf_elf_phdr_t* ph;
	Elf64_Ehdr      eh;

	if (!e || !elf->fp)
		return -1;

	if (!elf->fp)
		return -EINVAL;

	int ret = fseek(elf->fp, elf->start, SEEK_SET);
	if (ret < 0)
		return ret;

	ret = fread(&eh, sizeof(Elf64_Ehdr), 1, elf->fp);
	if (ret != 1)
		return -1;

	if (ELFMAG0    != eh.e_ident[EI_MAG0]
		|| ELFMAG1 != eh.e_ident[EI_MAG1]
		|| ELFMAG2 != eh.e_ident[EI_MAG2]
		|| ELFMAG3 != eh.e_ident[EI_MAG3]) {

		scf_loge("not elf file\n");
		return -1;
	}

	fseek(elf->fp, elf->start + eh.e_phoff, SEEK_SET);

	int i;
	for (i = 0; i < eh.e_phnum; i++) {

		ph = calloc(1, sizeof(scf_elf_phdr_t));
		if (!ph)
			return -ENOMEM;

		ret = fread(&ph->ph, sizeof(Elf64_Phdr), 1, elf->fp);

		if (ret != 1) {
			free(ph);
			return -1;
		}

		if (scf_vector_add(phdrs, ph) < 0) {
			free(ph);
			return -ENOMEM;
		}
	}

	return 0;
}

static int __elf_read_section(scf_elf_context_t* elf, elf_section_t** psection, const char* name)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	if (!e->sh_shstrtab_data) {

		int ret = elf_read_shstrtab(elf);
		if (ret < 0)
			return ret;
	}

	elf_section_t* s;
	int i;
	for (i = 0; i < e->sections->size; i++) {
		s  =        e->sections->data[i];

		if (!scf_string_cmp_cstr(s->name, name)) {

			if (s->data || __elf_read_section_data(elf, s) == 0) {
				*psection = s;
				return 0;
			}
			return -1;
		}
	}

	int j;
	for (j = 1; j < e->eh.e_shnum; j++) {

		for (i = 0; i < e->sections->size; i++) {
			s  =        e->sections->data[i];

			if (j == s->index)
				break;
		}

		if (i < e->sections->size)
			continue;

		s = calloc(1, sizeof(elf_section_t));
		if (!s)
			return -ENOMEM;

		long offset = e->eh.e_shoff + e->eh.e_shentsize * j;
		fseek(elf->fp, elf->start + offset, SEEK_SET);

		int ret = fread(&s->sh, sizeof(Elf64_Shdr), 1, elf->fp);
		if (ret != 1) {
			free(s);
			return -1;
		}

		s->index = j;
		s->name  = scf_string_cstr(e->sh_shstrtab_data->data + s->sh.sh_name);
		if (!s->name) {
			free(s);
			return -1;
		}

		ret = scf_vector_add(e->sections, s);
		if (ret < 0) {
			scf_string_free(s->name);
			free(s);
			return -1;
		}

		if (!scf_string_cmp_cstr(s->name, name))
			break;
	}

	if (j < e->eh.e_shnum) {

		if (!s->data) {
			if (__elf_read_section_data(elf, s) == 0) {
				*psection = s;
				return 0;
			}

			return -1;
		} else
			assert(s->data_len == s->sh.sh_size);

		*psection = s;
		return 0;
	}

	return -404;
}

int elf_read_syms(scf_elf_context_t* elf, scf_vector_t* syms, const char* sh_name)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	elf_section_t* symtab = NULL;
	elf_section_t* strtab = NULL;

	char* sh_strtab_name = NULL;

	if (!strcmp(sh_name, ".symtab"))
		sh_strtab_name = ".strtab";

	else if (!strcmp(sh_name, ".dynsym"))
		sh_strtab_name = ".dynstr";
	else
		return -EINVAL;

	int ret = __elf_read_section(elf, &symtab, sh_name);
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	ret = __elf_read_section(elf, &strtab, sh_strtab_name);
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	assert(symtab->data_len % sizeof(Elf64_Sym) == 0);

	scf_elf_sym_t* esym;
	Elf64_Sym*     sym;
	int i;
	for (i  = 0; i < symtab->data_len; i += sizeof(Elf64_Sym)) {

		sym = (Elf64_Sym*)(symtab->data + i);

		assert(sym->st_name < strtab->data_len);

		if (STT_NOTYPE == sym->st_info && 0 == i)
			continue;

		esym = calloc(1, sizeof(scf_elf_sym_t));
		if (!esym)
			return -ENOMEM;

		esym->name     = strtab->data + sym->st_name;
		esym->st_size  = sym->st_size;
		esym->st_value = sym->st_value;
		esym->st_shndx = sym->st_shndx;
		esym->st_info  = sym->st_info;

		if (scf_vector_add(syms, esym) < 0) {
			free(esym);
			return -ENOMEM;
		}
	}

	return 0;
}

int elf_add_dyn_need(scf_elf_context_t* elf, const char* soname)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	if (!e->dyn_needs) {
		e->dyn_needs = scf_vector_alloc();
		if (!e->dyn_needs)
			return -ENOMEM;
	}

	scf_string_t* s = scf_string_cstr(soname);
	if (!s)
		return -ENOMEM;

	if (scf_vector_add(e->dyn_needs, s) < 0) {
		scf_string_free(s);
		return -ENOMEM;
	}

	scf_loge("soname: %s\n", soname);
	return 0;
}

int elf_add_dyn_rela(scf_elf_context_t* elf, const scf_elf_rela_t* rela)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	if (!e->dyn_relas) {
		e->dyn_relas = scf_vector_alloc();
		if (!e->dyn_relas)
			return -ENOMEM;
	}

	Elf64_Rela* r = calloc(1, sizeof(Elf64_Rela));
	if (!r)
		return -ENOMEM;

	if (scf_vector_add(e->dyn_relas, r) < 0) {
		free(r);
		return -ENOMEM;
	}

	r->r_offset = rela->r_offset;
	r->r_addend = rela->r_addend;
	r->r_info   = rela->r_info;

	return 0;
}

int elf_read_relas(scf_elf_context_t* elf, scf_vector_t* relas, const char* sh_name)
{
	elf_native_t* e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	elf_section_t* sh_rela  = NULL;
	elf_section_t* symtab   = NULL;
	elf_section_t* strtab   = NULL;

	char* symtab_name = NULL;
	char* strtab_name = NULL;

	int ret = __elf_read_section(elf, &sh_rela, sh_name);
	if (ret < 0)
		return ret;

	scf_loge("sh_rela: %u\n", sh_rela->sh.sh_link);

	ret = __elf_read_section_by_index(elf, &symtab, sh_rela->sh.sh_link);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	ret = __elf_read_section_by_index(elf, &strtab, symtab->sh.sh_link);
	if (ret < 0) {
		scf_loge("\n");
		return ret;
	}

	assert(sh_rela->data_len % sizeof(Elf64_Rela) == 0);

	scf_elf_rela_t* erela;
	Elf64_Rela*     rela;
	Elf64_Sym*      sym;

	int i;
	for (i   = 0; i < sh_rela->data_len; i += sizeof(Elf64_Rela)) {

		rela = (Elf64_Rela*)(sh_rela->data + i);

		int sym_idx = ELF64_R_SYM(rela->r_info);

		assert(sym_idx < symtab->data_len / sizeof(Elf64_Sym));

		sym = (Elf64_Sym*)(symtab->data + sym_idx * sizeof(Elf64_Sym));

		assert(sym->st_name < strtab->data_len);

		erela = calloc(1, sizeof(scf_elf_rela_t));
		if (!erela)
			return -ENOMEM;

		erela->name     = strtab->data + sym->st_name;
		erela->r_offset = rela->r_offset;
		erela->r_info   = rela->r_info;
		erela->r_addend = rela->r_addend;

		if (scf_vector_add(relas, erela) < 0) {
			scf_loge("\n");
			free(erela);
			return -ENOMEM;
		}
	}

	return 0;
}

int elf_read_section(scf_elf_context_t* elf, scf_elf_section_t** psection, const char* name)
{
	elf_section_t* s   = NULL;
	scf_elf_section_t*     s2;
	elf_native_t*         e = elf->priv;

	if (!e || !elf->fp)
		return -1;

	int ret = __elf_read_section(elf, &s, name);
	if (ret < 0)
		return ret;

	s2 = calloc(1, sizeof(scf_elf_section_t));
	if (!s2)
		return -ENOMEM;

	s2->index        = s->index;
	s2->name         = s->name->data;
	s2->data         = s->data;
	s2->data_len     = s->data_len;
	s2->sh_type      = s->sh.sh_type;
	s2->sh_flags     = s->sh.sh_flags;
	s2->sh_addralign = s->sh.sh_addralign;

	*psection = s2;
	return 0;
}

int elf_write_symtab(scf_elf_context_t* elf)
{
	elf_native_t*  e  = elf->priv;
	Elf64_Sym      s0 = {0};
	elf_sym_t*     s;

	s0.st_info = ELF64_ST_INFO(STB_LOCAL, STT_NOTYPE);

	fwrite(&s0, sizeof(s0), 1, elf->fp); // entry index 0 in symtab is NOTYPE

	int i;
	for (i = 0; i < e->symbols->size; i++) {
		s         = e->symbols->data[i];

		fwrite(&s->sym, sizeof(s->sym), 1, elf->fp);
	}

	return 0;
}

int elf_write_strtab(scf_elf_context_t* elf)
{
	elf_sym_t*     s;
	elf_native_t*  e  = elf->priv;
	uint8_t        c  = 0;

	fwrite(&c, sizeof(c), 1, elf->fp);

	int i;
	for (i = 0; i < e->symbols->size; i++) {
		s         = e->symbols->data[i];

		if (s->name)
			fwrite(s->name->data, s->name->len + 1, 1, elf->fp);
	}

	return 0;
}

int elf_write_shstrtab(scf_elf_context_t* elf)
{
	elf_section_t* s;
	elf_native_t*  e   = elf->priv;
	uint8_t        c   = 0;
	char*          str = ".symtab\0.strtab\0.shstrtab\0";
	int            len = strlen(".symtab") + strlen(".strtab") + strlen(".shstrtab") + 3;

	fwrite(&c, sizeof(c), 1, elf->fp);

	int i;
	for (i = 0; i < e->sections->size; i++) {
		s  =        e->sections->data[i];

		fwrite(s->name->data, s->name->len + 1, 1, elf->fp);
	}

	fwrite(str, len, 1, elf->fp);
	return 0;
}

int elf_write_sections(scf_elf_context_t* elf)
{
	elf_section_t* s;
	elf_native_t*  e   = elf->priv;

	int i;
	for (i = 0; i < e->sections->size; i++) {
		s  =        e->sections->data[i];

		scf_loge("sh->name: %s, data: %p, len: %d\n", s->name->data, s->data, s->data_len);

		if (s->data && s->data_len > 0)
			fwrite(s->data, s->data_len, 1, elf->fp);
	}

	return 0;
}

int elf_write_rel(scf_elf_context_t* elf, uint16_t e_machine)
{
	elf_native_t*  e           = elf->priv;
	elf_section_t* s;
	elf_sym_t*     sym;

	int            nb_sections = 1 + e->sections->size + 1 + 1 + 1;
	uint64_t       shstrtab    = 1;
	uint64_t       strtab      = 1;
	uint64_t       symtab      = sizeof(Elf64_Sym ) * (e->symbols->size + 1);
	Elf64_Off      offset      = sizeof(Elf64_Ehdr) + sizeof(Elf64_Shdr) * nb_sections;

	// write elf header
	elf_header(&e->eh, ET_REL, e_machine, 0, 0, 0, nb_sections, nb_sections - 1);
	fwrite    (&e->eh, sizeof(e->eh), 1, elf->fp);

	// write null section header
	fwrite(&e->sh_null, sizeof(e->sh_null), 1, elf->fp);

	// write user's section header
	int i;
	for (i = 0; i < e->sections->size; i++) {
		s  =        e->sections->data[i];

		if (SHT_RELA == s->sh.sh_type)
			s->sh.sh_link = nb_sections - 3;

		section_header(&s->sh, shstrtab, 0, offset,
				s->data_len,
				s->sh.sh_link,
				s->sh.sh_info,
				s->sh.sh_entsize);

		s->sh.sh_addralign = 8;

		offset   += s->data_len;
		shstrtab += s->name->len + 1;

		fwrite(&s->sh, sizeof(s->sh), 1, elf->fp);
	}

	// set user's symbols' name
	int local_syms = 1;

	for (i = 0; i < e->symbols->size; i++) {
		sym       = e->symbols->data[i];

		if (sym->name) {
			sym->sym.st_name = strtab;
			strtab	 += sym->name->len + 1;
		} else
			sym->sym.st_name = 0;

		if (STB_LOCAL == ELF64_ST_BIND(sym->sym.st_info))
			local_syms++;
	}

	scf_loge("e->symbols->size: %d\n", e->symbols->size);
	// .symtab header
	section_header(&e->sh_symtab, shstrtab, 0, offset, symtab, nb_sections - 2, local_syms, sizeof(Elf64_Sym));

	offset   += symtab;
	shstrtab += strlen(".symtab") + 1;

	// .strtab header
	section_header(&e->sh_strtab, shstrtab, 0, offset, strtab, 0, 0, 0);

	offset   += strtab;
	shstrtab += strlen(".strtab") + 1;

	// .shstrtab header
	uint64_t shstrtab_len = shstrtab + strlen(".shstrtab") + 1;

	section_header(&e->sh_shstrtab, shstrtab, 0, offset, shstrtab_len, 0, 0, 0);

	fwrite(&e->sh_symtab,   sizeof(e->sh_symtab),   1, elf->fp);
	fwrite(&e->sh_strtab,   sizeof(e->sh_strtab),   1, elf->fp);
	fwrite(&e->sh_shstrtab, sizeof(e->sh_shstrtab), 1, elf->fp);

	elf_write_sections(elf);  // write user's section data

	elf_write_symtab  (elf);  // write .symtab   data (user's  symbols)
	elf_write_strtab  (elf);  // write .strtab   data (symbol  names of symtab)
	elf_write_shstrtab(elf);  // write .shstrtab data (section names of all sections)
	return 0;
}

int elf_sym_cmp(const void* v0, const void* v1)
{
	const elf_sym_t* sym0 = *(const elf_sym_t**)v0;
	const elf_sym_t* sym1 = *(const elf_sym_t**)v1;

	if (STB_LOCAL == ELF64_ST_BIND(sym0->sym.st_info)) {

		if (STB_GLOBAL == ELF64_ST_BIND(sym1->sym.st_info))
			return -1;

	} else if (STB_LOCAL == ELF64_ST_BIND(sym1->sym.st_info))
		return 1;

	return 0;
}

int elf_find_sym(elf_sym_t** psym, Elf64_Rela* rela, scf_vector_t* symbols)
{
	elf_sym_t*  sym;
	elf_sym_t*  sym2;

	int sym_idx = ELF64_R_SYM(rela->r_info);
	int j;

	assert(sym_idx >= 1);
	assert(sym_idx -  1 < symbols->size);

	sym = symbols->data[sym_idx - 1];

	if (0 == sym->sym.st_shndx) {

		int n = 0;

		for (j   = 0; j < symbols->size; j++) {
			sym2 =        symbols->data[j];

			if (0 == sym2->sym.st_shndx)
				continue;

			if (STB_LOCAL == ELF64_ST_BIND(sym2->sym.st_info))
				continue;

			if (!strcmp(sym2->name->data, sym->name->data)) {
				sym     = sym2;
				sym_idx = j + 1;
				n++;
			}
		}

		if (0 == n) {
			scf_loge("global symbol: %s not found\n", sym->name->data);
			return -1;

		} else if (n > 1) {
			scf_loge("tow global symbol: %s\n", sym->name->data);
			return -1;
		}
	} else if (ELF64_ST_TYPE(sym->sym.st_info) == STT_SECTION) {

		for (j   = symbols->size - 1; j >= 0; j--) {
			sym2 = symbols->data[j];

			if (ELF64_ST_TYPE(sym2->sym.st_info) != STT_SECTION)
				continue;

			if (sym2->sym.st_shndx == sym->sym.st_shndx) {
				sym     = sym2;
				sym_idx = j + 1;
				break;
			}
		}

		assert(j >= 0);
	}

	*psym = sym;
	return sym_idx;
}

void elf_process_syms(elf_native_t* e, uint32_t cs_index)
{
	elf_section_t* s;
	Elf64_Rela*    rela;
	elf_sym_t*     sym;

	int i;
	int j;
	int k;
	int shndx = 20;
#if 1
	for (i  = e->symbols->size - 1; i >= 0; i--) {
		sym = e->symbols->data[i];

		if (STT_SECTION == ELF64_ST_TYPE(sym->sym.st_info)) {
			if (shndx > cs_index) {

				shndx = sym->sym.st_shndx;

				assert(sym->sym.st_shndx - 1 < e->sections->size);

				sym->section = e->sections->data[sym->sym.st_shndx - 1];
				continue;
			}
		} else if (0 != sym->sym.st_shndx) {

			if (sym->sym.st_shndx - 1 < e->sections->size)
				sym->section = e->sections->data[sym->sym.st_shndx - 1];
			continue;
		}

		assert(0 == scf_vector_del(e->symbols, sym));

		scf_string_free(sym->name);
		free(sym);
	}
#endif
	qsort(e->symbols->data, e->symbols->size, sizeof(void*), elf_sym_cmp);

	for (j = 0; j < e->sections->size; j++) {
		s  =        e->sections->data[j];

		if (SHT_RELA != s->sh.sh_type)
			continue;

		if (!strcmp(s->name->data, ".rela.plt"))
			continue;

		assert(s->data_len % sizeof(Elf64_Rela) == 0);

		int sym_idx;
		for (k = 0; k < s->data_len; k += sizeof(Elf64_Rela)) {

			rela    = (Elf64_Rela*)(s->data + k);

			sym_idx = ELF64_R_SYM(rela->r_info);

			for (i  = 0; i < e->symbols->size; i++) {
				sym =        e->symbols->data[i];

				if (sym_idx == sym->index)
					break;
			}

			assert(i < e->symbols->size);

			rela->r_info = ELF64_R_INFO(i + 1, ELF64_R_TYPE(rela->r_info));
		}
	}
}

