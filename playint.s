	.module	playint
	.globl	_player
	.globl	_playRestore
	.globl	_playStopCh
	.globl	_playInterrupt
BUF	=	0
P	=	2
WAIT	=	4
CH	=	6
FLAGS	=	7
RR	=	8
ENV	=	24
SIZE	=	28
PLAYER_N	=	8
SN	=	0x90
B_ENV	=	0xe8
B_SLEEP	=	0xee
B_TERM	=	0xef
INTERVAL	=	17
	.area	_CODE
_playRestore:
	pop	bc
	pop	hl	;hl=Player*
	pop	de	;e=restore
	push	de
	push	hl
	push	bc
	ld	bc,#CH
	add	hl,bc
	ld	a,(hl)	;a=c->ch
	and	e
	ld	e,a	;e=restore&c->ch
	ld	c,#RR-CH
	add	hl,bc	;hl=p
	ld	c,#SN
	ld	d,#1	;d=m
3$:
	ld	a,e
	and	d
	jr	z,1$
	ld	b,#3
	otir
	jr	2$
1$:
	inc	hl
	inc	hl
	inc	hl
2$:
	inc	hl
	sla	d
	ld	a,d
	cp	#0x10
	jr	c,3$
	ret
_playStopCh:
	pop	bc
	pop	de	;e=ch
	push	de
	push	bc
	ld	d,#1
	ld	bc,#0x9f00|SN
4$:
	ld	a,e
	and	d
	jr	z,5$
	out	(c),b
5$:
	sla	d
	ld	a,b
	add	#0x20
	ld	b,a
	jr	nc,4$
	ret
_playInterrupt:
;	push	ix
	xor	a
	ld	(restore),a
	ld	(higher_ch),a
