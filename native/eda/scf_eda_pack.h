#ifndef SCF_EDA_PACK_H
#define SCF_EDA_PACK_H

#include "scf_pack.h"

enum {
	SCF_EDA_None,
	SCF_EDA_Battery,

	SCF_EDA_Resistor,
	SCF_EDA_Capacitor,
	SCF_EDA_Inductor,

	SCF_EDA_Diode,
	SCF_EDA_NPN,
	SCF_EDA_PNP,

	SCF_EDA_NAND,
	SCF_EDA_NOR,
	SCF_EDA_NOT,

	SCF_EDA_Components_NB,
};

#define SCF_EDA_PIN_NONE   0
#define SCF_EDA_PIN_IN     1
#define SCF_EDA_PIN_OUT    2
#define SCF_EDA_PIN_POS    4
#define SCF_EDA_PIN_NEG    8
#define SCF_EDA_PIN_CF     16
#define SCF_EDA_PIN_BORDER 32
#define SCF_EDA_PIN_CONST  64
#define SCF_EDA_PIN_IN0   128

#define SCF_EDA_V_INIT   -10001001.0
#define SCF_EDA_V_MIN    -10000000.0
#define SCF_EDA_V_MAX     10000000.0

#define SCF_EDA_V_Diode_ON  0.58
#define SCF_EDA_V_Diode_OFF 0.55

#define SCF_EDA_V_NPN_ON    0.70
#define SCF_EDA_V_NPN_OFF   0.61

#define SCF_EDA_V_PNP_ON    SCF_EDA_V_NPN_ON
#define SCF_EDA_V_PNP_OFF   SCF_EDA_V_NPN_OFF

enum {
	SCF_EDA_Battery_NEG,
	SCF_EDA_Battery_POS,
	SCF_EDA_Battery_NB,
};

enum {
	SCF_EDA_Diode_NEG,
	SCF_EDA_Diode_POS,
	SCF_EDA_Diode_NB,
};

enum {
	SCF_EDA_Status_ON,
	SCF_EDA_Status_OFF,
	SCF_EDA_Path_OFF,
	SCF_EDA_Path_TO,
};

enum {
	SCF_EDA_NPN_B,
	SCF_EDA_NPN_E,
	SCF_EDA_NPN_C,
	SCF_EDA_NPN_NB,
};

enum {
	SCF_EDA_PNP_B  = SCF_EDA_NPN_B,
	SCF_EDA_PNP_E  = SCF_EDA_NPN_E,
	SCF_EDA_PNP_C  = SCF_EDA_NPN_C,
	SCF_EDA_PNP_NB = SCF_EDA_NPN_NB,
};

enum {
	SCF_EDA_NAND_NEG,
	SCF_EDA_NAND_POS,

	SCF_EDA_NAND_IN0,
	SCF_EDA_NAND_IN1,
	SCF_EDA_NAND_OUT,

	SCF_EDA_NAND_NB,
};

enum {
	SCF_EDA_NOR_NEG,
	SCF_EDA_NOR_POS,

	SCF_EDA_NOR_IN0,
	SCF_EDA_NOR_IN1,
	SCF_EDA_NOR_OUT,

	SCF_EDA_NOR_NB,
};

enum {
	SCF_EDA_NOT_NEG,
	SCF_EDA_NOT_POS,

	SCF_EDA_NOT_IN,
	SCF_EDA_NOT_OUT,

	SCF_EDA_NOT_NB,
};

typedef struct {
	uint64_t  type;
	uint64_t  model;
	uint64_t  pid;

	double    v;
	double    a;
	double    r;

	double    uf;
	double    uh;
	double    hfe;

	void*     ops;
	char*     cpk;
} ScfEdata;

typedef struct {
	SCF_PACK_DEF_VAR(int, x0);
	SCF_PACK_DEF_VAR(int, y0);
	SCF_PACK_DEF_VAR(int, x1);
	SCF_PACK_DEF_VAR(int, y1);
} ScfLine;

SCF_PACK_TYPE(ScfLine)
SCF_PACK_INFO_VAR(ScfLine, x0),
SCF_PACK_INFO_VAR(ScfLine, y0),
SCF_PACK_INFO_VAR(ScfLine, x1),
SCF_PACK_INFO_VAR(ScfLine, y1),
SCF_PACK_END(ScfLine)

typedef struct scf_eops_s        ScfEops;
typedef struct scf_epin_s        ScfEpin;
typedef struct scf_ecomponent_s  ScfEcomponent;
typedef struct scf_efunction_s   ScfEfunction;
typedef struct scf_eboard_s      ScfEboard;

