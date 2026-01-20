//===---------------------------------------------------*- C -*---===
// File Last Updated: 11.21.24 21:23:49
//
//: md2mdoc
//
// DATE: January 21 2024
// BY  : John Kaul [john.kaul@outlook.com]
//
// DESCRIPTION
// This is a project to convert simple markdown to mdoc (man page)
// format.
//
// KEY:
// ------------------------------------------------------------------
// #           ->  .Sh     : section headers
// blank line  ->  .Pp     : Blank Line
// -<char>     ->  .It Fl  : List element
// -           ->  .El     : A single dash is assumed to be a `list end`.
// ~           ->  .El     : An alternate `list end` character.
// <           ->  .nf     : Start of a `no format` block.
// >           ->  .fi     : End of a `no format` block.
// ```         ->  .nf     : Start/End of a `no format` block.
// *           ->  .Bf     : Bold
// _           ->  .Em     : Italic
// ^           ->  .Sx     : Reference
// author:     ->  .Au     : Author
// date:       ->  .Dd     : Date
// title:      ->  .Dt .Os : Document title.
// # NAME      ->          : md2mdoc will assume the next line will
//                           be a name and a description. This should
//                           be formatted as the example below.
//  <!---
//  your comment goes here
//  and here
//  -->
//
// Example:
//      # NAME
//      projectname -- This is a test
//
//      # SYNOPSYS
//      projectname
//      [-abc argument]
//      variable
//
//      # OPTIONS
//      -a
//          a line for the `a` flag.
//      -b
//          a line for the `b` flag.
//      ...
//===-------------------------------------------------------------===

#include "version.h"

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#if __FreeBSD__                                         /* FreeBSD needs the following includes for
                                                           the S_IRUSR / S_IWUSR macros to work */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

//-------------------------------------------------------------------
// Constants Declarations
//-------------------------------------------------------------------
#define W_FLAGS (O_WRONLY | O_CREAT)                    /* Write flags for file output */
#define W_PERMS (S_IRUSR | S_IWUSR)                     /* Write permissions for file output */

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------
static void *processfd(void *arg);                             /* Open the FD and send to processline(); */
static void processline(char *str);                            /* Process one line of text at a time
                                                                  from the  input file. */
static void processnested(int fd, const char *str);
static int readline(int fd, char *buf, int nbytes);            /* Read a line of text from file. */
static int cimemcmp(const void *s1, const void *s2, size_t n); /* case independent memory regon compare */
static int append_to_fd(int fd, const char *fmt, ...);  /* Append text to fd */
static int read_until(const char **src, char delim, char *dst, size_t dstcap);
static void skip_one_space_or_newline(const char **src);

//-------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------
char *curfile;                                          /* Current input file name */
int filedescriptors[2];                                 /* An array to hold open file descriptors. */
int stripwhitespace = 1;                                /* Used to pause/stop stripping whitespace */
int codeblock       = 0;                                /* Used for middle of codeblock. */
int listblock       = 0;                                /* Used for list blocks. */
int nameflag        = 0;                                /* Set when this program find the string: "# NAME". */
int commentflag     = 0;                                /* Used for comment blocks (HTML style <!-- comment --> */

/*
 * TabAbortCode -- Enums for standard errors.
 */
enum TAbortCode { /*{{{*/
  abortInvalidCommandLineArgs = -1,
  abortRuntimeError = -2,
  abortUnimplementedFeature = -3
}; /*}}}*/

/*
 * Abort Messages --
 *      Keyed to ennumeration type TAbortCode
 */
const static char *abortMsg[] = {
    /*{{{*/
    NULL,
    "Invalid command line arguments",
    "Runtime error",
    "Unimplemented feature",
};
/*}}}*/

/*
 * AbortTranslation --
 *      A fatal error occurred durring the translation. Print the abort
 *      code and then exit.
 *
 * ARGS
 *  ac              :   abort code [-i.e. TabAbortCode]
 *
 * RETURN
 *  void
 *
 * EXAMPLE USAGE
 *  // --Check the command line arguments.
 *  //   if there are not enough arguments, exit.
 *  if (argc != 2) {
 *      fprintf(stderr, "Usage: %s <ARGUMENT>\n", argv[0]);
 *      AbortTranslation(abortInvalidCommandLineArgs);
 *  }
 */
static void AbortTranslation(enum TAbortCode ac) { /*{{{*/
  fprintf(stderr, "**** Error: %s\n", abortMsg[-ac]);
  exit(ac);
} /*}}}*/

/*
 * printussage --
 *      Prints the usage string to enduser (incase they give the wrong
 *      arguments.
 */
