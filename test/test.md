date: January 8, 2024
title: SAMPLE 1
author: John Kaul

# NAME
projectname - This is a sample test.

[
-a
    line below a newline.
]

_NOTE_
    The word note will be italic.

*WARNING* 
    The word warning will be bold.

^md2mdoc(7)^
    The word md2mdoc(7) will be a reference.

# SYNOPSIS
md2mdoc <mdfile> <mdocfile>

# DESCRIPTION
`fhints` makes a `hints` file from the specified C sources.  A `hints`
file gives the function prototypes of functions found in source files.
Each line of the hints file contains a hint that can be displayed on
Vim's status line.

By sourcing this `hints` file in Vim, indexed definitions (including `subroutines`, `typedefs`, `defines`, `structs`, `enums`, and `unions`) can be displayed on the status line when typing of the definition in the buffer.

# OPTIONS
-a
Append to hints file.

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

The code should looke like this.
<c
    #define VERSION 1
    unsigned long   str_version;    /* version number */
    unsigned long   str_numstr;     /* # of strings in the file */
    unsigned long   str_longlen;    /* length of longest string */
    unsigned long   str_shortlen;   /* length of shortest string */

    int cicmp(const char *cp) {
      int len;

      for (len = 0; *cp && (*cp & ~' '); ++cp, ++len)
        continue;
      if (!*cp) {
        return 1;
      }
      return 0;
    }
>

# MORE OPTIONS
[
-XX
some argument text
-YY
some more arg text
]

A code block started with graves.

_EXAMPLE_
```
    #define VERSION 1
    unsigned long   str_version;    /* version number */
    unsigned long   str_numstr;     /* # of strings in the file */
    unsigned long   str_longlen;    /* length of longest string */
    unsigned long   str_shortlen;   /* length of shortest string */

    int cicmp(const char *cp) {
      int len;

      for (len = 0; *cp && (*cp & ~' '); ++cp, ++len)
        continue;
      if (!*cp) {
        return 1;
      }
      return 0;
    }
```

Final line of text.
