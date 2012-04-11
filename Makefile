#
# Makefile for Asterisk Speech Server application
#

INSTALL=install
ETCDIR=/opt/etc/astsphinx
GRAMDIR=\"/opt/etc/astsphinx/grammar/\"
DESTDIR=/opt/bin
CC=gcc
OPTIMIZE=-O2
DEBUG=-g

LIBS+= 
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -D_REENTRANT  -D_GNU_SOURCE  

all: _all
	@echo " +------------Server  Build Complete --------+"  
	@echo " + server has successfully been built,       +"  
	@echo " + and can be installed by running:          +"
	@echo " +                                           +"
	@echo " +               make install                +"  
	@echo " +-------------------------------------------+" 

_all: astsphinx sphx_test


astsphinx.o: astsphinx.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPTIMIZE) `pkg-config pocketsphinx --cflags --libs` -DGRAMDIR=$(GRAMDIR) -c -o astsphinx.o astsphinx.c

prefork.o: prefork.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPTIMIZE) `pkg-config pocketsphinx --cflags --libs` -c -o prefork.o prefork.c

astsphinx: astsphinx.o prefork.o
	$(CC) `pkg-config pocketsphinx --cflags --libs` -o astsphinx astsphinx.o prefork.o

sphx_test: sphx_test.c
	$(CC) `pkg-config pocketsphinx --cflags --libs` -o sphx_test sphx_test.c
	        

clean:
	rm -f astsphinx  sphx_test *.d *.o *.so *~

install: _all
	$(INSTALL) -m 755 astsphinx $(DESTDIR)
	$(INSTALL) -m 755 sphx_test $(DESTDIR)
	
	@echo " +--------- Server Installation Complete ------+"  
	@echo " +                                             +"
	@echo " + server has successfully been installed      +"  
	@echo " + If you would like to install the sample     +"  
	@echo " + configuration file run:                     +"
	@echo " +                                             +"
	@echo " +              make samples                   +"
	@echo " +---------------------------------------------+"

samples:
	mkdir -p $(ETCDIR)
	for x in *.sample; do \
		if [ -f $(ETCDIR)/`basename $$x .sample` ]; then \
			if [ "$(OVERWRITE)" = "y" ]; then \
				if cmp -s $(ETCDIR)/`basename $$x .sample` $$x ; then \
					echo "Config file $$x is unchanged"; \
					continue; \
				fi ; \
				mv -f $(ETCDIR)/`basename $$x .sample` $(ETCDIR)/`basename $$x .sample`.old ; \
			else \
				echo "Skipping config file $$x"; \
				continue; \
			fi ;\
		fi ; \
		$(INSTALL) -m 644 $$x $(ETCDIR)/`basename $$x .sample` ;\
	done
	mkdir -p $(GRAMDIR)
	for x in grammar/*.sample; do \
		if [ -f $(GRAMDIR)/`basename $$x .sample` ]; then \
			if [ "$(OVERWRITE)" = "y" ]; then \
				if cmp -s $(ETCDIR)/`basename $$x .sample` $$x ; then \
					echo "Config file $$x is unchanged"; \
					continue; \
				fi ; \
				mv -f $(GRAMDIR)/`basename $$x .sample` $(GRAMDIR)/`basename $$x .sample`.old ; \
			else \
				echo "Skipping config file $$x"; \
				continue; \
			fi ;\
		fi ; \
		$(INSTALL) -m 644 $$x $(GRAMDIR)/`basename $$x .sample` ;\
	done

ifneq ($(wildcard .*.d),)
   include .*.d
endif
