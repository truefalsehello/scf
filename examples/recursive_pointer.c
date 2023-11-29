include "../lib/scf_capi.c";

int f(int** t0, int **t1);

int g(int** s0, int** s1)
{
	if (!*s1)
		*s1 = scf__auto_malloc(sizeof(int));

	if (!*s0)
		f(s0, s1);
	return 0;
}

int f(int** t0, int **t1)
{
	if (!*t0)
		*t0 = scf__auto_malloc(sizeof(int));

	if (!*t1)
		g(t0, t1);
	return 0;
}

int main()
{
	int* p0 = NULL;
	int* p1 = NULL;

	f(&p0, &p1);

	*p0 = 1;
	*p1 = 2;

	printf("%d, %d\n", *p0, *p1);
	return 0;
}
