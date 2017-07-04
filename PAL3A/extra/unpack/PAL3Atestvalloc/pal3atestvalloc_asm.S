.data
.align 4
.globl _unpack_entry
_unpack_entry:
.space 4

.text
.globl _start
_start:
	pusha
	call _testvalloc_entry
	popa
	jmp *_unpack_entry