;for (Player *c = player + PLAYER_N; c >= player; c--) {
	ld	iy,#_player
	ld	bc,#SIZE*(PLAYER_N-1)
	add	iy,bc
	ld	b,#PLAYER_N
29$:
	push	bc
;if (!c->p) continue;
	ld	a,P(iy)
	or	P+1(iy)
	jp	z,28$
;if (restore) playRestore(c, restore);
	ld	a,(restore)
	or	a
	jr	z,27$
	ld	c,a
	push	bc
	push	hl
	call	_playRestore
	pop	hl
	pop	bc
27$:
;if (c->wait) {
	ld	l,WAIT(iy)
	ld	h,WAIT+1(iy)
	ld	a,l
	or	h
	jr	z,25$
;s16 t = c->wait - INTERVAL;
	ld	bc,#-INTERVAL
	add	hl,bc
;if (t < 0) t = 0;
	jr	c,26$
	ld	hl,#0
26$:
;c->wait = t;
	ld	WAIT(iy),l
	ld	WAIT+1(iy),h
;for (u8 i = 0, j = 2, b = 0x90, m = 1; i < 4; i++, j += 4, b += 0x20; m <<= 1) {
	push	iy
	pop	hl
	ld	bc,#RR+3
	add	hl,bc	;hl=rp
	push	iy
	pop	ix
	ld	bc,#ENV
	add	ix,bc	;ix=envp
	ld	bc,#0x9001	;b=b,c=m
	ld	d,#0
24$:
;u16 t = c->r[j + 1] + c->env[i];
	ld	a,(ix)
	inc	ix
	add	(hl)
;if (t > 0xff) t = 0xff;
	jr	nc,22$
	ld	a,#0xff
22$:
;c->r[j + 1] = t;
	ld	(hl),a
;t = b | t >> 4;
	rrca
	rrca
	rrca
	rrca
	and	#0xf
	or	b
;c->r[j] = t;
	dec hl
	ld	(hl),a
;if (!(higher_ch & m)) SN = t;
	ld	e,a
	ld	a,(higher_ch)
	and	c
	jr	nz,23$
	ld	a,e
	out	(SN),a
23$:
	ld	e,#5
	add	hl,de
	sla	c
	ld	a,#0x20
	add	b
	ld	b,a
	jr	nc,24$
25$:
;while (c->p && !c->wait) {
;u8 *p = c->p;
	ld	l,P(iy)
	ld	h,P+1(iy)	;hl=p
20$:
	ld	a,l
	or	h
	jp	z,21$
	ld	a,WAIT(iy)
	or	a,WAIT+1(iy)
	jp	nz,21$
;last = 0, valid = 0;
	ld	ix,#0	;ix=lastp
	ld	e,#0	;e=valid
12$:
;while (*p != SLEEP && *p != TERM)
	ld	a,(hl)
	cp	#B_SLEEP
	jp	z,18$
	cp	#B_TERM
	jp	z,10$
	inc	hl
;if (*p >= ENV && *p < ENV + 4) {
	cp	#B_ENV
	jr	c,11$
	cp	#B_ENV+4
	jr	nc,11$
;c->env[*p - ENV] = p[1];
;p += 2;
	sub	#B_ENV
	ld	c,a
	ld	b,#0
	ld	a,(hl)
	inc	hl
	push	iy
	add	iy,bc
	ld	ENV(iy),a
	pop	iy
	jr	12$
11$:
;else {
;u8 d = *p++;
;u8 sel = d >> 3 & 0xe;
	ld	d,a	;d=d
	rrca
	rrca
	rrca
	and	#0xe	;a=sel
;if (d & 0x80) {
	bit	7,d
	jr	z,15$
;c->r[sel] = d;
;last = sel;
	add	#RR+1
	ld	c,a
	ld	b,#0
	push	iy
	pop	ix
	add	ix,bc	;ix=lastp
	ld	-1(ix),d
	sub	#RR+1
;u8 m = 1 << (sel >> 2);
	rrca
	rrca
	and	#3
	ld	b,a
	ld	a,#1	;m
	jr	z,13$
14$:
	rlca
	djnz	14$
13$:
;c->ch |= m;
	ld	c,a
	or	CH(iy)
	ld	CH(iy),a
;valid = !(higher_ch & m);
	ld	a,(higher_ch)
	and	c
	ld	a,#0
	jr	nz,17$
	ld	a,#1
17$:
	ld	e,a	;e=valid
;if (d & 0x10) c->r[sel | 1] = d << 4;
	bit	4,d
	jr	z,16$
	ld	a,d
	rlca
	rlca
	rlca
	rlca
	and	#0xf0
	ld	(ix),a
	jr	16$
15$:
;else c->r[last | 1] = d;
	ld	a,d
	ld	(ix),a
16$:
;if (valid) SN = d;
	ld	a,e
	or	a
	jp	z,12$
	ld	a,d
	out	(SN),a
	jp	12$
18$:
;if (*p == SLEEP) {
;c->wait = p[1] | p[2] << 8;
;p += 3;
	inc	hl
	ld	a,(hl)
	inc	hl
	ld	WAIT(iy),a
	ld	a,(hl)
	inc	hl
	ld	WAIT+1(iy),a
	jp	20$
10$:
;if (c->flags & PF_LOOP) p = c->buf;
	bit	0,FLAGS(iy)
	jr	z,19$
	ld	l,BUF(iy)
	ld	h,BUF+1(iy)
	jp	20$
19$:
;else {
;p = nil;
	ld	hl,#0
;restore |= ~higher_ch & c->ch;
	ld	a,(higher_ch)
	cpl
	and	CH(iy)
	ld	b,a
	ld	a,(restore)
	or	b
	ld	(restore),a
;c->ch = 0;
	xor	a
	ld	CH(iy),a
	jp	20$
21$:
;c->p = p;
	ld	P(iy),l
	ld	P+1(iy),h
;higher_ch |= c->ch;
	ld	a,(higher_ch)
	or	CH(iy)
	ld	(higher_ch),a
;restore &= ~c->ch;
	ld	a,(restore)
	and	CH(iy)
	ld	(restore),a
28$:
	ld	bc,#-SIZE
	add	iy,bc
	pop	bc
	dec	b
	jp	nz,29$
;playStopCh(restore);
	ld	hl,(restore)
	push	hl
	call	_playStopCh
	pop	hl
;	pop	ix
	ret
	.area	_DATA
restore:
	.ds	1
higher_ch:
	.ds	1
