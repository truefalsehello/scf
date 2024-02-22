#include"scf_pack.h"

int scf_pack(void* p, scf_pack_info_t* infos, int n_infos, uint8_t** pbuf, int* plen)
{
	if (!p || !infos || n_infos < 1 || !pbuf || !plen)
		return -EINVAL;

	uint8_t* buf = *pbuf;
	int      len = *plen;
	int      i;

	if (!buf)
		len = 0;

	for (i = 0; i < n_infos; i++) {
		printf("name: %s, size: %ld, offset: %ld, noffset: %ld, members: %p, n_members: %ld\n",
				infos[i].name, infos[i].size, infos[i].offset, infos[i].noffset, infos[i].members, infos[i].n_members);

		if (!infos[i].members) {

			uint8_t pack[8];
			int     size = 0;

			switch (infos[i].size) {
				case 1:
					pack[0] = *(uint8_t*)(p + infos[i].offset);
					size = 1;
					break;
				case 2:
					*(uint16_t*)pack = *(uint16_t*)(p + infos[i].offset);
					size = 2;
					break;
				case 4:
					*(uint32_t*)pack = *(uint32_t*)(p + infos[i].offset);
					size = 4;
					break;
				case 8:
					*(uint64_t*)pack = *(uint64_t*)(p + infos[i].offset);
					size = 8;
					break;
				default:
					scf_loge("data type NOT support!\n");
					return -EINVAL;
					break;
			};

			void* b = realloc(buf, len + size);
			if (!b)
				return -ENOMEM;
			buf = b;

			memcpy(buf + len, pack, size);
			len += size;
		}
	}

	*pbuf = buf;
	*plen = len;
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
	int j = 0;

	for (i = 0; i < n_infos; i++) {

		if (!infos[i].members) {

			switch (infos[i].size) {
				case 1:
					if (j + 1 <= len)
						*(uint8_t*)(p + infos[i].offset) = buf[j++];
					else {
						free(p);
						return -EINVAL;
					}
					break;

				case 2:
					if (j + 2 <= len) {
						*(uint16_t*)(p + infos[i].offset) = *(uint16_t*)(buf + j);
						j += 2;
					} else {
						free(p);
						return -EINVAL;
					}
					break;

				case 4:
					if (j + 4 <= len) {
						*(uint32_t*)(p + infos[i].offset) = *(uint32_t*)(buf + j);
						j += 4;
					} else {
						free(p);
						return -EINVAL;
					}
					break;

				case 8:
					if (j + 8 <= len) {
						*(uint64_t*)(p + infos[i].offset) = *(uint64_t*)(buf + j);
						j += 8;
					} else {
						free(p);
						return -EINVAL;
					}
					break;
				default:
					scf_loge("data type NOT support!\n");
					return -EINVAL;
					break;
			};
		}
	}

	*pp = p;
	return 0;
}

int scf_unpack_free(void* p, scf_pack_info_t* infos, int n_infos)
{
	if (!p || !infos || n_infos < 1)
		return -EINVAL;

	int i;
	for (i = 0; i < n_infos; i++) {

		if (infos[i].members) {
			int ret = scf_unpack_free(p + infos[i].offset, infos[i].members, infos[i].n_members);
		}
	}

	free(p);
	return 0;
}

