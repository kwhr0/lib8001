#! /usr/bin/perl

print <<'EOF';
	.module	tone
	.globl	_tone
	.globl	_tonetable
	.area	_CODE
_tone:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	e,4(ix)
	ld	d,5(ix)
	ld	c,6(ix)
	ld	b,7(ix)
	ld	a,8(ix)
	.db	0xdd
	ld	l,a
	ld	a,#1
	ex	af,af'
	ld	hl,#0
3$:
	add	hl,de
	jr	nc,2$
	ex	af,af'
	xor	#0x20
	out	(0x40),a
	ex	af,af'
	jp	5$
2$:
	ld	a,#0
	jr	1$
1$:
	jr	5$
5$:
	dec	bc
	ld	a,c
	or	b
	jp	nz,3$
	.db	0xdd
	ld	a,l
	or	a
	jr	z,4$
	dec	ix
	jp	3$
4$:
	ld	a,#1
	out	(0x40),a
	pop	ix
	ret
_tonetable:
EOF
for ($i = 12; $i < 128; $i++) {
	printf("\t.dw\t%d\n", 4.16 * 440.0 * 2.0 ** (($i - 69) / 12.0));
}
exit 0;
