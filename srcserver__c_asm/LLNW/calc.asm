.386
.model flat, c
EXTRN   printf:near
.data
fmt1 db "result = ", 0;
fmt2 db "%f", 13, 10, 0;
temp qword ?
K equ 00025630h; (153136d)
.code
calc PROC
    finit
    push ebp
	mov ebp, esp

	mov dword ptr temp, K
	fild temp
	fadd qword ptr [ebp + 8]  ; K + B2

	fld qword ptr [ebp + 20]
	fdiv dword ptr [ebp + 16]
	fchs
	fadd                      ; X = K + B2 - D2/C1
	
	fld dword ptr [ebp + 28]
	fmul qword ptr [ebp + 32]
	fadd                      ; X = K + B2 - D2/C1 + E1*F2

	pop ebp
	ret

calc ENDP
END