& January 8, 2024
% SAMPLE 1
@ John Kaul

# NAME
projectname - This is a sample test.

a
    line below a newline.

# SYNOPSIS
md2mdoc <mdfile> <mdocfile>

# DESCRIPTION
`fhints` makes a `hints` file from the specified C sources.  A `hints`
file gives the function prototypes of functions found in source
files.  Each line of the hints file contains a hint that can be
displayed on Vim's status line.

By sourcing this `hints` file in Vim, indexed definitions
(including `subroutines`, `typedefs`, `defines`, `structs`,
`enums`, and `unions`) can be displayed on the status line when
typing of the definition in the buffer.

# OPTIONS
-a
    Append to *hints* file.

-B
    Use backward searching patterns (?...?).

-d
    Create hints for `#defines` that don't take arguments (ON
    by default); `#defines` that take arguments are tagged
    automatically.
-

This is more text added to the bottom of the file.

This is another line of text added to the bottom of the page.

# SECTION
more text.

# OPTIONS
-XX
    some argument text
-YY
    some more arg text
-
