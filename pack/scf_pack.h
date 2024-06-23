#ifndef SCF_PACK_H
#define SCF_PACK_H

#include"scf_def.h"

typedef struct scf_pack_info_s  scf_pack_info_t;

struct scf_pack_info_s
{
	const char*      name;
	long             size;
	long             offset;
	long             noffset;
	long             msize;
	scf_pack_info_t* members;
	long             n_members;
};

long scf_pack       (void*  p,  scf_pack_info_t* infos, long n_infos,       uint8_t** pbuf, long* plen);
long scf_unpack     (void** pp, scf_pack_info_t* infos, long n_infos, const uint8_t*  buf,  long  len);
long scf_unpack_free(void*  p,  scf_pack_info_t* infos, long n_infos);

#define SCF_PACK_DEF_VAR(type, var)    type  var
#define SCF_PACK_DEF_VARS(type, vars)  long  n_##vars; type* vars

#define SCF_PACK_DEF_OBJ(type, obj)    type* obj
#define SCF_PACK_DEF_OBJS(type, objs)  long  n_##objs; type** objs

#define SCF_PACK_N_INFOS(type)         (sizeof(scf_pack_info_##type) / sizeof(scf_pack_info_##type[0]))

#define SCF_PACK_INFO_VAR(type, var)            {#var,  sizeof(((type*)0)->var),   offsetof(type, var), -1, -1, NULL, 0}
#define SCF_PACK_INFO_OBJ(type, obj, objtype)   {#obj,  sizeof(((type*)0)->obj),   offsetof(type, obj), -1, -1, scf_pack_info_##objtype, SCF_PACK_N_INFOS(objtype)}

#define SCF_PACK_INFO_VARS(type, vars, vtype) \
	{"n_"#vars, sizeof(((type*)0)->n_##vars), offsetof(type, n_##vars), -1, -1, NULL, 0}, \
	{#vars,     sizeof(((type*)0)->vars),     offsetof(type, vars),     offsetof(type, n_##vars), sizeof(vtype), NULL, 0}

#define SCF_PACK_INFO_OBJS(type, objs, objtype) \
	{"n_"#objs, sizeof(((type*)0)->n_##objs), offsetof(type, n_##objs), -1, -1, NULL, 0}, \
	{#objs,     sizeof(((type*)0)->objs),     offsetof(type, objs),     offsetof(type, n_##objs), sizeof(objtype*), scf_pack_info_##objtype, SCF_PACK_N_INFOS(objtype)}

#define SCF_PACK_TYPE(type) \
static scf_pack_info_t scf_pack_info_##type[] = {


#define SCF_PACK_END(type) \
}; \
static long type##_pack(type* p, uint8_t** pbuf, long* plen) \
{ \
	return scf_pack(p, scf_pack_info_##type, SCF_PACK_N_INFOS(type), pbuf, plen); \
} \
static long type##_unpack(type** pp, uint8_t* buf, long len) \
{ \
	return scf_unpack((void**)pp, scf_pack_info_##type, SCF_PACK_N_INFOS(type), buf, len); \
} \
static long type##_free(type* p) \
{ \
	return scf_unpack_free(p, scf_pack_info_##type, SCF_PACK_N_INFOS(type)); \
}

long scf_pack_read(uint8_t** pbuf, const char* cpk);

#endif
