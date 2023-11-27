include "../lib/scf_list.c";
include "../lib/scf_capi.c";

struct Data
{
	list list;
	int  a;
};

int main()
{
	Data* d;
	list  h;
	int   i;

	list_init(&h);

	for (i = 0; i < 10; i++) {
		d  = malloc(sizeof(Data));

		d->a = i;
		list_add_tail(&h, &d->list);
	}

	list* l;

	for (l = list_head(&h); l != list_sentinel(&h); ) {

		d  = container(l, Data, list);
		l  = list_next(l);

		printf("%d\n", d->a);

		list_del(&d->list);
		free(d);
	}

	return 0;
}
