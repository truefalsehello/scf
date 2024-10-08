int printf(const char* fmt, ...);

int sort(int* a, int m, int n)
{
	if (m >= n)
		return 0;

	int i = m;
	int j = n;
	int t;

	t = a[i];
	while (i < j) {

		while (i < j && t <= a[j])
			j--;
		a[i]  = a[j];
		a[j]  = t;

		while (i < j && a[i] <= t)
			i++;
		a[j]  = a[i];
		a[i]  = t;
	}

	sort(a, m, i - 1);
	sort(a, i + 1, n);
	return 0;
}

int main()
{
	int a[20] = {1, 3, 5, 7, 9, 8, 4, 2, 6, 0,
		11, 13, 15, 17, 19, 18, 16, 14, 12, 10};

	sort(a, 0, 19);

	int i;
	for (i = 0; i < 20; i++)
		printf("%d\n", a[i]);

	return 0;
}