struct scf_eops_s
{
	int (*off   )(ScfEpin* p0, ScfEpin* p1);
	int (*shared)(ScfEpin* p);
};

struct scf_epin_s
{
	SCF_PACK_DEF_VAR(uint64_t, id);
	SCF_PACK_DEF_VAR(uint64_t, cid);
	SCF_PACK_DEF_VAR(uint64_t, lid);
	SCF_PACK_DEF_VAR(uint64_t, flags);
	SCF_PACK_DEF_VARS(uint64_t, tos);
	SCF_PACK_DEF_VAR(uint64_t, c_lid);

	SCF_PACK_DEF_OBJ(ScfEcomponent, IC);

	SCF_PACK_DEF_VAR(double, v);
	SCF_PACK_DEF_VAR(double, a);

	SCF_PACK_DEF_VAR(double, r);
	SCF_PACK_DEF_VAR(double, uf);
	SCF_PACK_DEF_VAR(double, uh);
	SCF_PACK_DEF_VAR(double, hfe);

	SCF_PACK_DEF_VAR(double, dr);

	SCF_PACK_DEF_VAR(double, sr);
	SCF_PACK_DEF_VAR(double, pr);

	SCF_PACK_DEF_VAR(uint64_t, path);

	SCF_PACK_DEF_VAR(int, x);
	SCF_PACK_DEF_VAR(int, y);

	SCF_PACK_DEF_VAR(int, n_diodes);
	SCF_PACK_DEF_VAR(int, l_pos);

	SCF_PACK_DEF_VAR(uint8_t, vflag);
	SCF_PACK_DEF_VAR(uint8_t, pflag);
	SCF_PACK_DEF_VAR(uint8_t, vconst);
	SCF_PACK_DEF_VAR(uint8_t, aconst);
};

SCF_PACK_TYPE(ScfEpin)
SCF_PACK_INFO_VAR(ScfEpin, id),
SCF_PACK_INFO_VAR(ScfEpin, cid),
SCF_PACK_INFO_VAR(ScfEpin, lid),
SCF_PACK_INFO_VAR(ScfEpin, flags),
SCF_PACK_INFO_VARS(ScfEpin, tos, uint64_t),
SCF_PACK_INFO_VAR(ScfEpin, c_lid),

SCF_PACK_INFO_VAR(ScfEpin, v),
SCF_PACK_INFO_VAR(ScfEpin, a),

SCF_PACK_INFO_VAR(ScfEpin, r),
SCF_PACK_INFO_VAR(ScfEpin, uf),
SCF_PACK_INFO_VAR(ScfEpin, uh),
SCF_PACK_INFO_VAR(ScfEpin, hfe),

SCF_PACK_INFO_VAR(ScfEpin, dr),
SCF_PACK_INFO_VAR(ScfEpin, sr),
SCF_PACK_INFO_VAR(ScfEpin, pr),

SCF_PACK_INFO_VAR(ScfEpin, path),
SCF_PACK_INFO_VAR(ScfEpin, x),
SCF_PACK_INFO_VAR(ScfEpin, y),
SCF_PACK_INFO_VAR(ScfEpin, n_diodes),
SCF_PACK_INFO_VAR(ScfEpin, l_pos),

SCF_PACK_INFO_VAR(ScfEpin, vflag),
SCF_PACK_INFO_VAR(ScfEpin, pflag),
SCF_PACK_INFO_VAR(ScfEpin, vconst),
SCF_PACK_INFO_VAR(ScfEpin, aconst),
SCF_PACK_END(ScfEpin)

typedef struct {
	SCF_PACK_DEF_VAR(uint64_t, lid);
	SCF_PACK_DEF_VARS(uint64_t, cids);
} ScfEconn;

SCF_PACK_TYPE(ScfEconn)
SCF_PACK_INFO_VAR(ScfEconn, lid),
SCF_PACK_INFO_VARS(ScfEconn, cids, uint64_t),
SCF_PACK_END(ScfEconn)

typedef struct {
	SCF_PACK_DEF_VAR(uint64_t, id);
	SCF_PACK_DEF_VARS(uint64_t, pins);
	SCF_PACK_DEF_VAR(uint64_t, c_pins);
	SCF_PACK_DEF_VAR(uint64_t, flags);
	SCF_PACK_DEF_VAR(int64_t, color);

	SCF_PACK_DEF_OBJS(ScfEconn, conns);
	SCF_PACK_DEF_OBJS(ScfLine, lines);

	SCF_PACK_DEF_VAR(double, v);
	SCF_PACK_DEF_VAR(double, ain);
	SCF_PACK_DEF_VAR(double, aout);
	SCF_PACK_DEF_VAR(uint8_t, vconst);
	SCF_PACK_DEF_VAR(uint8_t, aconst);
	SCF_PACK_DEF_VAR(uint8_t, vflag);
	SCF_PACK_DEF_VAR(uint8_t, vinit);
} ScfEline;

