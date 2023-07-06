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

ScfEline* scf_eline__alloc()
{
	ScfEline* l = malloc(sizeof(ScfEline));
	if (!l)
		return NULL;

	scf_eline__init(l);
	return l;
}

void scf_eline__free(ScfEline* l)
{
	if (l)
		free(l);
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

int scf_epin__add_line(ScfEpin* pin, ScfEline* l)
{
	if (!pin || !l)
		return -EINVAL;

	for (size_t i = 0; i < pin->n_lines; i++) {

		if (pin->lines[i]->x0 == l->x0
		 && pin->lines[i]->y0 == l->y0
		 && pin->lines[i]->x1 == l->x1
		 && pin->lines[i]->y1 == l->y1)
			return 0;
	}

	void* p = realloc(pin->lines, sizeof(ScfEline*) * (pin->n_lines + 1));
	if (!p)
		return -ENOMEM;

	pin->lines = p;
	pin->lines[pin->n_lines++] = l;
	return 0;
}

int scf_epin__del_line(ScfEpin* pin, ScfEline* l)
{
	if (!pin || !l)
		return -EINVAL;

	void*  p;
	size_t i;
	size_t j;
	size_t n = pin->n_lines;

	for (i = 0; i < pin->n_lines; ) {

		if (pin->lines[i]->x0 == l->x0
		 && pin->lines[i]->y0 == l->y0
		 && pin->lines[i]->x1 == l->x1
		 && pin->lines[i]->y1 == l->y1) {

			for (j = i + 1;  j    < pin->n_lines; j++)
				pin->lines[j - 1] = pin->lines[j];

			pin->n_lines--;
		} else
			i++;
	}

	if (pin->n_lines < n) {
		p = realloc(pin->lines, sizeof(ScfEline*) * pin->n_lines);
		if (p)
			pin->lines = p;
	}

	return 0;
}

void scf_epin__free(ScfEpin* pin)
{
	if (pin) {
		scf_logd("pin: %p\n", pin);

		if (pin->lines) {
			size_t i;

			for (i = 0; i < pin->n_lines; i++)
				scf_eline__free(pin->lines[i]);

			free(pin->lines);
		}

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
	if (!f)
		return -EINVAL;

	size_t   i;
	size_t   j;
	void*    p;

	for (i = 0; i < f->n_components; i++) {

		if (f->components[i] == c) {

			for (j = i + 1;  j < f->n_components; j++)
				f->components[j - 1] = f->components[j];

			f->n_components--;

			p = realloc(f->components, sizeof(ScfEfunction*) * f->n_components);
			if (p)
				f->components = p;
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
