#include "scf_eda_pack.h"
//#include "scf_def.h"

static int component_pins[SCF_EDA_Components_NB] =
{
	0, // None
	SCF_EDA_Battery_NB, // SCF_EDA_Battery

	2, // SCF_EDA_Resistor
	2, // SCF_EDA_Capacitor
	2, // SCF_EDA_Inductor

	SCF_EDA_Diode_NB,
	SCF_EDA_NPN_NB,
	SCF_EDA_PNP_NB,
};

static scf_edata_t  component_datas[] =
{
	{SCF_EDA_None,       0,                   0, 0, 0,    0,   0,   0, 0},
	{SCF_EDA_Battery,    0, SCF_EDA_Battery_POS, 0, 0,    0,   0,   0, 0},

	{SCF_EDA_Resistor,   0,                   0, 0, 0,  1e4,   0,   0, 0},
	{SCF_EDA_Capacitor,  0,                   0, 0, 0,   10, 0.1,   0, 0},
	{SCF_EDA_Inductor,   0,                   0, 0, 0,    0,   0, 1e3, 0},
};

static scf_edata_t  pin_datas[] =
{
	{SCF_EDA_None,       0,                   0, 0, 0,    0,   0,   0, 0},

	{SCF_EDA_Diode,      0,   SCF_EDA_Diode_NEG, 0, 0,  750,   0,   0, 0},

	{SCF_EDA_NPN,        0,       SCF_EDA_NPN_B, 0, 0,  750,   0,   0, 0},
	{SCF_EDA_NPN,        0,       SCF_EDA_NPN_C, 0, 0,   10,   0,   0, 150},

	{SCF_EDA_PNP,        0,       SCF_EDA_PNP_B, 0, 0,  750,   0,   0, 0},
	{SCF_EDA_PNP,        0,       SCF_EDA_PNP_C, 0, 0,   10,   0,   0, 150},
};

static scf_edata_t* _pin_find_data(const uint64_t type, const uint64_t model, const uint64_t pid)
{
	scf_edata_t* ed;

	int i;
	for (i = 0; i < sizeof(pin_datas) / sizeof(pin_datas[0]); i++) {
		ed =              &pin_datas[i];

		if (ed->type == type && ed->model == model && ed->pid == pid)
			return ed;
	}

	return NULL;
}

static scf_edata_t* _component_find_data(const uint64_t type, const uint64_t model)
{
	scf_edata_t* ed;

	int i;
	for (i = 0; i < sizeof(component_datas) / sizeof(component_datas[0]); i++) {
		ed =              &component_datas[i];

		if (ed->type == type && ed->model == model)
			return ed;
	}

	return NULL;
}

ScfEconn* scf_econn__alloc()
{
	ScfEconn* ec = calloc(1, sizeof(ScfEconn));

	return ec;
}

int scf_econn__add_cid(ScfEconn* ec, uint64_t cid)
{
	if (!ec)
		return -EINVAL;

	for (size_t i = 0; i < ec->n_cids; i++) {

		if (ec->cids[i] == cid)
			return 0;
	}

	uint64_t* p = realloc(ec->cids, sizeof(uint64_t) * (ec->n_cids + 1));
	if (!p)
		return -ENOMEM;

	ec->cids = p;
	ec->cids[ec->n_cids++] = cid;
	return 0;
}

int scf_econn__del_cid(ScfEconn* ec, uint64_t cid)
{
	if (!ec)
		return -EINVAL;

	uint64_t* p;
	size_t    i;
	size_t    j;

	for (i = 0; i < ec->n_cids; i++) {

		if (ec->cids[i] == cid) {

			for (j = i + 1;  j  < ec->n_cids; j++)
				ec->cids[j - 1] = ec->cids[j];

			ec->n_cids--;

			p = realloc(ec->cids, sizeof(uint64_t) * ec->n_cids);
			if (p)
				ec->cids = p;
			return 0;
		}
	}

	return -EINVAL;
}

ScfEline* scf_eline__alloc()
{
	ScfEline* l = calloc(1, sizeof(ScfEline));

	return l;
}

int scf_eline__add_line(ScfEline* el, ScfLine*  l)
{
	if (!el || !l)
		return -EINVAL;

	for (size_t i = 0; i < el->n_lines; i++) {

		if (el->lines[i] == l)
			return 0;

		if (el->lines[i]->x0 == l->x0
		 && el->lines[i]->y0 == l->y0
		 && el->lines[i]->x1 == l->x1
		 && el->lines[i]->y1 == l->y1)
			return 0;
	}

	void* p = realloc(el->lines, sizeof(ScfLine*) * (el->n_lines + 1));
	if (!p)
		return -ENOMEM;

	el->lines = p;
	el->lines[el->n_lines++] = l;
	return 0;
}

