//===---------------------------------------------------*- C -*---===
// File Last Updated: 02.02.26 19:24:33
//
//: md2mdoc
//
// DATE: January 21 2024
// BY  : John Kaul [john.kaul@outlook.com]
//
// DESCRIPTION
// This is a project to convert simple markdown to mdoc (man page)
// format. This program defaults to `stdout` unless -o flag is used.
//
// OPTIONS
//  -o outfile
//      A file to write.
//
// KEY:
// ------------------------------------------------------------------
// #           ->  .Sh     : section headers
// blank line  ->  .Pp     : Blank Line
// @           ->  .Nm     : Project Name
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

//:~  #include "version.h"

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
#define SECTION ".Sh"
#define BOLD ".Sy "
#define ITALIC ".Em "
#define INLINE ".Li "
#define REFERENCE ".Xr "
#define OPTIONAL ".Op "
#define FLAG "Fl "
#define ARGUMENT " Ar "
#define ITEM ".It"
#define AUTHOR ".Au"
#define DATE ".Dd"
#define TITLE ".Dt"

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------
static void *processfd(void *arg);                      /* Open the FD and send to processline(); */
static void processline(FILE *out, char *str);          /* Process one line of text at a time
                                                           from the  input file. */
static void processnested(FILE *out, const char *str);
static int readline(FILE *fd, char *buf, int nbytes);   /* Read a line of text from file. */
static int cimemcmp(const void *s1, const void *s2, size_t n); /* case independent memory regon compare */
static int read_until(const char **src, char delim, char *dst, size_t dstcap);
static void skip_one_space_or_newline(const char **src);

//-------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------
FILE *filedescriptors[2];                               /* An array to hold open file descriptors. */
unsigned int stripwhitespace = 1;                       /* Used to pause/stop stripping whitespace */
unsigned int codeblock       = 0;                       /* Used for codeblocks. */
unsigned int listblock       = 0;                       /* Used for list blocks. */
unsigned int nameflag        = 0;                       /* Set when this program find the string: "# NAME". */
unsigned int commentflag     = 0;                       /* Used for comment blocks (HTML style <!-- comment --> */

/**
 * printussage --
 *      Prints the usage string to enduser (incase they give the wrong
 *      arguments.
 */
static void printusage(char *str) {
  fprintf(stderr, "**** Usage: %s <markdownfile>\n", str);
  fprintf(stderr, "**** Usage: %s <markdownfile> -o <outfile>\n", str);
}

/**
 * processfd --
 *      Read lines from a given filedescritor and pass them to the
 *      `processline` function.
 * Parameters:
 *  arg     -   a file stream (FILE *)
 *
 * Returns:
 * NULL
 */
static void *processfd(void *arg) {
  char buff[LINE_MAX];
  ssize_t nbytes;

  for (;;) {
    if ((nbytes = readline(arg, buff, LINE_MAX)) < 0)
      break;
    processline(filedescriptors[1], buff);
  }
  return NULL;
}

/**
 * cimemcmp --
 *      Preform a case independent memory region compare.
 *
 * Compare up to n bytes from s1 and s2 ignoring ASCII case differences
 * for alphabetic characters (works by folding space bit).
 *
 * Parameters:
 *  s1   -  pointer to first memory region
 *  s2   -  pointer to second memory region
 *  n    -  number of bytes to compare
 *
 * Returns: <0 if s1 < s2, 0 if equal, >0 if s1 > s2 (same semantics as memcmp).
 * Notes: Operates on raw bytes; caller should ensure buffers are at least n bytes.
 */