SCF_PACK_TYPE(ScfEline)
SCF_PACK_INFO_VAR(ScfEline, id),
SCF_PACK_INFO_VARS(ScfEline, pins, uint64_t),
SCF_PACK_INFO_VAR(ScfEline, c_pins),
SCF_PACK_INFO_VAR(ScfEline, flags),
SCF_PACK_INFO_VAR(ScfEline, color),

SCF_PACK_INFO_OBJS(ScfEline, conns, ScfEconn),
SCF_PACK_INFO_OBJS(ScfEline, lines, ScfLine),

SCF_PACK_INFO_VAR(ScfEline, v),
SCF_PACK_INFO_VAR(ScfEline, ain),
SCF_PACK_INFO_VAR(ScfEline, aout),
SCF_PACK_INFO_VAR(ScfEline, vconst),
SCF_PACK_INFO_VAR(ScfEline, aconst),
SCF_PACK_INFO_VAR(ScfEline, vflag),
SCF_PACK_END(ScfEline)

struct scf_ecomponent_s
{
	SCF_PACK_DEF_VAR(uint64_t, id);
	SCF_PACK_DEF_VAR(uint64_t, type);
	SCF_PACK_DEF_VAR(uint64_t, model);
	SCF_PACK_DEF_OBJS(ScfEpin, pins);

	SCF_PACK_DEF_VARS(uint8_t, cpk);
	SCF_PACK_DEF_OBJ(ScfEfunction, f);
	SCF_PACK_DEF_OBJ(ScfEops,      ops);

	SCF_PACK_DEF_VAR(double, v);
	SCF_PACK_DEF_VAR(double, a);

	SCF_PACK_DEF_VAR(double, dr);

	SCF_PACK_DEF_VAR(double, r);
	SCF_PACK_DEF_VAR(double, uf);
	SCF_PACK_DEF_VAR(double, uh);


	SCF_PACK_DEF_VAR(int64_t, count);
	SCF_PACK_DEF_VAR(int64_t, color);
	SCF_PACK_DEF_VAR(int, status);
	SCF_PACK_DEF_VAR(int, x);
	SCF_PACK_DEF_VAR(int, y);
	SCF_PACK_DEF_VAR(int, w);
	SCF_PACK_DEF_VAR(int, h);
	SCF_PACK_DEF_VAR(uint8_t, vflag);
	SCF_PACK_DEF_VAR(uint8_t, lock);
};

SCF_PACK_TYPE(ScfEcomponent)
SCF_PACK_INFO_VAR(ScfEcomponent, id),
SCF_PACK_INFO_VAR(ScfEcomponent, type),
SCF_PACK_INFO_VAR(ScfEcomponent, model),
SCF_PACK_INFO_OBJS(ScfEcomponent, pins, ScfEpin),

SCF_PACK_INFO_VARS(ScfEcomponent, cpk, uint8_t),

SCF_PACK_INFO_VAR(ScfEcomponent, v),
SCF_PACK_INFO_VAR(ScfEcomponent, a),

SCF_PACK_INFO_VAR(ScfEcomponent, dr),

SCF_PACK_INFO_VAR(ScfEcomponent, r),
SCF_PACK_INFO_VAR(ScfEcomponent, uf),
SCF_PACK_INFO_VAR(ScfEcomponent, uh),

SCF_PACK_INFO_VAR(ScfEcomponent, count),
SCF_PACK_INFO_VAR(ScfEcomponent, color),
SCF_PACK_INFO_VAR(ScfEcomponent, status),
SCF_PACK_INFO_VAR(ScfEcomponent, x),
SCF_PACK_INFO_VAR(ScfEcomponent, y),
SCF_PACK_INFO_VAR(ScfEcomponent, w),
SCF_PACK_INFO_VAR(ScfEcomponent, h),
SCF_PACK_INFO_VAR(ScfEcomponent, vflag),
SCF_PACK_INFO_VAR(ScfEcomponent, lock),
SCF_PACK_END(ScfEcomponent)

struct scf_efunction_s
{
	SCF_PACK_DEF_VARS(uint8_t, name);
	SCF_PACK_DEF_OBJS(ScfEcomponent, components);
	SCF_PACK_DEF_OBJS(ScfEline,      elines);
	SCF_PACK_DEF_VAR(int, x);
	SCF_PACK_DEF_VAR(int, y);
	SCF_PACK_DEF_VAR(int, w);
	SCF_PACK_DEF_VAR(int, h);
};