int scf_eline__del_line(ScfEline* el, ScfLine*  l)
{
	if (!el || !l)
		return -EINVAL;

	void*   p;
	size_t  i;
	size_t  j;

	for (i = 0; i < el->n_lines; i++) {

		if (el->lines[i]->x0 == l->x0
		 && el->lines[i]->y0 == l->y0
		 && el->lines[i]->x1 == l->x1
		 && el->lines[i]->y1 == l->y1) {

			for (j = i + 1;  j  < el->n_lines; j++)
				el->lines[j - 1] = el->lines[j];

			el->n_lines--;

			p = realloc(el->lines, sizeof(ScfLine*) * el->n_lines);
			if (p)
				el->lines = p;
			return 0;
		}
	}

	return -EINVAL;
}

int scf_eline__add_pin(ScfEline* el, uint64_t  cid, uint64_t pid)
{
	if (!el)
		return -EINVAL;

	for (size_t i = 0; i + 1 < el->n_pins; i += 2) {

		if (el->pins[i] == cid && el->pins[i + 1] == pid)
			return 0;
	}

	uint64_t* p = realloc(el->pins, sizeof(uint64_t) * (el->n_pins + 2));
	if (!p)
		return -ENOMEM;

	el->pins = p;
	el->pins[el->n_pins++] = cid;
	el->pins[el->n_pins++] = pid;
	return 0;
}

int scf_eline__del_pin(ScfEline* el, uint64_t  cid, uint64_t pid)
{
	if (!el)
		return -EINVAL;

	uint64_t* p;
	size_t    i;
	size_t    j;

	for (i = 0; i + 1 < el->n_pins; i += 2) {

		if (el->pins[i] == cid && el->pins[i + 1] == pid) {

			for (j = i + 2;  j  < el->n_pins; j++)
				el->pins[j - 2] = el->pins[j];

			el->n_pins -= 2;

			p = realloc(el->pins, sizeof(uint64_t) * el->n_pins);
			if (p)
				el->pins = p;
			return 0;
		}
	}

	return -EINVAL;
}

int scf_eline__add_conn(ScfEline* el, ScfEconn* ec)
{
	if (!el || !ec)
		return -EINVAL;

	for (size_t i = 0; i < el->n_conns; i++) {

		if (el->conns[i] == ec)
			return 0;
	}

	void* p = realloc(el->conns, sizeof(ScfEconn*) * (el->n_conns + 1));
	if (!p)
		return -ENOMEM;

	el->conns = p;
	el->conns[el->n_conns++] = ec;
	return 0;
}

int scf_eline__del_conn(ScfEline* el, ScfEconn* ec)
{
	if (!el || !ec)
		return -EINVAL;

	void*   p;
	size_t  i;
	size_t  j;

	for (i = 0; i < el->n_conns; i++) {

		if (el->conns[i] == ec) {

			for (j = i + 1;  j  < el->n_conns; j++)
				el->conns[j - 1] = el->conns[j];

			el->n_conns--;

			p = realloc(el->conns, sizeof(ScfEconn*) * el->n_conns);
			if (p)
				el->conns = p;
			return 0;
		}
	}

	return -EINVAL;
}

ScfEpin* scf_epin__alloc()
{
	ScfEpin* pin = calloc(1, sizeof(ScfEpin));

	return pin;
}

int scf_epin__add_component(ScfEpin* pin, uint64_t cid, uint64_t pid)
{
	if (!pin)
		return -EINVAL;

	for (size_t i = 0; i + 1 < pin->n_tos; i += 2) {

		if (pin->tos[i] == cid && pin->tos[i + 1] == pid)
			return 0;
	}

	uint64_t* p = realloc(pin->tos, sizeof(uint64_t) * (pin->n_tos + 2));
	if (!p)
		return -ENOMEM;

	pin->tos = p;
	pin->tos[pin->n_tos++] = cid;
	pin->tos[pin->n_tos++] = pid;
	return 0;
}

int scf_epin__del_component(ScfEpin* pin, uint64_t cid, uint64_t pid)
{
	if (!pin)
		return -EINVAL;

	uint64_t* p;
	size_t    i;
	size_t    j;

	for (i = 0; i + 1 < pin->n_tos; i += 2) {

		if (pin->tos[i] == cid && pin->tos[i + 1] == pid) {

			for (j = i + 2;  j  < pin->n_tos; j++)
				pin->tos[j - 2] = pin->tos[j];

			pin->n_tos -= 2;

			p = realloc(pin->tos, sizeof(uint64_t) * pin->n_tos);
			if (p)
				pin->tos = p;
			return 0;
		}
	}

	return -EINVAL;
}

ScfEcomponent* scf_ecomponent__alloc(uint64_t type)
{
	ScfEcomponent* c;
	scf_edata_t*      ed;

	if (type >= SCF_EDA_Components_NB)
		return NULL;

	c = calloc(1, sizeof(ScfEcomponent));
	if (!c)
		return NULL;

	c->type = type;

	ed = _component_find_data(c->type, c->model);
	if (ed) {
		c->v  = ed->v;
		c->a  = ed->a;
		c->r  = ed->r;
		c->uf = ed->uf;
		c->uh = ed->uh;
	}

	int i;
	for (i = 0; i < component_pins[type]; i++) {

		ScfEpin* pin = scf_epin__alloc();
		if (!pin) {
			ScfEcomponent_free(c);
			return NULL;
		}

		pin->id = i;

		if (scf_ecomponent__add_pin(c, pin) < 0) {
			ScfEcomponent_free(c);
			ScfEpin_free(pin);
			return NULL;
		}

		ed = _pin_find_data(c->type, c->model, pin->id);
		if (ed) {
			pin->v   = ed->v;
			pin->a   = ed->a;
			pin->r   = ed->r;
			pin->uf  = ed->uf;
			pin->uh  = ed->uh;
			pin->hfe = ed->hfe;
		}
	}

	return c;
}

