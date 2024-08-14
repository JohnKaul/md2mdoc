#===---------------------------------------------*- makefile -*---===
#: md2mdoc
#===--------------------------------------------------------------===

# Set the following variable to YES/NO to make this makefile quiet.
QUIET			:=      YES

_q = $(subst YES,@,$(subst NO,,$(QUIET)))

#--------------------------------------------------------------------
# Set the target name and source file list.
#--------------------------------------------------------------------
TARGET			= md2mdoc

$(TARGET) : SOURCES	= \
		  src/main.c

# NOTE:
# -Prefix set to $(HOME) so that binaries will be installed to %(HOME)/bin
prefix	        :=	$(HOME)/bin

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
		@$(CC) $(CFLAGS) -o md2mdoc $(SOURCES)

.PHONY: clean
clean:
	$(_q)$(REMOVE) md2mdoc $(objects)

.PHONY: almostclean
almostclean:
	$(_q)$(RM) $(objects)

.PHONY: tags
tags:
	$(_q)$(CTAGS) $(sources)

.PHONY: install
install:
	$(_q)$(CP) md2mdoc $(prefix)
	$(_q)$(CP) ./doc/md2mdoc.7 $(prefix)/man/man7

.PHONY: uninstall
uninstall:
	$(_q)$(RM) $(prefix)/md2mdoc
	$(_q)$(RM) $(prefix)/man/man7/md2mdoc.7

.PHONY: all
all: md2mdoc tags almostclean

# vim: set noet
