	.module	spritedraw
	.globl	_spriteDraw
; sp+2 src pointer
; sp+4 vram pointer
XPOS	=	6	; sp+6 x pix position
; sp+7 y pix position
XCNT	=	8	; sp+8 x chr count
; sp+9 y chr count
XLIM	=	10	; sp+10 x limit
YLIM	=	11	; sp+11 y limit
	.area	_CODE
_spriteDraw:
	push	ix
	ld	ix,#2
	add	ix,sp
	ld	a,XPOS(ix)
	sra	a
	cp	#0xd0
	jr	nc,5$
	and	#0x7f
5$:
	ld	(cx),a
	ld	b,#0
	ld	c,7(ix)
	sra	c
	sra	c
	jp	p,6$
	ld	b,#0xff
6$:
	ld	l,c
	ld	h,b
	add	hl,hl
	add	hl,bc
	add	hl,hl
	add	hl,bc
	add	hl,hl
	add	hl,bc
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	e,4(ix)
	ld	d,5(ix)
	add	hl,de
	ld	b,#0
	ld	a,(cx)
	or	a
	jp	p,8$
	ld	b,#0xff
8$:
	ld	c,a
	add	hl,bc
	ld	e,2(ix)
	ld	d,3(ix)
	ld	b,9(ix)
	ld	c,7(ix)
	ld	a,c
	and	#3
	jp	nz,y_not_aligned
	ld	a,XPOS(ix)
	and	#1
	jr	nz,x_not_aligned_y_aligned
;x_aligned_y_aligned
	ld	a,(cx)
	ld	XPOS(ix),a
	sra	c
	sra	c
4$:
	push	bc
	ld	b,XCNT(ix)
	ld	a,c	;y pos
	cp	YLIM(ix)
	jr	nc,7$
	push	hl
	ld	c,XPOS(ix)
2$:
	push	bc
	ld	b,XLIM(ix)
	ld	a,c	;x pos
	cp	b
	jr	nc,1$
	ld	a,(de)
	or	(hl)
	ld	(hl),a
	inc	de
	ld	a,(de)
	dec	de
	and	(hl)
	ld	(hl),a
1$:
	pop	bc
	inc	c
	inc	de
	inc	de
	inc	hl
	djnz	2$
	pop	hl
	jr	3$
7$:
	sla	b
	ld	c,b
	ld	b,#0
	ex	de,hl
	add	hl,bc
	ex	de,hl
3$:
	ld	bc,#120
	add	hl,bc
	pop	bc
	inc	c
	djnz	4$
	pop	ix
	ret
x_not_aligned_y_aligned:
	ld	a,(cx)
	ld	XPOS(ix),a
	sra	c
	sra	c
14$:
	push	bc
	ld	b,XCNT(ix)
	ld	a,c	;y pos
	cp	YLIM(ix)
	jr	nc,16$
	push	hl
	ld	c,XPOS(ix)
12$:
	push	bc
	ld	b,XLIM(ix)
	ld	a,c	;x pos
	cp	b
	jr	nc,15$
	ld	a,(de)
	rlca
	rlca
	rlca
	rlca
	and	#0xf0
	or	(hl)
	ld	(hl),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	rlca
	or	#0xf
	and	(hl)
	ld	(hl),a
	ld	a,c	;x pos
15$:
	inc	a
	cp	b
	jr	nc,11$
	ld	a,(de)
	rlca
	rlca
	rlca
	rlca
	and	#0xf
	inc	hl
	or	(hl)
	ld	(hl),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	rlca
	or	#0xf0
	and	(hl)
	ld	(hl),a
	dec	hl
11$:
	pop	bc
	inc	c
	inc	de
	inc	de
	inc	hl
	djnz	12$
	pop	hl
	jr	13$
16$:
	sla	b
	ld	c,b
	ld	b,#0
	ex	de,hl
	add	hl,bc
	ex	de,hl
13$:
	ld	bc,#120
	add	hl,bc
	pop	bc
	inc	c
	djnz	14$
	pop	ix
	ret
y_not_aligned:
	push	hl
	pop	iy
	ld	a,XPOS(ix)
	and	#1
	rlca
	rlca
	push	de
	ld	e,a
	ld	a,c
	and	#3
	or	e
	rlca
	ld	e,a
	ld	d,#0
	ld	hl,#table
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	(work),de
	ld	de,#15
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	(workt),de
	ld	de,#15
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	(workb),de
	pop	de
	ld	a,(cx)
	ld	XPOS(ix),a
	sra	c
	sra	c
34$:
	ld	hl,(work)
	push	bc
	ld	b,XCNT(ix)
	ld	a,c	;y pos
	cp	#255
	jr	nz,36$
	ld	hl,(workt)
	jr	35$
36$:
	cp	YLIM(ix)
	jr	nc,37$
	push	bc
	ld	b,YLIM(ix)
	dec	b
	cp	b
	pop	bc
	jr	nz,35$
	ld	hl,(workb)
35$:
	push	iy
	ld	c,XPOS(ix)
32$:
	push	bc
	ld	b,XLIM(ix)
	push	hl
	ld	hl,#31$
	ex	(sp),hl
	jp	(hl)
31$:
	pop	bc
	inc	c
	inc	de
	inc	de
	inc	iy
	djnz	32$
	pop	iy
	jr	33$
37$:
	sla	b
	ld	c,b
	ld	b,#0
	ex	de,hl
	add	hl,bc
	ex	de,hl
