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

This file will convert a simple markdown file to use mdoc 

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
$ git clone git@gitlab.localdomain:john/md2mdoc.git
```

## BUILD INSTRUCTIONS 

This project uses MAKE.

```bash
$ cd md2mdoc 
$ make 
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

## AUTHOR
* John Kaul - john.kaul@outlook.com
