#include"scf_pack.h"

typedef struct {
	SCF_PACK_DEF_VAR(int, x); // int x;
	SCF_PACK_DEF_VAR(int, y); // int y;
} A;

SCF_PACK_TYPE(A)
SCF_PACK_INFO_VAR(A, x),
SCF_PACK_INFO_VAR(A, y),
SCF_PACK_END(A)


int main()
{
	A  a = {1, 2};
	A* p = NULL;

	uint8_t* buf = NULL;
	int      len = 0;

	scf_A_pack(&a, &buf, &len);

	int i;
	for (i = 0; i < len; i++)
		printf("i: %d, %#x\n", i, buf[i]);

	scf_A_unpack(&p, buf, len);
	printf("p->x: %d, p->y: %d\n", p->x, p->y);

	scf_A_free(p);
	free(buf);
	return 0;
}
