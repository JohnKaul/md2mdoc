<!--------------------------------------------*- MARKDOWN -*------
File Last Updated: October 21 2020 16:01

File:    readme.md
Author:  John Kaul <john.kaul@outlook.com>
Brief:   This file contains the readme instructions for my project.
------------------------------------------------------------------>
# md2mdoc

## BRIEF

This project is a simple markdown to mdoc (for man pages) converter.

## DESCRIPTION

This file will convert a simple markdown file to use mdoc format. This
will allow for easier man page creation.

This tool was built to help myself create man pages. I normally keep
project notes, documentation in a simple markdown format. I use these
notes in my readme's, documentation things, etc. while I am making
large decisions about how my program should operate. This utility, I
thought, would allow me to keep my notes in a simple markdown format
until my project notes/design decisions are more complete and/or
ready to be moved to a more formal format (mdoc).

For example, in the design phase of a project I will create a
man-page-style-document where I write down my specifications
(program arguments, inputs, flags, etc.) and using a simple text file
allows me to make large decisions--cut/add--at will.

Example:
```markdown
    # NAME
    projectname -- my project to simplify the world.

    # SYNOPSIS
    projectname mdfile mdocfile

    # DESCRIPTION
    A long project decription about the proect here.

    # OPTIONS
    -a
        Append a file.

    -B
        Use backward searching patterns
    ...
```

Will get converted to:

```mdoc
    .Sh  NAME
    projectname -- my project to simplify the world.
    .Pp
    .Sh  SYNOPSIS
    projectname mdfile mdocfile
    .Pp
    .Sh  DESCRIPTION
    A Long project description about the project here.
    .Pp
    By sourcing this `hints` file in Vim, indexed definitions
    (including `subroutines`, `typedefs`, `defines`, `structs`,
    `enums`, and `unions`) can be displayed on the status line when
    typing of the definition in the buffer.
    .Pp
    .Sh  OPTIONS
    .Bl -tag -width Ds
    .It Fl a
    Append a file.
    .Pp
    .It Fl B
    Use backward searching patterns
    ...
```

## DOWNLOAD INSTRUCTIONS

To clone a copy:

```bash
$ cd ~/<place>/<you>/<keep>/<your>/<code>
$ git clone git@git:john/md2mdoc.git
```

## BUILD INSTRUCTIONS

This project uses MAKE to build, install and uninstall.

To build:
```bash
$ cd md2mdoc
$ make
```

To install:
```bash
% doas make install
```

To uninstall:
```bash
% doas make uninstall
```

I have also included a simple configure script which can be used to change the location for the install.

To change the install location you can use something like the following:
```bash
% ./configure --prefix=/home/john/bin
```

## CONTRIBUTION GUIDELINES

### Git Standards

#### Commiting

1.  Commit each file as changes are made.
2.  Do not commit files in batch.
3.  Please prefix all commits with the file you are commiting.
4.  Separate subject from body with a blank line
5.  Limit the subject line to 50 characters
6.  Capitalize the subject line
7.  Do not end the subject line with a period
8.  Use the imperative mood in the subject line
9.  Wrap the body at 72 characters
10. Use the body to explain what and why vs. how

## HISTORY
* Created for my personal use.

## AUTHOR
* John Kaul
