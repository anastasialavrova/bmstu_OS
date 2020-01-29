.386P
desct struc
	limit	dw 0
	base_l	dw 0
	base_m	db 0
	attr_1	db 0
	attr_2	db 0
	base_h	db 0
desct ends
data segment
	gdt_null	desct <0,0,0,0,0,0>
	gdt_data	desct <data_size-1,0,0,92h>
	gdt_code	desct <code_size-1,,,98h>
	gdt_stack	desct <255,0,0,92h>
	gdt_screen	desct <4095,8000h,0Bh,92h>
	gdt_size = $-gdt_null
	pdescr	df 0
	sym		db 1
	attr 	db 1Eh
	mes		db 27,'[31;42m Process working in real mode! ',27,'[0m$'
	data_size=$-gdt_null
data ends
text segment 'code' use16
	assume CS:text,DS:data
main proc
	xor		EAX, EAX
	mov		AX, data
	mov		DS, AX
	shl		EAX, 4
	mov 	EBP, EAX
	mov 	BX, offset gdt_data
	mov 	[BX].base_l, AX
	rol		EAX, 16
	mov 	[BX].base_m, AL
	xor		EAX, EAX
	mov 	AX, CS
	shl 	EAX, 4
	mov 	BX, offset gdt_code
	mov 	[BX].base_l, AX
	rol 	EAX, 16
	mov 	[BX].base_m, AL
	xor 	EAX, EAX
	mov 	AX, SS
	shl 	EAX, 4
	mov 	BX, offset gdt_stack
	mov 	[BX].base_l, AX
	rol 	EAX, 16
	mov 	[BX].base_m, AL
	mov 	dword ptr pdescr+2, EBP
	mov 	word ptr pdescr, gdt_size-1
	lgdt 	pdescr
	cli
	mov 	AL, 80h
	out 	70h, AL
	mov 	EAX, CR0
	or 		EAX, 1
	mov 	CR0, EAX
	db 		0EAh
	dw		offset continue
	dw 		16
continue:
	mov		AX, 8
	mov 	DS, AX
	mov		AX, 24
	mov		SS, AX
	mov		AX, 32
	mov 	ES, AX

	mov 	BX, 800
	mov 	CX, 640
	mov 	AX, word ptr sym
	mov		AH,	71h
screen: mov ES:[BX], AX
	add 	BX, 2
	inc 	AX
	loop	screen

	mov 	gdt_data.limit, 	0FFFFh
	mov 	gdt_code.limit, 	0FFFFh
	mov 	gdt_stack.limit, 	0FFFFh
	mov		gdt_screen.limit,	0FFFFh
	mov		AX, 8
	mov 	DS, AX
	mov 	AX, 24
	mov 	SS, AX
	mov 	AX, 32
	mov 	ES, AX
	db		0EAh
	dw		offset go
	dw 		16
go: mov		EAX, CR0
	and		EAX, 0FFFFFFFEh
	mov 	CR0, EAX
	db 		0EAh
	dw 		offset return
	dw 		text
return: mov AX, data
	mov		DS, AX
	mov 	AX, stk
	mov 	SS, AX
	sti
	mov		AL, 0
	out 	70h, AL
	mov 	AH, 09h
	mov 	DX, offset mes
	int 	21h
	mov 	AX, 4C00h
	int 	21h
main endp
code_size=$-main
text ends
stk 	segment stack 'stack'
	db 	256 dup ('^')
stk 	ends
	end main
