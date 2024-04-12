int printf(const char* fmt, ...);

int main()
{
	int a = 0;

	switch (a) {
		case 0:
			printf("0\n");
		case 1:
			printf("1\n");
			break;
		default:
			printf("default\n");
			break;
	};

	return 0;
}