int scf_ecomponent__add_pin(ScfEcomponent* c, ScfEpin* pin)
{
	if (!c || !pin)
		return -EINVAL;

	void* p = realloc(c->pins, sizeof(ScfEpin*) * (c->n_pins + 1));
	if (!p)
		return -ENOMEM;

	c->pins = p;
	c->pins[c->n_pins++] = pin;
	return 0;
}

int scf_ecomponent__del_pin(ScfEcomponent* c, ScfEpin* pin)
{
	if (!c)
		return -EINVAL;

	size_t   i;
	size_t   j;
	void*    p;

	for (i = 0; i < c->n_pins; i++) {

		if (c->pins[i] == pin) {

			for (j = i + 1; j  < c->n_pins; j++)
				c->pins[j - 1] = c->pins[j];

			c->n_pins--;

			p = realloc(c->pins, sizeof(ScfEpin*) * c->n_pins);
			if (p)
				c->pins = p;
			return 0;
		}
	}

	return -EINVAL;
}

ScfEfunction*  scf_efunction__alloc(const char* name)
{
	ScfEfunction* f = calloc(1, sizeof(ScfEfunction));
	if (!f)
		return NULL;

	f->n_name = strlen(name) + 1;

	f->name = strdup(name);
	if (!f->name) {
		free(f);
		return NULL;
	}

	return f;
}

int scf_efunction__add_component(ScfEfunction* f, ScfEcomponent* c)
{
	if (!f || !c)
		return -EINVAL;

	void* p = realloc(f->components, sizeof(ScfEcomponent*) * (f->n_components + 1));
	if (!p)
		return -ENOMEM;

	f->components = p;
	f->components[f->n_components++] = c;
	return 0;
}

int scf_efunction__del_component(ScfEfunction* f, ScfEcomponent* c)
{
	if (!f || !c)
		return -EINVAL;

	size_t   i;
	size_t   j;
	void*    p;

	for (i = 0; i < f->n_components; i++) {

		if (f->components[i] == c) {

			for (j = i + 1;  j < f->n_components; j++)
				f->components[j - 1] = f->components[j];

			f->n_components--;

			p = realloc(f->components, sizeof(ScfEcomponent*) * f->n_components);
			if (p)
				f->components = p;
			return 0;
		}
	}

	return -EINVAL;
}

int scf_efunction__add_eline(ScfEfunction* f, ScfEline* el)
{
	if (!f || !el)
		return -EINVAL;

	void* p = realloc(f->elines, sizeof(ScfEline*) * (f->n_elines + 1));
	if (!p)
		return -ENOMEM;

	f->elines = p;
	f->elines[f->n_elines++] = el;
	return 0;
}

int scf_efunction__del_eline(ScfEfunction* f, ScfEline* el)
{
	if (!f || !el)
		return -EINVAL;

	size_t   i;
	size_t   j;
	void*    p;

	for (i = 0; i < f->n_elines; i++) {

		if (f->elines[i] == el) {

			for (j = i + 1;  j < f->n_elines; j++)
				f->elines[j - 1] = f->elines[j];

			f->n_elines--;

			p = realloc(f->elines, sizeof(ScfEline*) * f->n_elines);
			if (p)
				f->elines = p;
			return 0;
		}
	}

	return -EINVAL;
}

ScfEboard* scf_eboard__alloc()
{
	ScfEboard* b = calloc(1, sizeof(ScfEboard));

	return b;
}

int scf_eboard__add_function(ScfEboard* b, ScfEfunction* f)
{
	if (!b || !f)
		return -EINVAL;

	void* p = realloc(b->functions, sizeof(ScfEfunction*) * (b->n_functions + 1));
	if (!p)
		return -ENOMEM;

	b->functions = p;
	b->functions[b->n_functions++] = f;
	return 0;
}

int scf_eboard__del_function(ScfEboard* b, ScfEfunction* f)
{
	if (!b)
		return -EINVAL;

	size_t   i;
	size_t   j;
	void*    p;

	for (i = 0; i < b->n_functions; i++) {

		if (b->functions[i] == f) {

			for (j = i + 1;  j < b->n_functions; j++)
				b->functions[j - 1] = b->functions[j];

			b->n_functions--;

			p = realloc(b->functions, sizeof(ScfEfunction*) * b->n_functions);
			if (p)
				b->functions = p;
			return 0;
		}
	}

	return -EINVAL;
}
