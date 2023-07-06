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
	SCF_EDA_Transistor_B,
	SCF_EDA_Transistor_C,
	SCF_EDA_Transistor_E,
	SCF_EDA_Transistor_NB,
};

ScfEline*      scf_eline__alloc();
void           scf_eline__free (ScfEline* l);

ScfEpin*       scf_epin__alloc();
int            scf_epin__add_component(ScfEpin* pin, uint64_t cid, uint64_t pid);
int            scf_epin__del_component(ScfEpin* pin, uint64_t cid, uint64_t pid);
int            scf_epin__add_line     (ScfEpin* pin, ScfEline* l);
int            scf_epin__del_line     (ScfEpin* pin, ScfEline* l);
void           scf_epin__free         (ScfEpin* pin);

ScfEcomponent* scf_ecomponent__alloc(uint64_t type);
int            scf_ecomponent__add_pin(ScfEcomponent* c, ScfEpin* pin);
int            scf_ecomponent__del_pin(ScfEcomponent* c, ScfEpin* pin);
void           scf_ecomponent__free   (ScfEcomponent* c);

ScfEfunction*  scf_efunction__alloc        (const   char* name);
int            scf_efunction__add_component(ScfEfunction* f, ScfEcomponent* c);
int            scf_efunction__del_component(ScfEfunction* f, ScfEcomponent* c);
void           scf_efunction__free         (ScfEfunction* f);

ScfEboard*     scf_eboard__alloc();
int            scf_eboard__add_function(ScfEboard* b, ScfEfunction* f);
int            scf_eboard__del_function(ScfEboard* b, ScfEfunction* f);
void           scf_eboard__free        (ScfEboard* b);

#endif