static void printusage(char *str) { /*{{{*/
  fprintf(stderr, "**** Usage: %s <markdownfile> <mdocfiletowrite>\n", str);
}
/*}}}*/

/* processfd --
 *      Read lines from a given filedescritor and pass them to the
 *      `processline` function.
 */
static void *processfd(void *arg) { /*{{{*/
  char buff[LINE_MAX];
  int fd;
  curfile = (char *)(arg);
  ssize_t nbytes;

  fd = *((int *)(arg));
  for (;;) {
    if ((nbytes = readline(fd, buff, LINE_MAX)) <= 0)
      break;
    processline(buff /*, (fd + 1), nbytes */);
  }
  return NULL;
}
/*}}}*/

/* cimemcmp --
 *      Preform a case independent memory region compare.
 */
static int cimemcmp(const void *s1, const void *s2, size_t n) { /*{{{*/
  if (n != 0) {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if ((*p1++ & ~' ') != (*p2++ & ~' '))
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return (0);
}
/*}}}*/

/**
 * append_to_fd --
 *      Append formatted string to fd using dprintf.
 *
 * Parameters:
 *  fd  -   File descriptor to write to.
 *  fmt -   printf-style format string.
 *  ... -   Format arguments.
 *
 * Returns number of bytes written or -1 on error.
 */
static int append_to_fd(int fd, const char *fmt, ...) { /* {{{ */
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vdprintf(fd, fmt, ap);
    va_end(ap);
    return ret;
}
/* }}} */

/**
 * read_until --
 *      Read characters from *src into dst up to delim or NUL or capacity-1.
 *
 * NOTES:
 *  Advances *src to the character after the closing delim if found, otherwise
 *  advances *src to the terminating NUL. Always NUL-terminates dst.
 *
 * Parameters:
 *  src      -   Pointer to input pointer; advanced as characters are consumed.
 *  delim    -   Delimiter character to stop at (not copied).
 *  dst      -   Destination buffer for extracted token (NUL-terminated).
 *  dstcap   -   Capacity of dst in bytes.
 *
 * Returns 1 if delim was found and consumed (i.e., *src advanced past it), 0 otherwise.
 */
static int read_until(const char **src, char delim, char *dst, size_t dstcap) { /* {{{ */
    size_t i = 0;
    const char *p = *src;

    while (*p && *p != delim) {
        if (i + 1 < dstcap) {          /* leave room for NUL */
            dst[i++] = *p;
        }
        p++;
    }
    dst[i] = '\0';

    if (*p == delim) {
        p++;                           /* consume closing delim */
        *src = p;
        return 1;
    } else {
        *src = p;                      /* reached NUL */
        return 0;
    }
}
/* }}} */

/**
 * skip_one_space_or_newline --
 *      If *src points to a single space or newline, advance past it.
 * Parameters:
 *  src  -   Pointer to input pointer; may be advanced by one.
 */
static void skip_one_space_or_newline(const char **src) {   /* {{{ */
    if (**src == ' ' || **src == '\n')
      (*src)++;
}
/* }}} */

/**
 * processnested --
 *      Process inline (nested) tokens from string `s` and write formatted output to `fd`.
 *
 * Parameters:
 *  fd  -   File descriptor
 *  s   -   String to parse
 */
static void processnested(int fd, const char *str) {             /* {{{ */
    const char *p = str;
    char tok[512];                                      /* Temporary token buffer capacity. */

    while (*p) {
        switch (*p) {
        case '*':                                       /* bold -> .Sy %s\n */
            p++;                                        /* eat '*' */
            read_until(&p, '*', tok, sizeof(tok));
            append_to_fd(fd, "\n.Sy %s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '_':                                       /* italic -> .Em %s\n */
            p++;
            read_until(&p, '_', tok, sizeof(tok));
            append_to_fd(fd, "\n.Em %s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '`':                                       /* inline literal -> .Li %s\n */
            p++;
            read_until(&p, '`', tok, sizeof(tok));
            append_to_fd(fd, "\n.Li %s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '^':                                       /* reference -> .Xr %s\n */
            p++;
            read_until(&p, '^', tok, sizeof(tok));
            append_to_fd(fd, "\n.Xr %s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '\\':                                      /* escape: \x\ -> x (or consume next char if present) */
            p++;                                        /* eat backslash */
            if (*p && *p != '\\') {
                /* copy single character up to buffer limit */
                tok[0] = *p;
                tok[1] = '\0';
                p++; /* consume escaped char */
                if (*p == '\\') p++;                    /* eat backslash if found */
                append_to_fd(fd, "%s", tok);
            } else if (*p == '\\') {
                /* literal backslash sequence "\\": output single backslash and consume */
                append_to_fd(fd, "\\");
                p++;
            } else {
                /* dangling backslash at end: output it */
                append_to_fd(fd, "\\");
            }
            break;

        default:
            /* regular character: write single char */
            append_to_fd(fd, "%c", *p);
            p++;
            break;
        } /* switch */
    } /* while */
}
/* }}} */

/*
 * processline --
 *      This process the current line from the file and handles
 *      line-level constructs (leading tokens, block state--codeblock,
 *      listblock, etc.).
 *
 * Parameters:
 *  str -   string to search
 */
static void processline(char *str) { /*{{{*/
    int c;                                                /* Current character */
    c = *str;                                             /* Start at the beginning of the string */
    int fd = filedescriptors[1];                          /* The output file */

    switch (c) {
      case '\n':                                          // Newlines are replaced with a break.
        dprintf(fd, ".Pp%s", str);
        break;

      case 'a':                                           // Look for the string 'author:'
        if(cimemcmp(str, "author:", 7) == 0) {
          str += 7;                                       /* Eat the `author:` string. */
          dprintf(fd, ".Au%s", str);
          break;
        }

      case 'd':                                           // Date
        if(cimemcmp(str, "date:", 5) == 0) {
          str += 5;                                       /* Eat the `date:` string */
          dprintf(fd, ".Dd%s", str);
          break;
        }

      case 't':                                           // Look for the string 'title:'
        if(cimemcmp(str, "title:", 6) == 0) {
          str += 6;                                       /* Eat the `title:` string. */
          dprintf(fd, ".Dt%s.Os\n", str);
          break;
        }

      case '#':                                           // Section break (heading)
        dprintf(fd, ".Sh%s", ++str);
        if(cimemcmp(str, " NAME", 5) == 0) {              /* If we've found a "NAME" heading, we can assume
                                                             the section looks something like:
                                                                  # NAME
                                                                  ProjectName -- Brief Decription
                                                             so we set some flags for the default
                                                             condition of this case statment to set
                                                             the .Nm and .Nd mdoc macros.  */
          nameflag = 1;
        }
        break;

      case '[':                                           // Start of an optional argument
                                                          // EG: to process the "[-abc]"
        str++;                                            /* Eat the bracket */
        dprintf(fd, ".Op ");
        if (*str == '-') {
          dprintf(fd, "Fl ");
          do {                                            /* Print the chars until space char */
            ++str;                                        /* eat the dash */
            if (*str != ' ')
              dprintf(fd, "%c", *str);
          } while (*str != ' ');
          if (*str == ' ') {                              /* If we've found a space, this means we've found an
                                                             optional argument. */
            dprintf(fd, " Ar ");
            do {                                          /* Print the chars until we find the closing bracket */
              ++str;                                      /* eat the space */
              if (*str != ']')
                dprintf(fd, "%c", *str);
            } while (*str != ']');
          }
          ++str;                                          /* Eat the last bracket */
        } else {                                          /* Assume this is just a plain optional arguemnt */
          dprintf(fd, "Ar ");
          do {                                            /* Print the chars until we find the closing bracket */
            if (*str != ']')
              dprintf(fd, "%c", *str);
            ++str;                                        /* Eat the closing bracket */
          } while (*str != ']');
        }

        dprintf(fd, "\n");
        break;

      case '-':                                           // A list item or a single dash is a list terminator
                                                          // EG: "-f" or "-f file" or just "-"
        if(cimemcmp("-->", str, 3) == 0) {                /* First check if this is the end of a comment block */
          commentflag = 0;
          break;
        }
        ++str;                                            /* eat the dash */
        if(listblock == 0) {                              /* Check to see if the `listblock` flag has been set.
                                                             if it hasn't, create the list block and set the flag. */
          dprintf(fd, ".Bl -tag -width Ds\n");
          listblock = 1;
        }

        if (listblock == 1 && *str != '\n') {             /* If the listblock flag has been set, and the next char
                                                             is NOT a newline, this is just a list item. */
          dprintf(fd, ".It");                             /* Add a 'list item' macro */
          if (*str >=65 && *str <= 122)                   /* if the next item is (A-Za-z) char. */
            dprintf(fd, " Fl ");                          /* Add a 'flag' macro. */

          dprintf(fd, "%c", *str);                        /* Print the flag. */
          ++str;

          if (*str == ' ') {                              /* if we find a space after the flag, this is an argument
                                                             EG: "-f argument"
                                                                    ^           */
            dprintf(fd, " Ar%s", str);                    /* Print the 'argument' macro and the string. */
          } else {
            dprintf(fd, "%s", str);                       /* else just print the line. */
          }
          ++str;
        }

        if (*str == '\n' && listblock == 1) {             /* However, if the line was only a dash and the listblock
                                                             is set then we need to close the item list. */
          dprintf(fd, ".El\n");
          listblock = 0;
        }
        break;

      case '~':                                           // An alternate list terminator
        dprintf(fd, ".El\n");
        break;

      case '<':                                           // The start of a `no format` section (this is also the
                                                          // symbol used in vim's docformat).
        if (cimemcmp("<!--", str, 4) == 0) {              /* Start of a comment block */
          commentflag = 1;
          break;
        }
        dprintf(fd, ".Bd -literal -offset indent\n");
        stripwhitespace = 0;                              /* Disable stripwhitespace. */
        codeblock = 1;                                    /* Set the `codeblock` flag */
        break;

      case '>':                                           // The end of a `no format` section
        dprintf(fd, ".Ed\n");
        stripwhitespace = 1;
        codeblock = 0;
        break;

      case '`':                                           // Code block
                                                          //   In markdown, READMEs, forum posts, etc.
                                                          //   codeblocks are defined with three (3) backticks.
        if(cimemcmp(str, "```", 3) == 0) {
          if(codeblock == 0) {                            /* Check to see if the `codeblock` flag has been set. */
            dprintf(fd, ".Bd -literal -offset indent\n");
            stripwhitespace = 0;                          /* Disable stripwhitespace. */
            codeblock = 1;
          } else if (codeblock == 1) {
            dprintf(fd, ".Ed\n");
            stripwhitespace = 1;
            codeblock = 0;
          }
          break;
        }

      default:
        if(commentflag == 1) {
          break;
        }
        if(stripwhitespace) {
          while (isspace((unsigned char)*str)) {
            ++str;
          }
        }

        if(nameflag == 1) {                               /* If we are supposed to process a name... */
          dprintf(fd, ".Nm ");
          do {                                            /* Print this chars until NOT a dash */
            if (*str != '-')
              dprintf(fd, "%c", *str);
            ++str;                                        /* Eat the char */
            if (*str == '-') {                            /* If we've encounted a dash, check for a doubledash. */
              if(cimemcmp(str, "--", 2) == 0) {           /* double dashes signifies a `namedescription`. */
                str += 2;                                 /* Eat the `--` string */
                dprintf(fd, "\n.Nd");
                do {
                  if (*str != '\n' || \
                      *str != ' ')
                    dprintf(fd, "%c", *str);
                  ++str;
                } while (*str != '\n');
              }
            }
          } while (*str != '\n');
          nameflag = 0;                                   /* turn off the `nameflag`. */
        }
        // Move to the next character
        c++;

        if (codeblock == 0) {                             /* If we're not in a clode block... */
          processnested(fd, str);                             /* Check the rest of the string for nested elements. */
        } else {                                          /* otherwise just print the line. */
          dprintf(fd, "%s", str);
        }
        break;
    }
}
/*}}}*/

/*
 * readline --
 *      Reads a line from file descriptor and stores the string in buf.
 *
 * ARGS
 *  fd       -   file descriptor
 *  buf      -   where to store the string
 *  nbytes   -   How may bytes to read.
 *
 * RETURNS
 *  int
 */
static int readline(int fd, char *buf, int nbytes) { /*{{{*/
  // Reads a line from file descriptor and stores the string in buf.
  int numread = 0;
  int returnval;

  while (numread < nbytes - 1) {
    returnval = read(fd, buf + numread, 1);
    if ((returnval == -1) && (errno == EINTR))
      continue;
    if ((returnval == 0) && (numread == 0))
      return 0;
    if (returnval == 0)
      break;
    if (returnval == -1)
      return -1;
    numread++;
    if (buf[numread - 1] == '\n') {
      buf[numread] = '\0';
      return numread;
    }
  }
  errno = EINVAL;
  return -1;
}
/*}}}*/

//------------------------------------------------------*- C -*------
// Main
//-------------------------------------------------------------------
int main(int argc, char *argv[]) {

  if (argc != 3) {
    printusage(argv[0]);
    fprintf(stderr, "**** %s version: %s\n", argv[0], program_version);
    AbortTranslation(abortInvalidCommandLineArgs);
  }

  if ((filedescriptors[0] = open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr, "Failed to open file %s:%s\n", argv[1], strerror(errno));
    return 1;
  }

  if ((filedescriptors[1] = open(argv[2], W_FLAGS, W_PERMS)) == -1) {
    fprintf(stderr, "Failed to open file %s:%s\n", argv[2], strerror(errno));
    return 1;
  }

  processfd(filedescriptors);

  return 0;
} ///:~
