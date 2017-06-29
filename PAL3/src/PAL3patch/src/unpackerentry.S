.text
.globl _sforce_unpacker_entry
_sforce_unpacker_entry:
	subl $4, %esp
	pusha
	call _sforce_unpacker_init
	movl %eax, 0x20(%esp)
	popa
	ret
