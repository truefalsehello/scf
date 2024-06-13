int printf(const char* fmt, ...);

int inc(int i)
{
	return ++i;
}

int main()
{
	int i = 1;

	printf("%d\n", inc(i));
	return 0;
}
