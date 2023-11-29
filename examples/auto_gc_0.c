include "../lib/scf_capi.c";

int* f()
{
	return scf__auto_malloc(sizeof(int));
}

int main()
{
	int* p = f();

	*p = 1;

	printf("%d\n", *p);
	return 0;
}
