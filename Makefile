CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -I.
LDFLAGS += -static

SRC = main.c

shorkmatrix: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkmatrix $(LDFLAGS)
	$(STRIP) shorkmatrix

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

install: shorkmatrix
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 shorkmatrix $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/shorkmatrix

clean:
	rm -f shorkmatrix

.PHONY: install uninstall clean
