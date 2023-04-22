#include"scf_parse.h"
#include"scf_3ac.h"
#include"scf_x64.h"
#include"scf_elf_link.h"

static char* __objs[] =
{
	"../lib/_start.o",
	"../lib/scf_object.o",
	"../lib/scf_atomic.o",
};

static char* __sofiles[] =
{
	"../lib/x64//lib64/ld-linux-x86-64.so.2",
	"../lib/x64/libc.so.6",
};

static char* __arm64_objs[] =
{
	"../lib/arm64/_start.o",
};

static char* __arm64_sofiles[] =
{
	"../lib/arm64//lib/ld-linux-aarch64.so.1",
	"../lib/arm64/libc.so.6",
};

void usage(char* path)
{
	fprintf(stderr, "Usage: %s [-c] [-a arch] [-o out] src0 [src1]\n\n", path);
	fprintf(stderr, "-c: only compile, not link\n");
	fprintf(stderr, "-a: select cpu arch (x64 or arm64), default is x64\n");
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		usage(argv[0]);
		return -EINVAL;
	}

	scf_vector_t* afiles  = scf_vector_alloc();
	scf_vector_t* sofiles = scf_vector_alloc();
	scf_vector_t* srcs    = scf_vector_alloc();
	scf_vector_t* objs    = scf_vector_alloc();

	char* out  = NULL;
	char* arch = "x64";
	int   link = 1;

	int   i;

	for (i = 1; i < argc; i++) {

		if ('-' == argv[i][0]) {

			if ('c' == argv[i][1]) {
				link = 0;
				continue;
			}

			if ('a' == argv[i][1]) {

				if (++i >= argc) {
					usage(argv[0]);
					return -EINVAL;
				}

				arch = argv[i];
				continue;
			}

			if ('o' == argv[i][1]) {

				if (++i >= argc) {
					usage(argv[0]);
					return -EINVAL;
				}

				out = argv[i];
				continue;
			}

			usage(argv[0]);
			return -EINVAL;
		}

		char*  fname = argv[i];
		size_t len   = strlen(fname);

		if (len < 3) {
			fprintf(stderr, "file '%s' invalid\n", fname);
			return -1;
		}

		scf_loge("fname: %s\n", fname);

		scf_vector_t* vec;

		if (!strcmp(fname + len - 2, ".a"))
			vec = afiles;

		else if (!strcmp(fname + len - 2, ".o"))
			vec = objs;

		else if (!strcmp(fname + len - 2, ".c"))
			vec = srcs;

		else if (!strcmp(fname + len - 3, ".so"))
			vec = sofiles;
		else {
			fprintf(stderr, "file '%s' invalid\n", fname);
			return -1;
		}

		if (scf_vector_add(vec, fname) < 0)
			return -ENOMEM;
	}

	scf_parse_t*  parse = NULL;

	if (scf_parse_open(&parse) < 0) {
		scf_loge("\n");
		return -1;
	}

	for (i = 0; i  < srcs->size; i++) {
		char* file = srcs->data[i];

		if (scf_parse_file(parse, file) < 0) {
			scf_loge("\n");
			return -1;
		}
	}

	char* obj  = "1.elf";
	char* exec = "1.out";

	if (out) {
		if (!link)
			obj  = out;
		else
			exec = out;
	}

	if (scf_parse_compile(parse, obj, arch) < 0) {
		scf_loge("\n");
		return -1;
	}

	scf_parse_close(parse);

	if (!link) {
		printf("%s(),%d, main ok\n", __func__, __LINE__);
		return 0;
	}

#define MAIN_ADD_FILES(_objs, _sofiles) \
	do { \
		for (i  = 0; i < sizeof(_objs) / sizeof(_objs[0]); i++) { \
			\
			int ret = scf_vector_add(objs, _objs[i]); \
			if (ret < 0) \
			return ret; \
		} \
		\
		for (i  = 0; i < sizeof(_sofiles) / sizeof(_sofiles[0]); i++) { \
			\
			int ret = scf_vector_add(sofiles, _sofiles[i]); \
			if (ret < 0) \
			return ret; \
		} \
	} while (0)


	if (!strcmp(arch, "arm64") || !strcmp(arch, "naja"))
		MAIN_ADD_FILES(__arm64_objs, __arm64_sofiles);
	else
		MAIN_ADD_FILES(__objs, __sofiles);


	if (scf_vector_add(objs, obj) < 0) {
		scf_loge("\n");
		return -1;
	}

	if (scf_elf_link(objs, afiles, sofiles, arch, exec) < 0) {
		scf_loge("\n");
		return -1;
	}

	printf("%s(),%d, main ok\n", __func__, __LINE__);
	return 0;
}