static int cimemcmp(const void *s1, const void *s2, size_t n) {
  if (n != 0) {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if ((*p1++ & ~' ') != (*p2++ & ~' '))
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return (0);
}

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
static int read_until(const char **src, char delim, char *dst, size_t dstcap) {
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

/**
 * Sanitize --
 *      Allow only "white-listed" chars in string.
 */
static void sanitize(char *user_data, size_t n) {
  if(!user_data) return;
  static char ok_chars[] = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "1234567890"
    " \f\t\n"
    "_"
    ;
  const char *end = user_data + n;
  for (user_data += strspn(user_data, ok_chars); user_data != end;
       user_data += strspn(user_data, ok_chars)) {
    *user_data = ' ';
  }
}

/**
 * skip_one_space_or_newline --
 *      If *src points to a single space or newline, advance past it.
 * Parameters:
 *  src  -   Pointer to input pointer; may be advanced by one.
 */
static void skip_one_space_or_newline(const char **src) {
    if (**src == ' ' || **src == '\n')
      (*src)++;
}

/**
 * processnested --
 *      Process inline (nested) tokens from string `s` and write formatted output to `fd`.
 *
 * Parameters:
 *  fd  -   File descriptor
 *  s   -   String to parse
 */
static void processnested(FILE *out, const char *str) {
    const char *p = str;
    char tok[512];                                      /* Temporary token buffer capacity. */
    unsigned cntr = 0;

    while (*p) {
        switch (*p) {
          case '@':                                     /* UNDOCUMENTED - Shortcut for '.Nm' (project name) */
            p++;
            if (cntr >= 1) fprintf(out, "\n");
            fprintf(out, ".Nm\n");
            read_until(&p, ' ', tok, sizeof(tok));
            break;
        case '*':                                       /* bold -> .Sy %s\n */
            p++;                                        /* eat '*' */
            read_until(&p, '*', tok, sizeof(tok));
            if (cntr >= 1) fprintf(out, "\n");
            fprintf(out, BOLD "%s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '_':                                       /* italic -> .Em %s\n */
            p++;
            read_until(&p, '_', tok, sizeof(tok));
            if (cntr >= 1) fprintf(out, "\n");
            fprintf(out, ITALIC "%s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '`':                                       /* inline literal -> .Li %s\n */
            p++;
            read_until(&p, '`', tok, sizeof(tok));
            if (cntr >= 1) fprintf(out, "\n");
            fprintf(out, INLINE "%s\n", tok);
            skip_one_space_or_newline(&p);
            break;

        case '^':                                       /* reference -> .Xr %s\n */
            p++;
            read_until(&p, '^', tok, sizeof(tok));
            sanitize(tok, strlen(tok));
            if (cntr >= 1) fprintf(out, "\n");
            fprintf(out, REFERENCE "%s", tok);
            while (*p == ' ' || \
                *p == ',' || \
                *p == '.')
              fprintf(out, "%c", *p++);
            if(*p != '\n') fprintf(out, "\n");
            continue;

        case '\\':                                      /* escape: \x\ -> x (or consume next char if present) */
            p++;                                        /* eat backslash */
            if (*p && *p != '\\') {
                /* copy single character up to buffer limit */
                tok[0] = *p;
                tok[1] = '\0';
                p++; /* consume escaped char */
                if (*p == '\\') p++;                    /* eat backslash if found */
                fprintf(out, "%s", tok);
            } else if (*p == '\\') {
                /* literal backslash sequence "\\": output single backslash and consume */
                fprintf(out, "\\");
                p++;
            } else {
                /* dangling backslash at end: output it */
                fprintf(out, "\\");
            }
            break;

        default:
            /* regular character: write single char */
            fprintf(out, "%c", *p);
            p++;
            break;
        } /* switch */
        cntr++;
    } /* while */
}

/**
 * processline --
 *      This process the current line from the file and handles
 *      line-level constructs (leading tokens, block state--codeblock,
 *      listblock, etc.).
 *
 * Parameters:
 *  str -   string to search
 */
static void processline(FILE *out, char *str) {
    int c;                                                /* Current character */
    c = *str;                                             /* Start at the beginning of the string */

    switch (c) {
      stripwhitespace = 0;
      case '\n':                                          // Newlines are replaced with a break.
        fprintf(out, ".Pp%s", str);
        break;

      case 'a':                                           // Look for the string 'author:'
        if(cimemcmp(str, "author:", 7) == 0) {
          str += 7;                                       /* Eat the `author:` string. */
          fprintf(out, AUTHOR "%s", str);
          break;
        }

      case 'd':                                           // Date
        if(cimemcmp(str, "date:", 5) == 0) {
          str += 5;                                       /* Eat the `date:` string */
          fprintf(out, DATE "%s", str);
          break;
        }

      case 't':                                           // Look for the string 'title:'
        if(cimemcmp(str, "title:", 6) == 0) {
          str += 6;                                       /* Eat the `title:` string. */
          fprintf(out, TITLE "%s.Os\n", str);
          break;
        }

      case '#':                                           // Section break (heading)
        if (*str++ == '#') {                              /* eat all the leading hashs. */
          while (isalpha(*str) > 0) str++;
        }
        sanitize(str, strlen(str));                       /* sanitize rest of string of all hashs */
        fprintf(out, SECTION "%s", str);
        if(cimemcmp(str, " NAME", 5) == 0) {              /* If we've found a "NAME" heading, we can
                                                             assume the section looks something like:
                                                                  # NAME
                                                                  ProjectName -- Brief Decription
                                                             so we set some flags for the default
                                                             condition of this case statment to set
                                                             the .Nm and .Nd mdoc macros.  */
          nameflag = 1;
          break;
        }
        break;

      case '[':                                           // Start of an optional argument
                                                          // EG: to process the "[-abc]"
        str++;                                            /* Eat the bracket */
        fprintf(out, OPTIONAL);
        if (*str == '-') {
          str++;
          fprintf(out, FLAG);
          do {                                            /* Print the chars until space char */
            if (*str != ' ')
              fprintf(out, "%c", *str);
            ++str;                                        /* eat the dash */
          } while (*str != ' '  && *str != ']');
          if (*str == ' ') {                              /* If we've found a space, this means we've found an
                                                             optional argument. */
            fprintf(out, ARGUMENT);
            do {                                          /* Print the chars until we find the closing bracket */
              ++str;                                      /* eat the space */
              if (*str != ']')
                fprintf(out, "%c", *str);
            } while (*str != ']');
          }
          ++str;                                          /* Eat the last bracket */
        } else {                                          /* Assume this is just a plain optional arguemnt */
          fprintf(out, ARGUMENT);
          do {                                            /* Print the chars until we find the closing bracket */
            if (*str != ']')
              fprintf(out, "%c", *str);
            ++str;                                        /* Eat the closing bracket */
          } while (*str != ']');
        }

        fprintf(out, "\n");
        break;

      case '-':                                           // A list item or a single dash is a list terminator
                                                          // EG: "-f" or "-f file" or just "-"
        if(cimemcmp(str, "-->", 3) == 0) {                /* First check if this is the end of a comment block */
          commentflag = 0;
          break;
        }
        ++str;                                            /* eat the dash */
        if(listblock == 0) {                              /* Check to see if the `listblock` flag has been set.
                                                             if it hasn't, create the list block and set the flag. */
          fprintf(out, ".Bl -tag -width Ds\n");
          listblock = 1;
        }

        if (listblock == 1 && *str != '\n') {             /* If the listblock flag has been set, and the next char
                                                             is NOT a newline, this is just a list item. */
          fprintf(out, ITEM);                             /* Add a 'list item' macro */
          if (*str >=65 && *str <= 122)                   /* if the next item is (A-Za-z) char. */
            fprintf(out, " Fl ");                          /* Add a 'flag' macro. */

          fprintf(out, "%c", *str);                        /* Print the flag. */
          ++str;

          if (*str == ' ') {                              /* if we find a space after the flag, this is an argument
                                                             EG: "-f argument"
                                                                    ^           */
            fprintf(out, " Ar%s", str);                    /* Print the 'argument' macro and the string. */
          } else {
            fprintf(out, "%s", str);                       /* else just print the line. */
          }
          ++str;
        }

        if (*str == '\n' && listblock == 1) {             /* However, if the line was only a dash and the listblock
                                                             is set then we need to close the item list. */
          fprintf(out, ".El\n");
          listblock = 0;
        }
        break;

      case '~':                                           // An alternate list terminator
        fprintf(out, ".El\n");
        break;

      case '<':                                           // The start of a `no format` section (this is also the
                                                          // symbol used in vim's docformat).
        if (cimemcmp(str, "<!--", 4) == 0) {              /* Start of a comment block */
          commentflag = 1;
          break;
        }
        fprintf(out, ".Bd -literal -offset indent\n");
        stripwhitespace = 0;                              /* Disable stripwhitespace. */
        codeblock = 1;                                    /* Set the `codeblock` flag */
        break;

      case '>':                                           // The end of a `no format` section
        fprintf(out, ".Ed\n");
        stripwhitespace = 1;
        codeblock = 0;
        break;

      case '`':                                           // Code block
                                                          //   In markdown, READMEs, forum posts, etc.
                                                          //   codeblocks are defined with three (3) backticks.
        if(cimemcmp(str, "```", 3) == 0) {
          if(codeblock == 0) {                            /* Check to see if the `codeblock` flag has been set. */
            fprintf(out, ".Bd -literal -offset indent\n");
            stripwhitespace = 0;                          /* Disable stripwhitespace. */
            codeblock = 1;
          } else if (codeblock == 1) {
            fprintf(out, ".Ed\n");
            stripwhitespace = 1;
            codeblock = 0;
          }
          break;
        }

      default:
        if(commentflag == 1) {
          break;
        }
        if(stripwhitespace == 1) {
          while (isspace(*str) > 0) str++;
        }

        if(nameflag == 1) {                               /* If we are supposed to process a name... */
          fprintf(out, ".Nm ");
          do {                                            /* Print this chars until NOT a dash */
            if (*str != '-')
              fprintf(out, "%c", *str);
            ++str;                                        /* Eat the char */
            if (*str == '-') {                            /* If we've encounted a dash, check for a doubledash. */
              if(cimemcmp(str, "--", 2) == 0) {           /* double dashes signifies a `namedescription`. */
                str += 2;                                 /* Eat the `--` string */
                fprintf(out, "\n.Nd");
                do {
                  if (*str != '\n' || \
                      *str != ' ')
                    fprintf(out, "%c", *str);
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
          processnested(out, str);                         /* Check the rest of the string for nested elements. */
        } else {                                          /* otherwise just print the line. */
          fprintf(out, "%s", str);
        }
        break;
    }
}

/**
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
static int readline(FILE *in, char *buf, int nbytes) {
     int linelen;
     while ((linelen = getline(&buf, (size_t *)&nbytes, in)) > 0)
             processline(filedescriptors[1], buf);
    return linelen;
}

//------------------------------------------------------*- C -*------
// Main
//-------------------------------------------------------------------
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printusage(argv[0]);
    return 1;
  }

  // -Default output is `stdout` unless specified otherwise.
  filedescriptors[1] = stdout;

  // -Parse the command line options.
  for (int i = 0; i < argc; i++) {
    if (argv[i] && strlen(argv[i]) > 1) {
      if (argv[i][0] == '-' && argv[i][1] == 'o') { filedescriptors[1] = fopen(argv[++i], "w"); }
      if (argv[i][0] != '-') { filedescriptors[0] = fopen(argv[i], "r"); }

      if (argv[i][0] == '|' || \
          argv[i][0] == '>') { break; }
    }
  }

  processfd(filedescriptors[0]);

  return 0;
} ///:~
