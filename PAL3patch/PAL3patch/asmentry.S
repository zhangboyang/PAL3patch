#include "asmpatch.h"

#define X ((MAX_PUSH_DWORDS - 3) * 4)
#define Y ((MAX_PUSH_DWORDS - 1) * 4)
#define Z ((MAX_PUSH_DWORDS + 3) * 4)

.text
.align 0x10
.globl _asmentry@4
_asmentry@4:
	pushf
	sub $X, %esp
	pushl Y(%esp)
	pushl Y(%esp)
	pushl Y(%esp)
	pushal
	addl $Z, 0xc(%esp)
	subl $0x6c, %esp
	fnsave (%esp)
	pushl %esp
	call _patchentry
	addl $0x4, %esp
	frstor (%esp)
	addl $0x6c, %esp
	movl 0xc(%esp), %ecx
	movl 0x24(%esp), %eax
	movl 0x20(%esp), %edx
	subl $0x8, %ecx
	movl %eax, 0x4(%ecx)
	movl %edx, (%ecx)
	movl %ecx, 0x20(%esp)
	popal
	popl %esp
	popf
	ret

/*
how to use asm entry:
	push FUNC_ID
	call asmentry
*/
