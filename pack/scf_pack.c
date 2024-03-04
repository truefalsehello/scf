#include"scf_pack.h"

int __scf_pack(void* p, int size, uint8_t** pbuf, int* plen)
{
	uint8_t pack[8];
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
			scf_logi("p: %p, %d\n", p, *(uint32_t*)p);
			*(uint32_t*)pack = *(uint32_t*)p;
			len = 4;
			break;
		case 8:
			scf_logi("p: %p, %ld, %#lx, %lg\n", p, *(uint64_t*)p, *(uint64_t*)p, *(double*)p);
			*(uint64_t*)pack = *(uint64_t*)p;
			len = 8;
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

	printf("\n");
	scf_logw("p: %p\n", p);

	int i;
	for (i = 0; i < n_infos; i++) {
		printf("name: %s, size: %ld, offset: %ld, noffset: %ld, msize: %ld, members: %p, n_members: %ld\n",
				infos[i].name, infos[i].size, infos[i].offset, infos[i].noffset, infos[i].msize, infos[i].members, infos[i].n_members);

		if (infos[i].noffset >= 0) {

			void* a = *(void**)(p + infos[i].offset);
			long  n = *(long* )(p + infos[i].noffset);
			int   j;

			scf_loge("a: %p, n: %ld, infos[i].msize: %ld, infos[i].noffset: %ld\n", a, n, infos[i].msize, infos[i].noffset);

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

		scf_loge("size: %ld\n\n", infos[i].size);
	}

	return 0;
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
			if (4 <= len) {
				*(uint32_t*)p = *(uint32_t*)buf;
				return 4;
			}
			break;

		case 8:
			if (8 <= len) {
				*(uint64_t*)p = *(uint64_t*)buf;
				return 8;
			}
			break;
		default:
			scf_loge("data type NOT support!\n");
			break;
	};

	return -EINVAL;
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

			scf_loge("a: %p, n: %ld, infos[i].msize: %ld\n", a, n, infos[i].msize);

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

			scf_loge("a: %p, n: %ld, infos[i].msize: %ld\n", a, n, infos[i].msize);

			if (infos[i].members) {

				for (j = 0; j < n; j++) {
					int ret = scf_unpack_free(*(void**)(a + j * infos[i].msize), infos[i].members, infos[i].n_members);
					if (ret < 0) {
						scf_loge("ret: %d\n", ret);
						return ret;
					}
				}
			}

			scf_logi("a: %p\n", a);
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

	scf_logi("p: %p\n", p);
	free(p);
	return 0;
}
