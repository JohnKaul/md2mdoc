# ===--------------------------------------------*- makefile -*---===
#: md2mdoc
# This is the main makefile for the project and should reside at
# the root directory.
#
# This makefile allows for having a separate binary tree (build
# location) from the source tree (source location).
#
# Subfolders should contain `module.mk` files which list the
# program/library output and souce code to build them.
#
# Example sub-directory makefile.
# module.mk:
#
# 	 #: programname
# 	 #  -programname source file list
# 	 _target      := programname
# 	 _depend      :=
#        _source      := \
#               src/main.c \
#               src/include/strtonum.c
#
#        $(eval $(call make-program,$(_target),$(_source),$(_depend)))
#        #:~
# ===-------------------------------------------------------------===

# Set the following variable to YES/NO to make this makefile quiet.
QUIET			:=      YES

_q = $(subst YES,@,$(subst NO,,$(QUIET)))

# ===-------------------------------------------------------------===
# Folder setup; these are common directories in my projects.
# ===-------------------------------------------------------------===
srcdir          :=	src
incdir          :=	$(srcdir)/include
extrainc        :=	../../include
testdir			:=	test
docdir          :=	doc
buildlocation   :=	.
# NOTE:
# -Prefix set to $(HOME) so that binaries will be installed to %(HOME)/bin
prefix	        :=	$(HOME)/bin

# ===-------------------------------------------------------------===
# We collect information from each module in these three variables.
# Initilize them here as simple variables.
#
# NOTE: These three variables should be left blank to allow for this
# 	makefile to set them when parsing the sub-module.mk files.
# ===-------------------------------------------------------------===
programs        :=
libraries       :=
sources         :=
documents       :=

include_dirs    :=	$(srcdir) $(incdir) $(docdir) $(extrainc)
vpath           %.h $(include_dirs)

objects         =	$(call source-to-object, $(sources))
dependencies    =	$(subst .o,.d,$(objects))

# ===-------------------------------------------------------------===
# The tools to use for building and other tasks.
# ===-------------------------------------------------------------===
CC              :=	cc
CFLAGS          :=	-fno-exceptions -pipe -Wall -W
CPPFLAGS        +=	$(addprefix -I ,$(include_dirs))
AR              :=	/usr/bin/libtool
ARFLAGS         :=	-static -o
MV              :=	mv -f
RM              :=	rm -f
SED             :=	sed
MKDIR           :=	mkdir -p
CP              :=	cp
CTAGS           :=	ctags
FHINTS			:=  $(HOME)/bin/fhints
TEST            :=	test -d

# ===-------------------------------------------------------------===
# Custom function to add the build location to files.
#
# $(call source-dir-to-binary-dir, directory-list)
# ===-------------------------------------------------------------===
source-dir-to-binary-dir = $(addprefix $(buildlocation)/, $1)

# ===-------------------------------------------------------------===
# Custom function for building and generating outputs for different
# file types.
#
# $(call source-to-object, source-file-list)
# ===-------------------------------------------------------------===
source-to-object = $(call source-dir-to-binary-dir,	\
		   $(subst .c,.o,$(filter %.c,$1)))

