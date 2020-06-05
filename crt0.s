	.module	crt0
	.globl	_main
	.globl	_interruptCount
	.globl	_keyTarget
	.globl	_keyWait
	.globl	l__DATA
	.globl	s__DATA
	.globl	l__INITIALIZER
	.globl	s__INITIALIZER
	.globl	s__INITIALIZED
	.globl	_interruptHandler
	.area	_HEADER (ABS)
	.org	0
;	ld	sp,#0xf300	; stack location (single VRAM)
	ld	sp,#0xe700	; stack location (double VRAM)
	call	gsinit
	jr	0$
	ret
0$:
	ei
	call	_main
1$:
	halt
	jr	1$
	.org	0x38
	push	af
	exx
	ld	hl,#_interruptCount
	inc	(hl)
	ld	a,(_keyTarget)
	sub	(hl)
	jr	nz,6$
	ld	(_keyWait),a
6$:
	ld	hl,(_interruptHandler)
	ld	a,l
	or	h
	jr	z,4$
	push	ix
	push	iy
	ld	de,#5$
	push	de
	jp	(hl)
5$:
	pop	iy
	pop	ix
4$:
	exx
	pop	af
	ei
	reti
	.org	0x66
	retn
	;
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area	_GSINIT
	.area	_GSFINAL
	.area	_DATA
	.area	_INITIALIZED
	.area	_HEAP
	.area	_HEAP_END
	;
	.area	_GSINIT
gsinit:
	ld	bc,#l__DATA
	ld	hl,#s__DATA
3$:
	ld	(hl),#0
	inc	hl
	dec	bc
	ld	a,c
	or	b
	jr	nz,3$
	ld	bc,#l__INITIALIZER
	ld	a,b
	or	a,c
	jr	z,2$
	ld	de,#s__INITIALIZED
	ld	hl,#s__INITIALIZER
	ldir
2$:
	.area	_GSFINAL
	ret
	.area	_DATA
_interruptHandler:
	.ds	2
_interruptCount:
	.ds	1
_keyTarget:
	.ds	1
_keyWait:
	.ds	1
