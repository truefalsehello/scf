#include"scf_vm.h"

int main()
{
	scf_vm_t* vm = NULL;

	int ret = scf_vm_open(&vm, "naja");
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	ret = scf_vm_run(vm, "../parse/1.out", "x64");
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	printf("main ok\n");
	return 0;
}

