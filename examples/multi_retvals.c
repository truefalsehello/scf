int printf(const char* fmt, ...);

int, int ret3value(int i, int j, int k)
{
	return i, j;
}

int main()
{
	int a, b, c;

	a, b = ret3value(1, 2, 3);

	printf("%d, %d\n", a, b);
//	printf("%d, %d, %d\n", ret3value(1, 2, 3));

	return 0;
}

