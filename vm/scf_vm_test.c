#include"scf_vm.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("usage: ./nvm file\n\n");
		printf("file: an ELF file with naja bytecode\n");
		return -1;
	}

	scf_vm_t* vm = NULL;

	int ret = scf_vm_open(&vm, "naja");
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	ret = scf_vm_run(vm, argv[1], "x64");
	if (ret < 0) {
		scf_loge("\n");
		return -1;
	}

	printf("main ok\n");
	return ret;
}

