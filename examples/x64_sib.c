int printf(const char* fmt, ...);

int main()
{
	int a[] = {1, 2, 3, 4, 5};

	int i;
	for (i = 0; i < 5; i++)
		printf("%d\n", a[i]);

	return 0;
}
