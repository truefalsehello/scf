#include "scf_eda_pb.h"
#include "scf_def.h"

static int component_pins[SCF_EDA_Components_NB] =
{
	0, // None
	SCF_EDA_Battery_NB, // SCF_EDA_Battery

	2, // SCF_EDA_Resistor
	2, // SCF_EDA_Capacitor
	2, // SCF_EDA_Inductor

	SCF_EDA_Diode_NB,
	SCF_EDA_Transistor_NB,
};

ScfEconn* scf_econn__alloc()
{
	ScfEconn* ec = malloc(sizeof(ScfEconn));
	if (!ec)
		return NULL;

	scf_econn__init(ec);
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

void scf_econn__free(ScfEconn* ec)
{
	if (ec) {
		if (ec->cids)
			free(ec->cids);

		free(ec);
	}
}

ScfEline* scf_eline__alloc()
{
	ScfEline* l = malloc(sizeof(ScfEline));
	if (!l)
		return NULL;

	scf_eline__init(l);
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

void scf_eline__free(ScfEline* l)
{
	if (l) {
		size_t i;

		if (l->pins)
			free(l->pins);

		if (l->conns) {
			for (i = 0; i < l->n_conns; i++)
				scf_econn__free(l->conns[i]);

			free(l->conns);
		}

		if (l->lines) {
			for (i = 0; i < l->n_lines; i++)
				free(l->lines[i]);

			free(l->lines);
		}

		free(l);
	}
}

ScfEpin* scf_epin__alloc()
{
	ScfEpin* pin = malloc(sizeof(ScfEpin));
	if (!pin)
		return NULL;

	scf_epin__init(pin);
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

void scf_epin__free(ScfEpin* pin)
{
	if (pin) {
		scf_logd("pin: %p\n", pin);

		if (pin->tos)
			free(pin->tos);
		free(pin);
	}
}

ScfEcomponent* scf_ecomponent__alloc(uint64_t type)
{
	if (type >= SCF_EDA_Components_NB)
		return NULL;

	ScfEcomponent* c = malloc(sizeof(ScfEcomponent));
	if (!c)
		return NULL;

	scf_ecomponent__init(c);

	c->type = type;

	int i;
	for (i = 0; i < component_pins[type]; i++) {

		ScfEpin* pin = scf_epin__alloc();
		if (!pin) {
			scf_ecomponent__free(c);
			return NULL;
		}

		pin->id = i;

		if (scf_ecomponent__add_pin(c, pin) < 0) {
			scf_ecomponent__free(c);
			scf_epin__free(pin);
			return NULL;
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

void scf_ecomponent__free(ScfEcomponent* c)
{
	if (c) {
		scf_logd("c: %ld\n", c->id);

		if (c->pins) {
			size_t i;

			for (i = 0; i < c->n_pins; i++)
				scf_epin__free(c->pins[i]);

			free(c->pins);
		}

		free(c);
	}
}

ScfEfunction*  scf_efunction__alloc(const char* name)
{
	ScfEfunction* f = malloc(sizeof(ScfEfunction));
	if (!f)
		return NULL;

	scf_efunction__init(f);

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

void scf_efunction__free(ScfEfunction* f)
{
	if (f) {
		scf_logd("f: %s\n", f->name);

		if (f->components) {
			size_t i;

			for (i = 0; i < f->n_components; i++)
				scf_ecomponent__free(f->components[i]);

			free(f->components);
		}

		free(f->name);
		free(f);
	}
}

ScfEboard* scf_eboard__alloc()
{
	ScfEboard* b = malloc(sizeof(ScfEboard));
	if (!b)
		return NULL;

	scf_eboard__init(b);
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

void scf_eboard__free(ScfEboard* b)
{
	if (b) {
		if (b->functions) {
			size_t i;

			for (i = 0; i < b->n_functions; i++)
				scf_efunction__free(b->functions[i]);

			free(b->functions);
		}

		free(b);
	}
}
