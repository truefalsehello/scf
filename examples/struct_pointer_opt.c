int printf(const char* fmt, ...);

struct S {
	int* p0;
	int* p1;
};

int f()
{
	int a = 1;
	int b = 2;
	S   s = {&a, &b};

	int** pp = &s->p0;

	**pp += 3;
	return a;
}

int main()
{
	printf("%d\n", f());
	return 0;
}
