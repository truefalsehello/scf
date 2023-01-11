int printf(const char* fmt, ...);

int count = 0;

void hanoi(int n, char a, char b, char c)
{
	if (1 == n)
		printf("count: %d, n: %d, %c->%c\n", count++, n, a, c);
	else {
		hanoi(n-1, a, c, b);
		printf("count: %d, n: %d, %c->%c\n", count++, n, a, c);
		hanoi(n-1, b, a, c);
	}
}

int main()
{
	hanoi(4, 'A', 'B', 'C');
	return 0;
}
