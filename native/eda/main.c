#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"scf_eda_pb.h"
#include"scf_def.h"


int main()
{
	ScfEcomponent* r      = scf_ecomponent__alloc(SCF_EDA_None);
	ScfEcomponent* d0     = scf_ecomponent__alloc(SCF_EDA_None);
	ScfEcomponent* d1     = scf_ecomponent__alloc(SCF_EDA_None);

	ScfEpin*       rp0    = scf_epin__alloc();
	ScfEpin*       rp1    = scf_epin__alloc();

	ScfEpin*       d0p0   = scf_epin__alloc();
	ScfEpin*       d0p1   = scf_epin__alloc();

	ScfEpin*       d1p0   = scf_epin__alloc();
	ScfEpin*       d1p1   = scf_epin__alloc();

	ScfEpin*       rps[]  = {rp0,  rp1};
	ScfEpin*       d0ps[] = {d0p0, d0p1};
	ScfEpin*       d1ps[] = {d1p0, d1p1};

	int64_t c[] = {1, 2};
	int64_t b[] = {0, 2};
	int64_t a[] = {0, 1};

	scf_epin__add_component(rp1, 1, SCF_EDA_Diode_NEG);
	scf_epin__add_component(rp1, 2, SCF_EDA_Diode_NEG);

	scf_epin__add_component(d0p1, 0, 0);
	scf_epin__add_component(d0p1, 2, SCF_EDA_Diode_NEG);

	scf_epin__add_component(d1p1, 0, 0);
	scf_epin__add_component(d1p1, 1, SCF_EDA_Diode_NEG);

	r->id      = 0;
	r->type    = SCF_EDA_Resistor;
	scf_ecomponent__add_pin(r, rp0);
	scf_ecomponent__add_pin(r, rp1);

	d0->id     = 1;
	d0->type   = SCF_EDA_Diode;
	scf_ecomponent__add_pin(d0, d0p0);
	scf_ecomponent__add_pin(d0, d0p1);

	d1->id     = 2;
	d1->type   = SCF_EDA_Diode;
	scf_ecomponent__add_pin(d1, d1p0);
	scf_ecomponent__add_pin(d1, d1p1);

	ScfEfunction* f = scf_efunction__alloc("test");
	scf_efunction__add_component(f, r);
	scf_efunction__add_component(f, d0);
	scf_efunction__add_component(f, d1);

	ScfEboard* board = scf_eboard__alloc();
	scf_eboard__add_function(board, f);

	size_t rlen  = scf_ecomponent__get_packed_size(r);
	size_t d0len = scf_ecomponent__get_packed_size(d0);
	size_t d1len = scf_ecomponent__get_packed_size(d1);
	size_t flen  = scf_efunction__get_packed_size(f);
	size_t blen  = scf_eboard__get_packed_size(board);

	printf("rlen: %ld, d0len: %ld, d1len: %ld, flen: %ld, blen: %ld\n", rlen, d0len, d1len, flen, blen);

	uint8_t pb[1024];

	scf_eboard__pack(board, pb);

	ScfEboard* p = scf_eboard__unpack(NULL, blen, pb);

	printf("p: %p\n", p);
	size_t i;
	size_t j;
	size_t k;
	size_t l;

	for (i = 0; i < p->n_functions; i++) {
		ScfEfunction* pf = p->functions[i];

		printf("f: %s\n", pf->name);

		for (l = 0; l < pf->n_components; l++) {
			ScfEcomponent* pc = pf->components[l];

			printf("i: %ld, pc: %p, id: %ld, cid: %ld, n_pins: %ld\n", i, pc, pc->id, pc->type, pc->n_pins);

			for (j = 0; j < pc->n_pins; j++) {
				ScfEpin* pp = pc->pins[j];

				printf("j: %ld, pp: %p, n_tos: %ld, cid: %ld, pid: %ld\n", j, pp, pp->n_tos, pp->cid, pp->id);

				for (k = 0; k + 1 < pp->n_tos; k += 2) {
					printf("k: %ld, cid: %ld, pid: %ld\n", k, pp->tos[k], pp->tos[k + 1]);
				}
			}
			printf("\n");
		}
		printf("\n\n");
	}

	scf_eboard__free(board);

	scf_eboard__free_unpacked(p, NULL);
	return 0;
}