33$:
	ld	bc,#120
	add	iy,bc
	pop	bc
	inc	c
	djnz	34$
	pop	ix
	ret
x0y1:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	and	#0xee
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0x11
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0x11
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0xee
	and	120(iy)
	ld	120(iy),a
	ret
x0y2:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	and	#0xcc
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0x33
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rrca
	rrca
	and	#0x33
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0xcc
	and	120(iy)
	ld	120(iy),a
	ret
x0y3:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#0x88
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0x77
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rrca
	and	#0x77
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0x88
	and	120(iy)
	ld	120(iy),a
	ret
x0y1t:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0x11
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0xee
	and	120(iy)
	ld	120(iy),a
	ret
x0y2t:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	and	#0x33
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0xcc
	and	120(iy)
	ld	120(iy),a
	ret
x0y3t:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	and	#0x77
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0x88
	and	120(iy)
	ld	120(iy),a
	ret
x0y1b:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	and	#0xee
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0x11
	and	0(iy)
	ld	0(iy),a
	ret
x0y2b:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	and	#0xcc
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0x33
	and	0(iy)
	ld	0(iy),a
	ret
x0y3b:
	ld	a,c
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#0x88
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0x77
	and	0(iy)
	ld	0(iy),a
	ret
x1y1:
	ld	a,c
	cp	b
	jr	nc,41$
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0xe0
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0x1f
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rlca
	and	#0x10
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0xef
	and	120(iy)
	ld	120(iy),a
41$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0xe
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0xf1
	and	1(iy)
	ld	1(iy),a
	ld	a,(de)
	rlca
	and	#1
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0xfe
	and	121(iy)
	ld	121(iy),a
	ret
x1y2:
	ld	a,c
	cp	b
	jr	nc,42$
	ld	a,(de)
	rrca
	rrca
	and	#0xc0
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0x3f
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rlca
	rlca
	and	#0x30
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0xcf
	and	120(iy)
	ld	120(iy),a
42$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	and	#0xc
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0xf3
	and	1(iy)
	ld	1(iy),a
	ld	a,(de)
	rlca
	rlca
	and	#3
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0xfc
	and	121(iy)
	ld	121(iy),a
	ret
x1y3:
	ld	a,c
	cp	b
	jr	nc,43$
	ld	a,(de)
	rrca
	and	#0x80
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0x7f
	and	0(iy)
	ld	0(iy),a
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#0x70
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0x8f
	and	120(iy)
	ld	120(iy),a
43$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	and	#8
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0xf7
	and	1(iy)
	ld	1(iy),a
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#7
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0xf8
	and	121(iy)
	ld	121(iy),a
	ret
x1y1t:
	ld	a,c
	cp	b
	jr	nc,47$
	ld	a,(de)
	rlca
	and	#0x10
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0xef
	and	120(iy)
	ld	120(iy),a
47$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	and	#1
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	or	#0xfe
	and	121(iy)
	ld	121(iy),a
	ret
x1y2t:
	ld	a,c
	cp	b
	jr	nc,48$
	ld	a,(de)
	rlca
	rlca
	and	#0x30
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0xcf
	and	120(iy)
	ld	120(iy),a
48$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	and	#3
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	or	#0xfc
	and	121(iy)
	ld	121(iy),a
	ret
x1y3t:
	ld	a,c
	cp	b
	jr	nc,49$
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#0x70
	or	120(iy)
	ld	120(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0x8f
	and	120(iy)
	ld	120(iy),a
49$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rlca
	rlca
	rlca
	and	#7
	or	121(iy)
	ld	121(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rlca
	rlca
	rlca
	or	#0xf8
	and	121(iy)
	ld	121(iy),a
	ret
x1y1b:
	ld	a,c
	cp	b
	jr	nc,44$
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0xe0
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0x1f
	and	0(iy)
	ld	0(iy),a
44$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	rrca
	and	#0xe
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	rrca
	or	#0xf1
	and	1(iy)
	ld	1(iy),a
	ret
x1y2b:
	ld	a,c
	cp	b
	jr	nc,45$
	ld	a,(de)
	rrca
	rrca
	and	#0xc0
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0x3f
	and	0(iy)
	ld	0(iy),a
45$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	rrca
	and	#0xc
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	rrca
	or	#0xf3
	and	1(iy)
	ld	1(iy),a
	ret
x1y3b:
	ld	a,c
	cp	b
	jr	nc,46$
	ld	a,(de)
	rrca
	and	#0x80
	or	0(iy)
	ld	0(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0x7f
	and	0(iy)
	ld	0(iy),a
46$:
	ld	a,c
	inc	a
	cp	b
	ret	nc
	ld	a,(de)
	rrca
	and	#8
	or	1(iy)
	ld	1(iy),a
	inc	de
	ld	a,(de)
	dec	de
	rrca
	or	#0xf7
	and	1(iy)
	ld	1(iy),a
	ret
table:
	.dw	0,x0y1,x0y2,x0y3
	.dw	0,x1y1,x1y2,x1y3
	.dw	0,x0y1t,x0y2t,x0y3t
	.dw	0,x1y1t,x1y2t,x1y3t
	.dw	0,x0y1b,x0y2b,x0y3b
	.dw	0,x1y1b,x1y2b,x1y3b
	.area	_DATA
work:
	.ds	2
workt:
	.ds	2
workb:
	.ds	2
cx:
	.ds	1
