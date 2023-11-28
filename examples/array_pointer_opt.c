int printf(const char* fmt, ...);

int f()
{
	int a = 1;
	int b = 2;

	int*  pa[2] = {&a, &b};

	int** pp = &pa[0];

	**pp += 3;
	return a;
}

int main()
{
	int i = 1;
	int a = 2;
	int b = 3;

	printf("%d\n", f());
	return 0;
}
