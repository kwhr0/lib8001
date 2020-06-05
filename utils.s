	.module	utils
	.globl	_sleep
	.globl	_inp
	.globl	_getR
	.globl	_enterCritical
	.globl	_exitCritical
	.globl	_idle
	.area	_CODE
_sleep:
	pop	hl
	pop	bc
	push	bc
	push	hl
	jr	3$
2$:
	ld	a,#161
1$:
	dec	a
	jr	nz,1$
	dec	bc
3$:
	ld	a,c
	or	b
	jr	nz,2$
	ret
_inp:
	pop	hl
	pop	bc
	push	bc
	push	hl
	in	l,(c)
	ld	h,#0
	ret
_getR:
	ld	a,r
	ld	l,a
	ld	h,#0
	ret
_enterCritical:
	ld	a,r
	di
	jp	po,4$
	ld	hl,#1
	ret
4$:
	ld	hl,#0
	ret
_exitCritical:
	pop	hl
	pop	bc
	push	bc
	push	hl
	ld	a,c
	or	a
	ret	z
	ei
	ret
_idle:
	halt
	ret
