#include"scf_pack.h"

typedef struct {
	SCF_PACK_DEF_VAR(int, x); // int x;
	SCF_PACK_DEF_VAR(int, y); // int y;
	SCF_PACK_DEF_VARS(int, z);
} A;

SCF_PACK_TYPE(A)
SCF_PACK_INFO_VAR(A, x),
SCF_PACK_INFO_VAR(A, y),
SCF_PACK_INFO_VARS(A, z, int),
SCF_PACK_END(A)

typedef struct {
	SCF_PACK_DEF_VAR(double, d);
	SCF_PACK_DEF_OBJS(A, as);
} B;

SCF_PACK_TYPE(B)
SCF_PACK_INFO_VAR(B, d),
SCF_PACK_INFO_OBJS(B, as, A),
SCF_PACK_END(B)

int main()
{
	int z0[] = {1, 2, 3, 4};
	int z1[] = {5, 6};
	A  a0 = {0x90, 0xff, sizeof(z0) / sizeof(z0[0]), z0};
	A  a1 = {9, 10, sizeof(z1) / sizeof(z1[0]), z1};
	A* as[] = {&a0, &a1};

	B  b = {3.14, sizeof(as) / sizeof(as[0]), as};
	B* p = NULL;

	printf("b: %p, b->as: %p, b->as[0]: %p, b->as[1]: %p\n", &b, b.as, b.as[0], b.as[1]);
	printf("a0: %p, a1: %p\n", &a0, &a1);
	printf("z0: %p, z1: %p\n", z0, z1);

	uint8_t* buf = NULL;
	int      len = 0;

	B_pack(&b, &buf, &len);

	int i;
	int j;
	for (i = 0; i < len; i++)
		printf("i: %d, %#x\n", i, buf[i]);
#if 1
	B_unpack(&p, buf, len);
	printf("p: %p, p->as: %p\n", p, p->as);
	printf("p->d: %lg, p->n_as: %ld, p->as: %p\n", p->d, p->n_as, p->as);

	for (i = 0; i < p->n_as; i++) {
		printf("p->as[%d]: %p\n", i, p->as[i]);

		A* a = p->as[i];
		printf("a->x: %#x, a->y: %#x, a->n_z: %ld,a->z: %p\n", a->x,a->y, a->n_z, a->z);

		for (j = 0; j < a->n_z; j++)
			printf("a->z[%d]: %d\n", j, a->z[j]);
		printf("\n");
	}

	B_free(p);
	free(buf);
#endif
	return 0;
}
