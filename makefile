# vim: set noet
#===---------------------------------------------*- makefile -*---===
#: md2mdoc
#===--------------------------------------------------------------===

#--------------------------------------------------------------------
# Set the target name and source file list.
#--------------------------------------------------------------------
TARGET			= md2mdoc

$(TARGET) : SOURCES	= \
		  src/main.c

PREFIX			:=	/usr/local/bin
MANPATH			:=	/usr/local/share/man/man7

CC				:= cc
CFLAGS			:= -fno-exceptions -pipe -Wall -W
REMOVE			:= rm -f
CTAGS           := ctags
CP              := cp

# for BSD
HASH_VERSION:sh	= git rev-parse --short=7 HEAD
# for GNU (ignored by non-gmake versions)
HASH_VERSION 	?= $(shell git rev-parse --short=7 HEAD)

#-X- HASH_VERSION=$(printf "$(date '+%Y.%m.%d')."; git rev-parse --short=7 HEAD)

#--------------------------------------------------------------------
# Define the target compile instructions.
#--------------------------------------------------------------------
md2mdoc: clean
	MD2MDOC_TARGET='md2mdoc'
		@echo "const char program_version[] = \"${HASH_VERSION}\";" > src/version.h
		$(CC) $(CFLAGS) -o md2mdoc $(SOURCES)
		@rm src/version.h

.PHONY: clean
clean:
	MD2MDOC_CLEAN='md2mdoc'
		$(REMOVE) md2mdoc $(objects)

.PHONY: almostclean
almostclean:
	$(RM) $(objects)

.PHONY: tags
tags:
	$(CTAGS) -R .

.PHONY: install
install:
	$(CP) md2mdoc $(PREFIX)
	$(CP) ./doc/md2mdoc.7 $(MANPATH)

.PHONY: uninstall
uninstall:
	$(RM) $(PREFIX)/md2mdoc
	$(RM) $(MANPATH)/md2mdoc.7

.PHONY: all
all: md2mdoc tags almostclean
