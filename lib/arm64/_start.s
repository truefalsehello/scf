.text
.global _start, main

_start:
	bl   main
	mov  x8, #93
	svc  #0
.fill 4, 1, 0
