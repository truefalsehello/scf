include "../lib/scf_capi.c";
include "../lib/scf_complex.c";

int main()
{
	complex* c1;
	complex* c2;
	complex* c3;

	complex* c0 = create complex(1.0, 2.0);

	c1 = create complex(3.0, 4.0);

	c2 = c0 + c1;
	c3 = c0 * c1;

	printf("c2: %lf,%lf\n", c2->real, c2->imag);
	printf("c3: %lf,%lf\n", c3->real, c3->imag);

	return 0;
}

