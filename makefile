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

PREFIX	        :=	
MANPATH			:=	/usr/local/share/man/man7

CC				:= cc
CFLAGS			:= -fno-exceptions -pipe -Wall -W 
REMOVE			:= rm -f
CTAGS           := ctags
CP              := cp

#--------------------------------------------------------------------
# Define the target compile instructions.
#--------------------------------------------------------------------
md2mdoc: clean
	MD2MDOC_TARGET='md2mdoc'
		$(CC) $(CFLAGS) -o md2mdoc $(SOURCES)

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