# ===-------------------------------------------------------------===
# This function aids in the including of sub-folder .mk files.
# We retieve the name of the current makefile with `MAKEFILE_LIST`
#
# $(subdirectory)
# ===-------------------------------------------------------------===
subdirectory = $(patsubst %/*.mk,%,		\
		 $(word				\
		   $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

# ===-------------------------------------------------------------===
# This function is for building libraries (from within the sub-dir
# makefile).
#
# $(call make-library,library-name,source-file-list)
#
# EX:
# $(eval $(call make-library,$(_target),$(_source)))
# ===-------------------------------------------------------------===
define make-library
 libraries 	+= $(buildlocation)/$1
 sources	+= $2

 $(buildlocation)/$1: $(call source-dir-to-binary-dir,	\
			$(subst .c,.o,$(filter %.c,$2)))
	$(_q)$(AR) $(ARFLAGS) $$@ $$^
endef

# ===-------------------------------------------------------------===
# This function is for building programs (from within the sub-dir
# makefile).
#
# $(call make-program,programname,source-file-list)
#
# EX:
# $(eval $(call make-program,$(_target),$(_source),))
# or without a dependency
# $(eval $(call make-program,$(_target),$(_source),))
# ===-------------------------------------------------------------===
define make-program
 programs	+= $(buildlocation)/$1
 sources	+= $2

 $(buildlocation)/$1: $(call source-dir-to-binary-dir,\
		$(subst .c,.o,$(filter %.c,$2)))\
                $(call source-dir-to-binary-dir,$3)
	$(_q)$(CC) $(CFLAGS) -o $$@ $$^
endef

# ===-------------------------------------------------------------===
# A custom function to copy some files.
# documents	+= $2 $(buildlocation)/$1
# ===-------------------------------------------------------------===
define copy-file
 documents	+= $2

 $(buildlocation)/$1: $(call source-dir-to-binary-dir, $2)
	$(_q)$(CP) $2 $(buildlocation)
endef

# ===-------------------------------------------------------------===
# This function computes `yacc` and `lex` output files.
#
# $(call generated-source, source-file-list)
# ===-------------------------------------------------------------===
generated-source = 	$(call source-dir-to-binary-dir,\
			$(subst .y,.c,$(filter %.y,$1))	\
			$(subst .y,.h,$(filter %.y,$1))	\
			$(subst .l,.c,$(filter %.l,$1)))\
		$(filter %.c,$1)

# ===-------------------------------------------------------------===
# This function relies on the global variable `_source` in the
# `module.mk` files.
#
# NOTE: we must use `eval` because this function expands to more than
# one line of make code.
#
# $(eval $(compile-rules))
# ===-------------------------------------------------------------===
define compile-rules
  $(foreach f, $(_source),\
    $(call one-compile-rule,$(call source-to-object,$f),$f))
endef

# ===-------------------------------------------------------------===
# This function is a way to inform make of the location of the output
# files in the linking of the source and object files.
# -i.e. we created a rule instead of having to provide an explicit
#  rule for linking the source and object files.
#
# $(call one-compile-rule, binary-file, source-files)
# ===-------------------------------------------------------------===
define one-compile-rule
  $1: $(call generated-source,$2)
	$(_q)$(COMPILE.c) -o $$@ $$<

  $(subst .o,.d,$1): $(call generated-source,$2)
	$(_q)$(CC) $(CFLAGS) $(CPPFLAGS) -M $$< | \
	$(SED) 's,\($$(notdir $$*)\.o\) *:,$$(dir $$@)\1 $$@: ,' > $$@.tmp
	$(_q)$(MV) $$@.tmp $$@

endef

# ===-------------------------------------------------------------===
# This function will create a folder in the build_location for each
# directory in the `include_dirs` variable.
# -i.e. this builds the build-tree folders.
# ===-------------------------------------------------------------===
create-output-directories	:=					\
	$(shell for f in $(call source-dir-to-binary-dir,$(include_dirs));\
		do							\
		$(TEST) $$f || $(MKDIR) $$f;				\
	done)

# ===-------------------------------------------------------------===
# Establish a default `all` target.
# ===-------------------------------------------------------------===
.PHONY: all
all:

# ===-------------------------------------------------------------===
# Establish other targets.
# ===-------------------------------------------------------------===
.PHONY: libraries
libraries: $(libraries)

.PHONY: programs
programs: $(objects)

.PHONY: clean
clean:
	$(_q)$(RM) $(objects) $(dependencies) $(programs) $(libraries)

.PHONY: almostclean
almostclean:
	$(_q)$(RM) $(objects)

.PHONY: tags
tags:
	$(_q)$(CTAGS) $(sources)

.PHONY: hints
hints:
	$(_q)$(FHINTS) $(sources)

.PHONY: install
install:
	$(_q)$(CP) $(programs) $(addsuffix /,$(prefix))
	$(_q)$(CP) $(docdir)/md2mdoc.7 $(addsuffix /,$(prefix)/man/man7)
#	$(_q)$(CP) $(documents) $(addsuffix /,$(prefix))

.PHONY: uninstall
uninstall:
	$(_q)$(RM) $(addprefix $(prefix)/,$(programs))
#	$(_q)$(RM) $(addprefix $(prefix)/,$(documents))

# ===-------------------------------------------------------------===
# Include any subdirectory makefiles.
# ===-------------------------------------------------------------===
-include $(addsuffix /*.mk,$(docdir))
-include $(addsuffix /*.mk,$(incdir))
include $(addsuffix /*.mk,$(srcdir))
-include $(addsuffix /*.mk,$(testdir))
ifneq "$(MAKECMDGOAS)" "clean"
 -include $(dependencies)
endif

# set the 'all' target to build the program(s), run ctags,
# and cleanup object files.
all: $(programs) almostclean tags hints

# silently run the compile rules.
.SILENT:
$(eval $(compile-rules))

#:~
# vim: noet

