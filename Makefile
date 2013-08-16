prefix=/usr/local
bindir=$(prefix)/bin
includedir=$(prefix)/include
libdir=$(prefix)/lib
sysconfdir=$(prefix)/etc

PROGSRC = $(sort $(wildcard *.c))
PROGOBJ = $(PROGSRC:.c=.o)
PROGS = $(PROGSRC:.c=)

CFLAGS = -std=gnu99 -fstack-protector-all -Wa,--noexecstack
LDFLAGS = -static -lcrypt

CC    ?= gcc

-include config.mak

all: $(PROGS)

install: $(PROGS:%=$(DESTDIR)$(bindir)/%)

clean:
	rm -f $(PROGS)

%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(DESTDIR)$(bindir)/%: %
	install -D -m 755 $< $@

.PHONY: all clean install



