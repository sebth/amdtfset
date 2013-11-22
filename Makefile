LDLIBS=-ldl
PREFIX=/usr/local
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m644
TARGET=amdtfset

$(TARGET): $(TARGET).c

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin \
		$(DESTDIR)$(PREFIX)/share/man/man1
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(PREFIX)/bin
	-$(INSTALL_DATA) $(TARGET).1 $(DESTDIR)$(PREFIX)/share/man/man1

install-strip: $(TARGET)
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/$(TARGET).1

tags: $(TARGET).c
	ctags $(TARGET).c

clean:
	rm -f $(TARGET) tags core
