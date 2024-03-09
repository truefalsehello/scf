#include"scf_pack.h"

int __scf_pack_one_index(uint8_t* pack, uint64_t u, int shift)
{
	int max  = -1;
	int bits = 1u << shift;
	int i;
	int j;
	int k;

	j = 0;
	k = 3;
	for (i = bits - 1; i >= 0; i--) {

		if (!(u & (1ull << i)))
			continue;

		if (-1 == max)
			max = i;
		else
			max -= i;
		scf_logd("max: %d, i: %d, j: %d, k: %d, shift: %d\n", max, i, j, k, shift);

		pack[j] |= max << k;

		if (8 - k < shift) {
			pack[++j] = max >> (8 - k);
			k = shift - (8 - k);
		} else
			k += shift;

		if (i < 2)
			shift  = 1;
		else if (i < 4)
			shift  = 2;

		else if (i < 8)
			shift  = 3;
		else if (i < 16)
			shift  = 4;

		else if (i < 32)
			shift  = 5;
		else if (i < 64)
			shift  = 6;
		else {
			scf_loge("pack error\n");
			return -EINVAL;
		}

		scf_logd("max: %d, i: %d, j: %d, k: %d, shift: %d\n\n", max, i, j, k, shift);

		max = i;
	}

	if (8 - k < shift && 0 != max) {
		pack[++j] = 0;
		scf_logd("max: %d, i: %d, j: %d, k: %d, shift: %d\n\n", max, i, j, k, shift);
	}

	return j + 1;
}

int __scf_pack_byte_map(uint8_t* pack, uint64_t u, int shift)
{
	uint8_t* p = (uint8_t*)&u;
	uint8_t map = 0;

	int bytes = 1u << shift >> 3;
	int i;
	int j;
	int k;

	pack[0] |= 0x4;
	j = 0;
	k = 4;

	for (i = 0; i < bytes; i++) {
		if (p[i])
			map |= 1u << i;
	}

	if (((1u << bytes) - 1) & ~map) {
		pack[0] |= 0x8;
		pack[j] |= map << k;

		if (bytes > k) {
			pack[++j] = map >> (8 - k);
			k = bytes - k;
		} else
			k += bytes;
	}

	for (i = 0; i < bytes; i++) {
		if (p[i]) {
			pack[++j] = p[i];
			scf_logd("bytes: %d, map: %#x, i: %d, j: %d, p[i]: %#x\n", bytes, map, i, j, p[i]);
		}
	}

	return j + 1;
}

int __scf_pack2(uint8_t* pack, uint64_t u, int shift)
{
	int sum  = 0;
	int not  = 0;
	int bits = 1u << shift;
	int i;

	for (i = 0; i < bits; i++) {
		if (u & (1ull << i))
			sum++;
	}

	if (sum > (bits >> 1)) { // bits / 2
		not = 1;
		sum = bits - sum;
		u = ~u;
	}

	scf_logd("bits: %d, not: %d, u: %#lx, sum: %d\n", bits, not, u, sum);

	pack[0] = not;

	if (u < 64) {
		pack[0] |= u << 2;
		return 1;
	}

	pack[0] |= 0x2;

	if (sum < 4)
		return __scf_pack_one_index(pack, u, shift);
	return __scf_pack_byte_map(pack, u, shift);
}

