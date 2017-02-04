// MAX_PUSH_DWORDS controls how many dwords 'asmentry' will reserve for possible stack pushes
// this value can't be too large (no more than one page)
// this value mustn't smaller than 3
#define MAX_PUSH_DWORDS 16

#define X ((MAX_PUSH_DWORDS - 3) * 4)
#define Y ((MAX_PUSH_DWORDS - 1) * 4)
#define Z ((MAX_PUSH_DWORDS + 3) * 4)

.data
.p2align 2
.globl _max_push_dwords
_max_push_dwords:
.long MAX_PUSH_DWORDS

.text
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
