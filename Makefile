LIBOBJS = base.rel beep.rel graph.rel time.rel mml.rel play.rel \
	playint.rel tone.rel tone_dcsg.rel sprite.rel spritedraw.rel \
	mathi.rel utils.rel R.rel
DEPFILES = base.c beep.c graph.c time.c mml.c play.c sprite.c mathi.c R.c
DEPEND = Depend

all: $(DEPEND) lib8001.a crt0.rel

lib8001.a: $(LIBOBJS)
	sdar r $@ $(LIBOBJS)
#	sdcclib r $@ $(LIBOBJS)

tone.s: mktone.pl
	./mktone.pl >$@

tone_dcsg.c: mktone_dcsg.pl
	./mktone_dcsg.pl >$@

$(DEPEND):
	rm -f $(DEPEND)
	for file in $(DEPFILES); do sdcc -MM $$file | sed -e 's/\.o:/.rel:/' >> $(DEPEND); done
#	sdcc -MM $(DEPFILES) > $(DEPEND)

clean:; rm -f $(DEPEND) tone.s tone_dcsg.c lib8001.a *.{asm,lk,lst,rel,sym}

%.rel: %.c
	sdcc -c -mz80 $<
%.rel:%.s
	sdasz80 -lsw -o $@ $<

-include $(DEPEND)