int __scf_unpack2(void* p, int shift, const uint8_t* buf, int len)
{
	int bits = 1u << shift;
	int max  = -1;
	int i;
	int j;
	int k;

	if (len < 1)
		return -EINVAL;

	uint64_t u = 0;

	if (!(buf[0] & 0x2)) {

		u = buf[0] >> 2;
		j = 1;

	} else if (!(buf[0] & 0x4)) {
		j = 0;
		k = 3;

		while (1) {
			if (j >= len)
				return -EINVAL;

			i = (buf[j] >> k) & ((1u << shift) - 1);

			if (shift > 8 - k) {
				if (++j >= len)
					return -EINVAL;

				i |= buf[j] << (8 - k);
				i &= (1u << shift) - 1;
				k  = shift - (8 - k);
			} else
				k += shift;

			if (-1 == max)
				max = i;
			else if (0 == i)
				break;
			else
				max -= i;

			u |= 1ull << max;

			if (max < 2)
				shift  = 1;
			else if (max < 4)
				shift  = 2;

			else if (max < 8)
				shift  = 3;
			else if (max < 16)
				shift  = 4;

			else if (max < 32)
				shift  = 5;
			else if (max < 64)
				shift  = 6;
			else {
				scf_loge("unpack error\n");
				return -EINVAL;
			}

			scf_logd("max: %d, i: %d, j: %d, k: %d, shift: %d\n\n", max, i, j, k, shift);
		}

		j++;
		scf_logd("u: %ld, %#lx\n", u, u);

	} else if (!(buf[0] & 0x8)) {
		j = 1;

		for (k = 0; k < bits / 8; k++) {

			if (j >= len)
				return -EINVAL;

			uint64_t u8 = buf[j];

			u |= u8 << (k << 3);

			scf_logd("buf[%d]: %#x, u: %#lx, k: %d, bits: %d\n", j, buf[j], u, k, bits);
			j++;
		}

	} else {
		uint8_t map = buf[0] >> 4;

		if (bits > 32) {
			if (1 >= len)
				return -EINVAL;

			map |= buf[1] << 4;
			j = 2;
		} else
			j = 1;

		for (k = 0; k < bits / 8; k++) {
			if (!(map & (1u << k)))
				continue;

			if (j >= len)
				return -EINVAL;

			uint64_t u8 = buf[j++];

			u |= u8 << (k << 3);

			scf_logd("buf[%d]: %#x, u: %#lx, k: %d, map: %#x, bits: %d\n", j, buf[j], u, k, map, bits);
		}
	}

	if (buf[0] & 0x1)
		u = ~u;

	switch (bits) {
		case 32:
			*(uint32_t*)p = u;
			break;
		case 64:
			*(uint64_t*)p = u;
			break;
		default:
			scf_loge("bits %d Not support!\n", bits);
			return -EINVAL;
			break;
	};

	scf_logd("u: %#lx, j: %d, bits: %d\n", u, j, bits);
	return j;
}

int __scf_unpack(void* p, int size, const uint8_t* buf, int len)
{
	switch (size) {
		case 1:
			if (1 <= len) {
				*(uint8_t*)p = buf[0];
				return 1;
			}
			break;

		case 2:
			if (2 <= len) {
				*(uint16_t*)p = *(uint16_t*)buf;
				return 2;
			}
			break;

		case 4:
#if 0
			if (4 <= len) {
				*(uint32_t*)p = *(uint32_t*)buf;
				return 4;
			}
#else
			return __scf_unpack2(p, 5, buf, len);
#endif
			break;

		case 8:
#if 0
			if (8 <= len) {
				*(uint64_t*)p = *(uint64_t*)buf;
				return 8;
			}
#else
			return __scf_unpack2(p, 6, buf, len);
#endif
			break;
		default:
			scf_loge("data type NOT support!\n");
			break;
	};

	return -EINVAL;
}

int __scf_pack(void* p, int size, uint8_t** pbuf, int* plen)
{
	uint8_t pack[64];
	int     len = 0;

	switch (size) {
		case 1:
			pack[0] = *(uint8_t*)p;
			len = 1;
			break;
		case 2:
			*(uint16_t*)pack = *(uint16_t*)p;
			len = 2;
			break;
		case 4:
#if 1
			len = __scf_pack2(pack, *(uint32_t*)p, 5);
			if (len < 0)
				return len;
#else
			*(uint32_t*)pack = *(uint32_t*)p;
			len = 4;
#endif
			scf_logd("p: %p, %d, len: %d\n\n", p, *(uint32_t*)p, len);
			break;
		case 8:
#if 1
			len = __scf_pack2(pack, *(uint64_t*)p, 6);
			if (len < 0)
				return len;
#else
			*(uint64_t*)pack = *(uint64_t*)p;
			len = 8;
#endif
			scf_logd("p: %p, %ld, %#lx, %lg, len: %d\n\n", p, *(uint64_t*)p, *(uint64_t*)p, *(double*)p, len);
			break;
		default:
			scf_loge("data size '%d' NOT support!\n", size);
			return -EINVAL;
			break;
	};

	void* b = realloc(*pbuf, *plen + len);
	if (!b)
		return -ENOMEM;
	*pbuf = b;

	memcpy(*pbuf + *plen, pack, len);
	*plen += len;
	return 0;
}

