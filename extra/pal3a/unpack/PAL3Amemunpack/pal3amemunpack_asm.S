.data
.align 4
.globl _oep
_oep:
.space 4

.text
.globl _start
_start:
	add $4, %esp
	pusha
	call _unpack
	popa
	jmp *_oep