SCF_PACK_TYPE(ScfEfunction)
SCF_PACK_INFO_VARS(ScfEfunction, name,       uint8_t),
SCF_PACK_INFO_OBJS(ScfEfunction, components, ScfEcomponent),
SCF_PACK_INFO_OBJS(ScfEfunction, elines,     ScfEline),
SCF_PACK_INFO_VAR(ScfEfunction,  x),
SCF_PACK_INFO_VAR(ScfEfunction,  y),
SCF_PACK_INFO_VAR(ScfEfunction,  w),
SCF_PACK_INFO_VAR(ScfEfunction,  h),
SCF_PACK_END(ScfEfunction)

struct scf_eboard_s
{
	SCF_PACK_DEF_OBJS(ScfEfunction, functions);
};

SCF_PACK_TYPE(ScfEboard)
SCF_PACK_INFO_OBJS(ScfEboard, functions, ScfEfunction),
SCF_PACK_END(ScfEboard)


ScfEconn*      scf_econn__alloc();
int            scf_econn__add_cid(ScfEconn* ec, uint64_t  cid);
int            scf_econn__del_cid(ScfEconn* ec, uint64_t  cid);

ScfEline*      scf_eline__alloc();
int            scf_eline__add_line(ScfEline* el, ScfLine*  l);
int            scf_eline__del_line(ScfEline* el, ScfLine*  l);

int            scf_eline__add_pin (ScfEline* el, uint64_t  cid, uint64_t pid);
int            scf_eline__del_pin (ScfEline* el, uint64_t  cid, uint64_t pid);

int            scf_eline__add_conn(ScfEline* el, ScfEconn* ec);
int            scf_eline__del_conn(ScfEline* el, ScfEconn* ec);

ScfEpin*       scf_epin__alloc();
int            scf_epin__add_component(ScfEpin* pin, uint64_t cid, uint64_t pid);
int            scf_epin__del_component(ScfEpin* pin, uint64_t cid, uint64_t pid);

ScfEcomponent* scf_ecomponent__alloc  (uint64_t type);
int            scf_ecomponent__add_pin(ScfEcomponent* c, ScfEpin* pin);
int            scf_ecomponent__del_pin(ScfEcomponent* c, ScfEpin* pin);
ScfEdata*      scf_ecomponent__find_data(const uint64_t type, const uint64_t model);

ScfEfunction*  scf_efunction__alloc        (const   char* name);
int            scf_efunction__add_component(ScfEfunction* f, ScfEcomponent* c);
int            scf_efunction__del_component(ScfEfunction* f, ScfEcomponent* c);
int            scf_efunction__add_eline    (ScfEfunction* f, ScfEline* el);
int            scf_efunction__del_eline    (ScfEfunction* f, ScfEline* el);

ScfEboard*     scf_eboard__alloc();
int            scf_eboard__add_function(ScfEboard* b, ScfEfunction* f);
int            scf_eboard__del_function(ScfEboard* b, ScfEfunction* f);

#define EDA_INST_ADD_COMPONENT(_ef, _c, _type) \
	do { \
		_c = scf_ecomponent__alloc(_type); \
		if (!_c) \
			return -ENOMEM; \
		\
		(_c)->id = (_ef)->n_components; \
		\
		int ret = scf_efunction__add_component(_ef, _c); \
		if (ret < 0) { \
			ScfEcomponent_free(_c); \
			_c = NULL; \
			return ret; \
		} \
		\
		for (size_t i = 0;  i < (_c)->n_pins; i++) \
			(_c)->pins[i]->cid = (_c)->id; \
	} while (0)

#define EDA_PIN_ADD_COMPONENT(_pin, _cid, _pid) \
	do { \
		int ret = scf_epin__add_component((_pin), _cid, _pid); \
		if (ret < 0) \
			return ret; \
	} while (0)

#define EDA_PIN_ADD_PIN(_c0, _pid0, _c1, _pid1) \
	do { \
		int ret = scf_epin__add_component((_c0)->pins[_pid0], (_c1)->id, (_pid1)); \
		if (ret < 0) \
			return ret; \
		\
		ret = scf_epin__add_component((_c1)->pins[_pid1], (_c0)->id, (_pid0)); \
		if (ret < 0) \
			return ret; \
	} while (0)

#define EDA_PIN_ADD_PIN_EF(_ef, _p0, _p1) \
	EDA_PIN_ADD_PIN((_ef)->components[(_p0)->cid], (_p0)->id, (_ef)->components[(_p1)->cid], (_p1)->id)

#endif
