CFILES += ../util/scf_string.c

CFILES += ../elf/scf_elf.c
CFILES += ../elf/scf_elf_link.c
CFILES += ../elf/scf_elf_native.c
CFILES += ../elf/scf_elf_native32.c
CFILES += ../elf/scf_elf_x64.c
CFILES += ../elf/scf_elf_x64_so.c
CFILES += ../elf/scf_elf_arm64.c
CFILES += ../elf/scf_elf_arm64_so.c
CFILES += ../elf/scf_elf_arm32.c
CFILES += ../elf/scf_elf_arm32_so.c
CFILES += ../elf/scf_elf_naja.c
CFILES += ../elf/scf_elf_naja_so.c
CFILES += ../elf/scf_dwarf.c
CFILES += ../elf/scf_dwarf_abbrev.c
CFILES += ../elf/scf_dwarf_info.c
CFILES += ../elf/scf_dwarf_line.c

CFILES += ../vm/scf_vm.c
CFILES += ../vm/scf_vm_naja.c
CFILES += ../vm/scf_vm_naja_asm.c
CFILES += ../vm/scf_vm_test.c

CFLAGS += -g
#CFLAGS += -Wall 
CFLAGS += -I../util
CFLAGS += -I../core
CFLAGS += -I../lex
CFLAGS += -I../parse
CFLAGS += -I../elf
CFLAGS += -I../vm
CFLAGS += -I../native
CFLAGS += -I../native/x64
CFLAGS += -I../native/risc

LDFLAGS += -ldl

all:
	gcc $(CFLAGS) $(CFILES) $(LDFLAGS) -o nvm

clean:
	rm *.o
