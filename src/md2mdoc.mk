# ===--------------------------------------------*- makefile -*---===
#: md2mdoc
# This makefile is used by my project-root makefile and is
# located in the source directory.
#
# _target       =       program name
# _depend       =       required libaries
# _source       =       source file list
# ===-------------------------------------------------------------===

#: md2mdoc
_target	:= md2mdoc
_depend :=
_source	:= src/main.c

$(eval $(call make-program,$(_target),$(_source),$(_depend)))
#:~

# vim: set noet
