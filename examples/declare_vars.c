//第5章/setcc.c

int printf(const char* fmt, ...);

int main()
{
	int a = 1, b = 2, c = 3, d = 4;

	int ret = a > b && b < c || c < d;

	printf("ret: %d\n", ret);
	return 0;
}
