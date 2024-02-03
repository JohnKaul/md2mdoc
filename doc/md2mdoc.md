& Feb 03 2024
% md2mdoc 7
@ John Kaul

# NAME
md2mdoc -- a simple markdown to mdoc converter.

# SYNOPSIS
md2mdoc <mdfile> <mdocfile>

# DESCRIPTION
This utility will convert simple markdown syntax characters to mdoc
(man page) format characters. It will remove all leading spaces from
lines.

This utility is not intelligent at all, it will give wonky results if
confused but it does offer a very good staring point for creating man
page documentation for your project.

The use of this utility is intended to offer a way to keep project
notes/documention in a simple markdown format (plain text) where large
sections can be added/deleted easier. When the time comes to create
more formal documentaiton, this utility aids in that process (assuming
your markdown isn't too complicated).

# CHARACTER CONVERSION TABLE
<md
    #           ->  .Sh     : section headers
    blank line  ->  .Pp     : Blank Line
    # OPTIONS   : This section is assumed to contain project options,
                  so a list block is started directly below this section.
    -<char>     ->  .It Fl  : list element
    - (only)    ->  .El     : a single dash is assumed to be a `list end`.
    ~           ->  .El     : an alternate `list end` character.
    @           ->  .Au     : author
    &           ->  .Dd     : date
    %           ->  .Dt     : document title
    <           ->  .nf     : start of a `no format` block.
    >           ->  .fi     : end of a `no format` block.
>

# SAMPLE MARKDOWN EXAMPLE
<md
    & Feb 03 2024
    % progname 7
    @ John Kaul

    # NAME
    progname -- a program to change the world.

    # SYNOPSIS
    progname <inpputfile> <outputfile>

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
    -

    # CODE EXAMPLE
    <c
        unsigned long   str_version;    /* version number */
        unsigned long   str_numstr;     /* # of strings in the file */
        unsigned long   str_longlen;    /* length of longest string */
    >
>

# HISTORY
Created for personal use.

# AUTHOR
John Kaul (john.kaul@outlook.com)
