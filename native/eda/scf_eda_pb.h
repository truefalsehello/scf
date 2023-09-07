#ifndef SCF_EDA_PB_H
#define SCF_EDA_PB_H

#include "scf_eda.pb-c.h"

enum {
	SCF_EDA_None,
	SCF_EDA_Battery,

	SCF_EDA_Resistor,
	SCF_EDA_Capacitor,
	SCF_EDA_Inductor,

	SCF_EDA_Diode,
	SCF_EDA_Transistor,

	SCF_EDA_Components_NB,
};

#define SCF_EDA_PIN_NONE  0
#define SCF_EDA_PIN_IN    1
#define SCF_EDA_PIN_OUT   2
#define SCF_EDA_PIN_POS   4
#define SCF_EDA_PIN_NEG   8

#define SCF_EDA_V_INIT   -10001001.0
#define SCF_EDA_V_MIN    -10000000.0
#define SCF_EDA_V_MAX     10000000.0

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
	SCF_EDA_Transistor_B,
	SCF_EDA_Transistor_E,
	SCF_EDA_Transistor_C,
	SCF_EDA_Transistor_NB,
};

typedef struct {
	uint64_t  type;
	uint64_t  model;
	uint64_t  pid;

	double    v;
	double    a;
	double    r;
	double    jr;
	double    uf;
	double    uh;
	double    hfe;
} scf_edata_t;


ScfEconn*      scf_econn__alloc();
int            scf_econn__add_cid(ScfEconn* ec, uint64_t  cid);
int            scf_econn__del_cid(ScfEconn* ec, uint64_t  cid);
void           scf_econn__free(ScfEconn* ec);

ScfEline*      scf_eline__alloc();
int            scf_eline__add_line(ScfEline* el, ScfLine*  l);
int            scf_eline__del_line(ScfEline* el, ScfLine*  l);

int            scf_eline__add_pin (ScfEline* el, uint64_t  cid, uint64_t pid);
int            scf_eline__del_pin (ScfEline* el, uint64_t  cid, uint64_t pid);

int            scf_eline__add_conn(ScfEline* el, ScfEconn* ec);
int            scf_eline__del_conn(ScfEline* el, ScfEconn* ec);
void           scf_eline__free    (ScfEline* el);

ScfEpin*       scf_epin__alloc();
int            scf_epin__add_component(ScfEpin* pin, uint64_t cid, uint64_t pid);
int            scf_epin__del_component(ScfEpin* pin, uint64_t cid, uint64_t pid);
void           scf_epin__free         (ScfEpin* pin);

ScfEcomponent* scf_ecomponent__alloc  (uint64_t type);
int            scf_ecomponent__add_pin(ScfEcomponent* c, ScfEpin* pin);
int            scf_ecomponent__del_pin(ScfEcomponent* c, ScfEpin* pin);
void           scf_ecomponent__free   (ScfEcomponent* c);

ScfEfunction*  scf_efunction__alloc        (const   char* name);
int            scf_efunction__add_component(ScfEfunction* f, ScfEcomponent* c);
int            scf_efunction__del_component(ScfEfunction* f, ScfEcomponent* c);

int            scf_efunction__add_eline    (ScfEfunction* f, ScfEline* el);
int            scf_efunction__del_eline    (ScfEfunction* f, ScfEline* el);
void           scf_efunction__free         (ScfEfunction* f);

ScfEboard*     scf_eboard__alloc();
int            scf_eboard__add_function(ScfEboard* b, ScfEfunction* f);
int            scf_eboard__del_function(ScfEboard* b, ScfEfunction* f);
void           scf_eboard__free        (ScfEboard* b);

#endif
