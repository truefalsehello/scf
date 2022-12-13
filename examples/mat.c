
include "../lib/scf_capi.c";

const int    MAT_TYPE_NONE   = 0;
const int    MAT_TYPE_U8     = 1;
const int    MAT_TYPE_DOUBLE = 2;

struct mat;

struct mat
{
	uint8_t* data;

	int64_t  z;
	int64_t  x;
	int64_t  y;
	int64_t  c;

	int64_t  depth;
	int64_t  width;
	int64_t  height;
	int64_t  count;

	int64_t  xstep;
	int64_t  ystep;
	int64_t  zstep;
	int64_t  cstep;

	int64_t  xstride;
	int      type;

	int __init(mat* this, int type, uint8_t* data, int depth, int width, int height, int count)
	{
		this->z = 0;
		this->x = 0;
		this->y = 0;
		this->c = 0;

		this->depth  = depth;
		this->width  = width;
		this->height = height;
		this->count  = count;

		this->xstep  = 1;
		this->ystep  = 1;
		this->zstep  = 1;
		this->cstep  = 1;
		this->type   = type;

		this->xstride = this->width;
		int64_t size  = this->depth * this->width * this->height * this->count;

		if (MAT_TYPE_U8 == type) {

		} else if (MAT_TYPE_DOUBLE == type) {
			size *= sizeof(double);
		} else
			return -1;

		this->data = scf__auto_malloc(size);
		if (!this->data)
			return -1;

		if (data)
			memcpy(this->data, data, size);
		return 0;
	}

/*	int operator+=(mat* this, mat* that)
	{
		if (this->depth != that->depth
				|| this->width  != that->width
				|| this->height != that->height
				|| this->count  != that->count
				|| this->type   != that->type)
			return -1;

		if (MAT_TYPE_DOUBLE != this->type)
			return -1;

		int64_t c;
		int64_t y;
		int64_t x;
		int64_t z;

		double* d0 = (double*)this->data;
		double* d1 = (double*)that->data;

		for (c = 0; c < this->count; c++) {

			int64_t c0 = this->c + this->cstep * c;
			int64_t c1 = that->c + that->cstep * c;

			int64_t coffset0 = c0 * this->height * this->xstride;
			int64_t coffset1 = c1 * that->height * that->xstride;

			for (y = 0; y < this->height; y++) {

				int64_t y0 = this->y + this->ystep * y;
				int64_t y1 = that->y + that->ystep * y;

				int64_t yoffset0 = y0 * this->xstride * this->depth;
				int64_t yoffset1 = y1 * that->xstride * that->depth;

				for (x = 0; x < this->width; x++) {

					int64_t x0 = this->x + this->xstep * x;
					int64_t x1 = that->x + that->xstep * x;

					int64_t xoffset0 = x0 * this->depth;
					int64_t xoffset1 = x1 * that->depth;

					for (z = 0; z < this->depth; z++) {

						int64_t z0 = this->z + this->zstep * z;
						int64_t z1 = that->z + that->zstep * z;

						d0[coffset0 + yoffset0 + xoffset0 + z0] += d1[coffset1 + yoffset1 + xoffset1 + z1];
					}
				}
			}
		}

		return 0;
	}
*/
	mat*, int operator+(mat* this, mat* that)
	{
/*		if (this->depth != that->depth
				|| this->width  != that->width
				|| this->height != that->height
				|| this->count  != that->count
				|| this->type   != that->type)
			return NULL, -1;

		if (MAT_TYPE_DOUBLE != this->type)
			return NULL, -1;
*/
		mat* res;

		res = create mat(MAT_TYPE_DOUBLE, NULL, this->depth, this->width, this->height, this->count);
		if (!res)
			return NULL, -1;

		int64_t c;
		int64_t y;
		int64_t x;
		int64_t z;

		double* d0 = (double*)this->data;
		double* d1 = (double*)that->data;
		double* d2 = (double*)res->data;

		for (c = 0; c < this->count; c++) {

			int64_t c0 = this->c + this->cstep * c;
			int64_t c1 = that->c + that->cstep * c;

			int64_t coffset0 = c0 * this->height * this->xstride;
			int64_t coffset1 = c1 * that->height * that->xstride;
			int64_t coffset2 = c  * res->height  * res->xstride;

			for (y = 0; y < this->height; y++) {

				int64_t y0 = this->y + this->ystep * y;
				int64_t y1 = that->y + that->ystep * y;

				int64_t yoffset0 = y0 * this->xstride * this->depth;
				int64_t yoffset1 = y1 * that->xstride * that->depth;
				int64_t yoffset2 = y  * res->xstride  * res->depth;

				for (x = 0; x < this->width; x++) {

					int64_t x0 = this->x + this->xstep * x;
					int64_t x1 = that->x + that->xstep * x;

					int64_t xoffset0 = x0 * this->depth;
					int64_t xoffset1 = x1 * that->depth;
					int64_t xoffset2 = x  * res->depth;

					for (z = 0; z < this->depth; z++) {

						int64_t z0 = this->z + this->zstep * z;
						int64_t z1 = that->z + that->zstep * z;

						d2[coffset2 + yoffset2 + xoffset2 + z] = d1[coffset1 + yoffset1 + xoffset1 + z1] + d0[coffset0 + yoffset0 + xoffset0 + z0];
					}
				}
			}
		}

		return res, 0;
	}

	void __release(mat* this)
	{
		if (this->data)
			scf__auto_freep(&this->data, NULL);
	}
};


int main()
{
	double a[4] = {1, 2,  3,  4};
	double b[4] = {5, 6,  7,  8};
	double c[4] = {9, 10, 11, 12};

	mat* m0;
	mat* m1;
	mat* m2;
	mat* m3;
	mat* m4;

	m0 = create mat(MAT_TYPE_DOUBLE, (uint8_t*)a, 1, 2, 2, 1);
	m1 = create mat(MAT_TYPE_DOUBLE, (uint8_t*)b, 1, 2, 2, 1);
	m2 = create mat(MAT_TYPE_DOUBLE, (uint8_t*)c, 1, 2, 2, 1);

	m3 = m0 + m1 + m2;
//	scf_printf("%s(),%d\n", __func__, __LINE__);

	double* dd = (double*)m3->data;

	int i;
	for (i = 0; i < 4; i++)
		printf("m3: %lf\n", dd[i]);
//		scf_printf("m1: %lf\n", *(double*)(m1->data + i * sizeof(double)));

	return 0;
}

