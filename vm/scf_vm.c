#include"scf_vm.h"

extern scf_vm_ops_t  vm_ops_naja;
extern scf_vm_ops_t  vm_ops_naja_asm;

static scf_vm_ops_t* vm_ops_array[] =
{
	&vm_ops_naja,
	&vm_ops_naja_asm,

	NULL,
};

int scf_vm_open(scf_vm_t** pvm, const char* arch)
{
	scf_vm_ops_t* ops = NULL;
	scf_vm_t*     vm;

	int  i;
	for (i = 0; vm_ops_array[i]; i++) {

		if (!strcmp(vm_ops_array[i]->name, arch)) {
			ops =   vm_ops_array[i];
			break;
		}
	}

	if (!ops) {
		scf_loge("\n");
		return -EINVAL;
	}

	vm = calloc(1, sizeof(scf_vm_t));
	if (!vm)
		return -ENOMEM;

	vm->ops = ops;

	if (vm->ops->open) {
		int ret = vm->ops->open(vm);
		if (ret < 0)
			return ret;
	}

	*pvm = vm;
	return 0;
}

int scf_vm_clear(scf_vm_t* vm)
{
	if (!vm)
		return -EINVAL;

	if (vm->elf) {
		scf_elf_close(vm->elf);
		vm->elf = NULL;
	}

	if (vm->sofiles) {
		int  i;

		for (i = 0; i < vm->sofiles->size; i++)
			dlclose(vm->sofiles->data[i]);

		scf_vector_free(vm->sofiles);
		vm->sofiles = NULL;
	}

	vm->text = NULL;
	vm->rodata = NULL;
	vm->data = NULL;

	return 0;
}

int scf_vm_close(scf_vm_t* vm)
{
	if (vm) {
		scf_vm_clear(vm);

		if (vm->ops && vm->ops->close)
			vm->ops->close(vm);

		free(vm);
		vm = NULL;
	}

	return 0;
}

int scf_vm_run(scf_vm_t* vm, const char* path, const char* sys)
{
	if (vm  && vm->ops && vm->ops->run && path)
		return vm->ops->run(vm, path, sys);

	scf_loge("\n");
	return -EINVAL;
}

