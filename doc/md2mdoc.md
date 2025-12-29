date: Feb 03 2024
title: md2mdoc 7
author: John Kaul

# NAME
md2mdoc -- a simple markdown to mdoc converter.

# SYNOPSIS
md2mdoc <input MDfile> <output MDOCfile>

# DESCRIPTION
This utility will convert simple markdown syntax characters to mdoc
(man page) format characters. It will remove all leading spaces from
lines unless within a code block.

This utility is not intelligent at all, it will give wonky results if
confused but it does offer a very good staring point for creating man
page documentation for your project.

The use of this utility is intended to offer a way to keep project
notes/documention in a simple markdown format (plain text) where large
sections can be added/deleted easier. When the time comes to create
more formal documentaiton, this utility aids in that process (assuming
your markdown isn't too complicated).

# CHARACTER CONVERSION TABLE

```md
    #           ->  .Sh     : section headers
    # NAME      ->          : md2mdoc will assume the next line will
                              be a name and a description. This should
                              be formatted as the example below.
    blank line  ->  .Pp     : Blank Line
    - <string>  ->  .It     : List item.
    -<char>     ->  .It Fl  : List element and a space after the <char>
                              is assumed to be an argument to the flag.
    EXAMPLE:
    -<char>          ->  .It Fl <char>
    -<char> <string> ->  .It Fl <char> Ar <string>
    - <string>       ->  .It <string>

    -           ->  .El     : A single dash--followed by a newline--is
                              assumed to be a `list end`.
    ~           ->  .El     : An alternate `list end` character.
    <           ->  .nf     : Start of a `no format` block.
    >           ->  .fi     : End of a `no format` block.
    ```         ->  .nf     : Start/End of a `no format` block.
    *           ->  .Bf     : Bold
    _           ->  .Em     : Italic
    ^           ->  .Sx     : Reference
    [           ->  .Op     : Optional argument
    author:     ->  .Au     : Author
    date:       ->  .Dd     : Date
    title:      ->  .Dt .Os : Document title with section number. See
                              manual section numbers and example below.
   <!--         ->          : Start of a comment block.
   -->          ->          : End of a comment block.
```

# SAMPLE MARKDOWN EXAMPLE

```md
    <!--
        This is a markdown comment block and should be ignored
        (i.e. not added to mdoc file).
    -->
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

    # OPTIONS
    -a
        Append to the outout file.

    -B
        Use backward searching patterns (?...?).

    -d
        Parse `#defines` that don't take arguments.

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

    *WARNING*
        Never do the above.

    _NOTE_
        This is a note block.

    # SEE ALSO
    ^md2mdoc(7)^
    ^this^
    ^that^
    ^theotherthing^
```

_NOTE_
    All the symbols this program finds are located at the beginning of
    the line. -i.e. This program is not good at parsing the entire
    strings looking for symbols.


# MANUAL SECTIONS
The standard sections for man pages:
```md
       1      User Commands
       2      System Calls
       3      C Library Functions
       4      Devices and Special Files
       5      File Formats and Conventions
       6      Games et. Al.
       7      Miscellanea
       8      System Administration tools and Deamons
```

# MARKDOWN COMMENTS
A comments style header can be kept in the markdown file which md2mdoc
will ignore during processing. The comment style is HTML Tag style
which is also ignored in other markdown processors.

# SEE ALSO
^mdoc(7)^
^mandoc(1)^
^man(1)^

# HISTORY
Created for my personal use.

# AUTHOR
John Kaul
