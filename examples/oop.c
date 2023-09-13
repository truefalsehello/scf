int printf(const char* fmt, ...);

int Apt(int i, int j);

struct Aops
{
	Apt* pt0;
	Apt* pt1;
};

int add(int i, int j)
{
	return i + j;
}

int sub(int i, int j)
{
	return i - j;
}

Aops aops =
{
	add,
	sub,
};

int main()
{
	int a = aops->pt0(3, 1);
	int b = aops->pt1(3, 1);

	printf("a: %d, b: %d\n", a, b);
	return 0;
}
