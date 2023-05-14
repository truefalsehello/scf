.text
.global _start, main

_start:
	bl   main
	mov  r7, #1
	swi  #0
.fill 4, 1, 0