int scf_pack(void* p, scf_pack_info_t* infos, int n_infos, uint8_t** pbuf, int* plen)
{
	if (!p || !infos || n_infos < 1 || !pbuf || !plen)
		return -EINVAL;

	if (!*pbuf)
		*plen = 0;

//	printf("\n");
	scf_logd("p: %p\n", p);

	int i;
	for (i = 0; i < n_infos; i++) {
		scf_logd("name: %s, size: %ld, offset: %ld, noffset: %ld, msize: %ld, members: %p, n_members: %ld\n",
				infos[i].name, infos[i].size, infos[i].offset, infos[i].noffset, infos[i].msize, infos[i].members, infos[i].n_members);

		if (infos[i].noffset >= 0) {

			void* a = *(void**)(p + infos[i].offset);
			long  n = *(long* )(p + infos[i].noffset);
			int   j;

			scf_logd("a: %p, n: %ld, infos[i].msize: %ld, infos[i].noffset: %ld\n", a, n, infos[i].msize, infos[i].noffset);

			for (j = 0; j < n; j++) {

				if (infos[i].members) {
					int ret = scf_pack(*(void**)(a + j * infos[i].msize), infos[i].members, infos[i].n_members, pbuf, plen);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}
				} else {
					int ret = __scf_pack(a + j * infos[i].msize, infos[i].msize, pbuf, plen);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}
				}
			}

			continue;
		}

		if (infos[i].members) {

			int ret = scf_pack(*(void**)(p + infos[i].offset), infos[i].members, infos[i].n_members, pbuf, plen);
			if (ret < 0) {
				scf_loge("ret: %d\n", ret);
				return ret;
			}
			continue;
		}

		int ret = __scf_pack(p + infos[i].offset, infos[i].size, pbuf, plen);
		if (ret < 0) {
			scf_loge("ret: %d\n", ret);
			return ret;
		}

		scf_logd("size: %ld\n\n", infos[i].size);
	}

	return 0;
}

int scf_unpack(void** pp, scf_pack_info_t* infos, int n_infos, const uint8_t* buf, int len)
{
	if (!pp || !infos || n_infos < 1 || !buf || len < 1)
		return -EINVAL;

	int size = infos[n_infos - 1].offset + infos[n_infos - 1].size;

	void* p  = calloc(1, size);
	if (!p)
		return -ENOMEM;

	int i;
	int j;
	int k = 0;

	for (i = 0; i < n_infos; i++) {

		if (infos[i].noffset >= 0) {

			long  n = *(long*)(p + infos[i].noffset);
			void* a = calloc(n, infos[i].msize);
			if (!a)
				return -ENOMEM;
			*(void**)(p + infos[i].offset) = a;

			scf_logd("a: %p, n: %ld, infos[i].msize: %ld\n", a, n, infos[i].msize);

			for (j = 0; j < n; j++) {

				if (infos[i].members) {
					int ret = scf_unpack((void**)(a + j * infos[i].msize), infos[i].members, infos[i].n_members, buf + k, len - k);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}

					k += ret;

				} else {
					int ret = __scf_unpack(a + j * infos[i].msize, infos[i].msize, buf + k, len - k);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}

					k += ret;
				}
			}

			continue;
		}

		if (infos[i].members) {

			int ret = scf_unpack((void**)(p + infos[i].offset), infos[i].members, infos[i].n_members, buf + k, len - k);
			if (ret < 0) {
				scf_loge("ret: %d\n", ret);
				return ret;
			}

			k += ret;
			continue;
		}

		int ret = __scf_unpack(p + infos[i].offset, infos[i].size, buf + k, len - k);
		if (ret < 0) {
			scf_loge("ret: %d\n", ret);
			return ret;
		}

		k += ret;
	}

	*pp = p;
	return k;
}

int scf_unpack_free(void* p, scf_pack_info_t* infos, int n_infos)
{
	if (!p || !infos || n_infos < 1)
		return -EINVAL;

	int i;
	int j;
	for (i = 0; i < n_infos; i++) {

		if (infos[i].noffset >= 0) {
			long  n = *(long* )(p + infos[i].noffset);
			void* a = *(void**)(p + infos[i].offset);

			scf_logd("a: %p, n: %ld, infos[i].msize: %ld\n", a, n, infos[i].msize);

			if (infos[i].members) {

				for (j = 0; j < n; j++) {
					int ret = scf_unpack_free(*(void**)(a + j * infos[i].msize), infos[i].members, infos[i].n_members);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}
				}
			}

			scf_logd("a: %p\n", a);
			free(a);
			continue;
		}

		if (infos[i].members) {
			int ret = scf_unpack_free(*(void**)(p + infos[i].offset), infos[i].members, infos[i].n_members);
			if (ret < 0) {
				scf_loge("ret: %d\n", ret);
				return ret;
			}
		}
	}

	scf_logd("p: %p\n", p);
	free(p);
	return 0;
}
