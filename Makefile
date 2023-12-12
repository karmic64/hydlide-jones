ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif


CFLAGS:=-s -Ofast -Wall

%$(DOTEXE): %.c
	$(CC) $(CFLAGS) -o $@ $<



.PHONY: default
default: indy.nes



exported-tracks.txt: export-tracks$(DOTEXE) hydlide-jones.txt
	./$^ $@

indy.nes: indy.asm exported-tracks.txt
	64tass -a -f -o $@ $<


