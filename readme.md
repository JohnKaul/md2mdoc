<!--------------------------------------------*- MARKDOWN -*------
File Last Updated: October 21 2020 16:01

File:    readme.md
Author:  John Kaul <john.kaul@outlook.com>
Brief:   This file contains the readme instructions for my project.
------------------------------------------------------------------>
# md2mdoc

## BRIEF

This project is a simple markdown to mdoc (for man pages) converter.

## SYNOPSIS
md2mdoc [-o outputfile] inputfile

## OPTIONS
-o outputfile
    A mandoc file to write.

- inputfile
    A file written in the markdown syntax outlined below.

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
    .Nm projectname 
    .Nd my project to simplify the world.
    .Pp
    .Sh  SYNOPSIS
    projectname mdfile mdocfile
    .Pp
    .Sh  DESCRIPTION
    A Long project description about the project here.
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

## EXAMPLES
Create an 'output' mandoc file from markdown 'input':
```sh
    % md2mdoc -o output input
```

Create an 'output' mandoc file from markdown 'input' via redirect:
```sh
    % md2mdoc input > output
```

Pipe a markdown file 'input' to ^mandoc(1)^ for processing on the fly:
```sh
    % md2mdoc input | mandoc -mdoc
```

Pipe a markdown file 'input' to ^mandoc(1)^ for viewing:
```sh
    % md2mdoc input | mandoc -mdoc | less
```

## DOWNLOAD INSTRUCTIONS

To clone a copy:

```sh
    $ cd ~/<place>/<you>/<keep>/<your>/<code>
    $ git clone git@git:john/md2mdoc.git
```

## BUILD INSTRUCTIONS

This project uses MAKE to build, install and uninstall.

To build:
```sh
    $ cd md2mdoc
    $ make
```

To install:
```bash
    % doas make install
```

To uninstall:
```sh
    % doas make uninstall
```

I have also included a simple configure script which can be used to change the location for the install.

To change the install location you can use something like the following:
```sh
    % ./configure --prefix=/home/john/bin
```

## CONTRIBUTION GUIDELINES
### Git Standards
Contributions should be single-subject only (single-line bug fixes,
small refactors, spelling/typo fixes, etc.). Changes that touch
multiple files or address several unrelated topics are hard to review
and may have unforeseen side effects; contributions spanning multiple
subjects may be requested to be split into smaller, focused commits or
closed.

This project includes a test script (`runtest.sh`) which can run
against possible code changes.

<!---
### Versioning
Use MAJOR.MINOR.PATCH:
- **MAJOR** — incompatible API changes
- **MINOR** — added or improved backward-compatible functionality
- **PATCH** — backward-compatible bug fixes
-->

### Committing
1. List file(s) in the subject line. If committing several files,
   prefix the subject line with "(MF)" (multiple files) and separate
   filenames with commas.
2. Separate subject from body with a blank line.
3. Limit the subject line to 50 characters.
4. Do not end the subject line with a period.
5. Use the imperative mood in the subject line.
6. Wrap the body at 72 characters.
7. Use the body to explain what and why (not how).

Example commit:
```
    (MF) changelog, version.h, md2mdoc.7

    1. Update documentation.
```

### Use of AI
Code produced with AI assistance may be accepted only if it is:
- Trivial and not copyrightable (e.g., single-line fixes, basic refactors), or
- Accompanied by a public statement from the AI provider that they do
  not assert copyright over the generated work.

AI-assisted code is often harder to maintain and for that reason,
human-authored patches for larger changes is preferred. Large-scope
refactors or additions that appear to be AI-generated can be closed
without explanation.

## HISTORY
* Created for my personal use.

## AUTHOR
* John Kaul
