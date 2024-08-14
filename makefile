#===---------------------------------------------*- makefile -*---===
#: md2mdoc
#===--------------------------------------------------------------===

#--------------------------------------------------------------------
# Set the target name and source file list.
#--------------------------------------------------------------------
TARGET			= md2mdoc

$(TARGET) : SOURCES	= \
		  src/main.c

EXTRAINC_DIR	:= ../../include

CC				:= cc
CFLAGS			:= -fno-exceptions -pipe -Wall -W -I $(EXTRAINC_DIR)
REMOVE			:= rm -f

#--------------------------------------------------------------------
# Define the target compile instructions.
#--------------------------------------------------------------------
md2mdoc: clean
	MD2MDOC_TARGET='md2mdoc'
		@$(CC) $(CFLAGS) -o md2mdoc $(SOURCES)

.PHONY: clean
clean:
	@$(REMOVE) md2mdoc $(OBJECTS)

.PHONY: almostclean
almostclean:
	$(_q)$(RM) $(objects)

.PHONY: tags
tags:
	$(_q)$(CTAGS) $(sources)

.PHONY: all
all: md2mdoc tags almostclean

# vim: set noet
