.\\\" Copyright (c) 2026 John Kaul
.\\\" SPDX-License-Identifier: BSD-2-Clause
date: Feb 02 2026
title: md2mdoc 7
author: John Kaul

# NAME
md2mdoc -- a simple markdown to mdoc converter.

# SYNOPSIS
$name
[-o outputfile]
inputfile

# OPTIONS
-o outputfile
    A mandoc file to write.

- inputfile
    A file written in the markdown syntax outlined below.
-

# DESCRIPTION
This utility will convert simple markdown syntax characters to ^mdoc(7)^ (man page) format characters. It will remove all leading spaces from lines unless within a code block. It will print to `stdout` unless specified otherwise.

This utility is not intelligent at all, it will give wonky results if confused but it does offer a very good staring point for creating man page documentation for your project.

The use of this utility is intended to offer a way to keep project notes/documentation in a simple markdown format (plain text) where large sections can be added/deleted easier. When the time comes to create more formal documentation, this utility aids in that process (assuming your markdown isn't too complicated).

# CHARACTER CONVERSION TABLE

```md
 markdown         mdoc       description
 ------------------------------------------------------------------------
 # string      ->  .Sh     : section headers
 ## string     ->  .Ss     : subsection headers
 # NAME        ->          : md2mdoc will assume the next line will
                             be a name and a description. This should
                             be formatted as the example below.
 blank line    ->  .Pp     : Blank Line
 - string      ->  .It     : List item (type: tag).
 -char         ->  .It Fl  : List element and a space after the <char>
                             is assumed to be an argument to the flag.
 -             ->  .El     : A single dash--followed by a newline--is
                             a `list end` character.
 ,---
 | EXAMPLE:
 |   -char             ->    .It Fl <char>
 |   -char string      ->    .It Fl <char> Ar <string>
 |   - string          ->    .It <string>
 |   -                 ->    .El
 `---

 ~ string      ->  .It     : List item (type: dash).
 ~             ->  .El     : A tilde--followed by a newline--is a
                             `list end` character.
 ,---
 | EXAMPLE:
 |   ~ string          ->    .It <string>
 |   ~                 ->    .El
 `---

 <             ->  .Bd     : Start of a `no format` ("begin display") block.
 >             ->  .Fi     : End of a `no format` block.
 ```           ->  .Bd     : Start/End of a `no format` ("begin display") block.
 `string`      ->  .Li     : Inline Literal
 *string*      ->  .Sy     : Bold
 _string_      ->  .Em     : Italic
 ^string^      ->  .Xr     : Manpage Reference
 $string       ->  .Sx     : Manpage Section Reference
 $name         ->  .Nm     : Name
 $string       ->  .Sx     : Section Reference
 @string       ->  .Cm     : Command Modifier
 [string]      ->  .Op     : Optional argument
 author:       ->  .Au     : Author
 date:         ->  .Dd     : Date
 title:        ->  .Dt .Os : Document title with section number. See
                             manual section numbers and example below.
 <!--          ->          : Start of a comment block.
 -->           ->          : End of a comment block.
```

# SAMPLE MARKDOWN EXAMPLE

```md
 <!--
     This is a markdown comment block and should be ignored
     (i.e. not added to mdoc file).
 -->
 .\\\\\\" Copyright (c) <year> <who>

 date: Feb 03 2024
 title: progname 7
 author: John Kaul

 # NAME
 progname -- a program to change the world.

 # SYNOPSIS
 progname
 [-abc]
 [-f file]
 inpputfile
 outputfile

 # DESCRIPTION
 This utility will change the world because
 it will remove all leading spaces from lines
 in a text file.

 ~ A List item.
 ~ Another list item.
 ~

 # OPTIONS
 -a
     Append to the outout file.

 -b
     Use backward searching patterns (?...?).

 -c
     Another flag.

 -f file
     Pass a file argument to this option.

 - inputfile
     The file to pass as an input.

 - outputfile
     The file to write.
 -

 # CODE EXAMPLE
 ```c
     unsigned long   str_version;    /* version number */
     unsigned long   str_numstr;     /* # of strings in the file */
     unsigned long   str_longlen;    /* length of longest string */
 ```

 *WARNING:*
     Never do the above.

 _NOTE:_
     This is a note block.

 # SEE ALSO
 ^this(1)^, ^that(2)^, ^theotherthing(3)^
```

_NOTE:_
Most symbols will naturally occur at the start of a line (header, code block, etc.) but a few items like \*bold\*, \_italic\_, \`literal\`, and \^refernces\^, can be nested within the text.

# MARKDOWN SYNTAX ELEMENTS
Markdown syntax elements $name parses.

## Document preamble
```
 date:         document date (one argument)
 title:        document title (one argument)
 author:       document author (one argument)
```
## Semantic markup for command line utilities
When $name encounters a section header titled @NAME the next line is checked and if a string formatted as:
```
 # NAME
 name -- description
```
the @name will be assigned to the .Nm ("name") macro, and @description will be assigned to the .Nd ("description") macro.

After the "name" has been parsed, the `$name` markdown shortcut can be used which will be replaced with the .Nm macro in the ^mdoc(1)^ output.

Utility argument and flag strings are parsed by placing them in square brackets (\[ \]) and I have found that it's often easier to place these argument strings on their own lines (especially during the design phase of my utilities). The utility argument strings will only be parsed if they are on a line of their own.

```
 [-abc]
 [-f file]
 [argument]
 argument
```
## Option Flag lists
```
 -a
     description
 -a argument
     description
 - argument
     description
 -
```
## Displays and lists
Both the traditional three-backticks and the "Vim documentation" method are allowed for code blocks.

List types are:
~ single dash (-) are used for "tag" type lists.
~ single tilde (~) are used for "dash" type lists.
~
```
 ```           display block: [-literal -offset indent]
 ```           end list block.
 <             display block: [-literal -offset indent]
 <             end list block.
 - string      List item: [-tag -width Ds]
 -             End list item.
 ~ string      List item: [-dash -compact]
 ~             End list item.
```
## Sections and cross references
```
 # string      Section header (one line)
 ## string     Subsection header (one line)
 ^string^      manpage reference (enclosed string)
 $name         project name (one string - alpha)
               (See: Semantic markup for command line utilities)
 $string       internal cross reference (one string - alpha)
 @string       Command Modifier
```
## Physical markup
```
 *string*      boldface font (symbolic) (enclosed string)
 _string_      italic font or underline (emphasis)
 `string`      Literal quote (enclosed string)
```

## Manual sections
The standard sections for man pages:
```md
 1             User Commands
 2             System Calls
 3             C Library Functions
 4             Devices and Special Files
 5             File Formats and Conventions
 6             Games et. Al.
 7             Miscellanea
 8             System Administration tools and Deamons
```

## MARKDOWN COMMENTS
A comments style header can be kept in the markdown file which $name will ignore during processing. The comment style is HTML Tag style which is also ignored in other markdown processors.

# EXAMPLES
Create an 'output' ^mandoc(1)^ file from markdown 'input':
```sh
 % md2mdoc -o output input
```

Create an 'output' ^mandoc(1)^ file from markdown 'input' via redirect:
```sh
 % md2mdoc input > output
```

Pipe a markdown file 'input' to ^mandoc(1)^ for processing on the fly:
```sh
 % md2mdoc input | mandoc -mdoc
```

Pipe a markdown file 'input' to ^mandoc(1)^ for viewing with ^less(1)^:
```sh
 % md2mdoc input | mandoc -mdoc | less
```

Pipe a markdown file 'input' to ^mandoc(1)^ for viewing with ^vim(1)^:
```sh
 % md2mdoc input | mandoc -mdoc | vim -M +MANPAGER -c 'map q :q<CR>' -
```

# SEE ALSO
^mdoc(7)^, ^mandoc(1)^, ^man(1)^

# HISTORY
Created for my personal use.

# AUTHOR
John Kaul
