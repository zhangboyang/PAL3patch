#include "common.h"

#define X ((MAX_PUSH_DWORDS - 2) * 4)
#define Y ((MAX_PUSH_DWORDS - 1) * 4)
#define Z ((MAX_PUSH_DWORDS + 2) * 4)

.text
.align 0x10
.globl _asmentry@4
_asmentry@4:
	sub $X, %esp
	pushl Y(%esp)
	pushl Y(%esp)
	pushal
	addl $Z, 0xc(%esp)
	pushl %esp
	call _patchentry
	addl $0x4, %esp
	movl 0x20(%esp), %eax
	movl 0xc(%esp), %ecx
	subl $4, %ecx
	movl %eax, (%ecx)
	movl %ecx, 0x20(%esp)
	popal
	popl %esp
	ret

/*
when MAX_PUSH_DWORDS == 10:
    00401000    83EC 20         SUB ESP,20
    00401003    FF74E4 24       PUSH DWORD PTR SS:[ESP+24]
    00401007    FF74E4 24       PUSH DWORD PTR SS:[ESP+24]
    0040100B    60              PUSHAD
    0040100C    8344E4 0C 30    ADD DWORD PTR SS:[ESP+0C],30
    00401011    54              PUSH ESP
    00401012    E8 29000000     CALL 00401040
    00401017    83C4 04         ADD ESP,4
    0040101A    8B44E4 20       MOV EAX,DWORD PTR SS:[ESP+20]
    0040101E    8B4CE4 0C       MOV ECX,DWORD PTR SS:[ESP+0C]
    00401022    83E9 04         SUB ECX,4
    00401025    8901            MOV DWORD PTR DS:[ECX],EAX
    00401027    894CE4 20       MOV DWORD PTR SS:[ESP+20],ECX
    0040102B    61              POPAD
    0040102C    5C              POP ESP
    0040102D    C3              RETN

how to use asm entry:
	push FUNC_ID
	call asmentry
*/
