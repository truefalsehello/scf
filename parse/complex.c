
include "../lib/scf_capi.c";

struct complex
{
	double real;
	double imag;

	int __init(complex* this, double real, double imag)
	{
		this->real = real;
		this->imag = imag;
		return 0;
	}

/*	int operator+=(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		this->real += that->real;
		this->imag += that->imag;
		return 0;
	}

	int operator-=(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		this->real -= that->real;
		this->imag -= that->imag;
		return 0;
	}

	int operator*=(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		double a = this->real;
		double b = this->imag;
		double c = that->real;
		double d = that->imag;

		this->real = a * c - b * d;
		this->imag = a * d + b * c;
		return 0;
	}

	int operator/=(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		double a = this->real;
		double b = this->imag;
		double c = that->real;
		double d = that->imag;

		this->real = (a * c + b * d) / (c * c + d * d);
		this->imag = (b * c - a * d) / (c * c + d * d);
		return 0;
	}
*/
	complex*, int operator+(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		complex* res;

		res = create complex(0.0, 0.0);
		if (!res)
			return NULL, -1;

		res->real = this->real + that->real;
		res->imag = this->imag + that->imag;
		return res, 0;
	}

	complex*, int operator-(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		complex* res;

		res = create complex(0.0, 0.0);
		if (!res)
			return NULL, -1;

		res->real = this->real - that->real;
		res->imag = this->imag - that->imag;
		return res, 0;
	}

	complex*, int operator*(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		complex* res;

		res = create complex(0.0, 0.0);
		if (!res)
			return NULL, -1;

		double a = this->real;
		double b = this->imag;
		double c = that->real;
		double d = that->imag;

		res->real = a * c - b * d;
		res->imag = a * d + b * c;
		return res, 0;
	}

	complex*, int operator/(complex* this, complex* that)
	{
		if (!this || !that)
			return -1;

		complex* res;

		res = create complex(0.0, 0.0);
		if (!res)
			return NULL, -1;

		double a = this->real;
		double b = this->imag;
		double c = that->real;
		double d = that->imag;

		res->real = (a * c + b * d) / (c * c + d * d);
		res->imag = (b * c - a * d) / (c * c + d * d);
		return res, 0;
	}
};

int main()
{
	complex* c0;
	complex* c1;
	complex* c2;
	complex* c3;

	c0 = create complex(1.0, 2.0);
	c1 = create complex(3.0, 4.0);

	c2 = c0 + c1;
	c3 = c0 * c1;

	printf("c2: %lf,%lf\n", c2->real, c2->imag);
	printf("c3: %lf,%lf\n", c3->real, c3->imag);

	return 0;
}

