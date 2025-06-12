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

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __FreeBSD__                                         /* FreeBSD needs the following includes for
                                                           the S_IRUSR / S_IWUSR macros to work */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#endif

//-------------------------------------------------------------------
// Constants Declarations
//-------------------------------------------------------------------
const char program_version[] = "0.0.3";

#define W_FLAGS (O_WRONLY | O_CREAT)                    /* Write flags for file output */
#define W_PERMS (S_IRUSR | S_IWUSR)                     /* Write permissions for file output */

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------
void *processfd(void *arg);                             /* Open the FD and send to processline(); */
void processline(char *cmd);                            /* Process one line of text at a time
                                                           from the  input file. */
int readline(int fd, char *buf, int nbytes);            /* Read a line of text from file. */

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
void AbortTranslation(enum TAbortCode ac) { /*{{{*/
  fprintf(stderr, "**** Fatal translation error: %s\n", abortMsg[-ac]);
  exit(ac);
} /*}}}*/

/*
 * printussage --
 *      Prints the usage string to enduser (incase they give the wrong
 *      arguments.
 */
void printusage(char *str) { /*{{{*/
  fprintf(stderr, "**** Usage: %s <markdownfile> <mdocfiletowrite>\n", str);
}
/*}}}*/

/* processfd --
 *      Read lines from a given filedescritor and pass them to the
 *      `processline` function.
 */
void *processfd(void *arg) { /*{{{*/
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
int cimemcmp(const void *s1, const void *s2, size_t n) { /*{{{*/
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

/*
 * processline --
 *      This process the current line from the file and preforms
 *      simple character substion.
 */
void processline(char *cmd) { /*{{{*/
  int c;                                                /* Current character */
  c = *cmd;                                             /* Start at the beginning of the string */
  int fd = filedescriptors[1];                          /* The output file */

  switch (c) {
    case '\n':                                          // Newlines are replaced with a break.
      dprintf(fd, ".Pp%s", cmd);
      break;
    case 'a':                                           // Look for the string 'author:'
      if(cimemcmp(cmd, "author:", 7) == 0) {
        cmd += 7;                                       /* Eat the `author:` string. */
        dprintf(fd, ".Au%s", cmd);
        break;
      }
    case 'd':                                           // Date
      if(cimemcmp(cmd, "date:", 5) == 0) {
        cmd += 5;                                       /* Eat the `date:` string */
        dprintf(fd, ".Dd%s", cmd);
        break;
      }
    case 't':                                           // Look for the string 'title:'
      if(cimemcmp(cmd, "title:", 6) == 0) {
        cmd += 6;                                       /* Eat the `title:` string. */
        dprintf(fd, ".Dt%s.Os\n", cmd);
        break;
      }
    case '#':                                           // Section break (heading)
      dprintf(fd, ".Sh%s", ++cmd);
      if(cimemcmp(cmd, " NAME", 5) == 0) {              /* If we've found a "NAME" heading, we can assume
                                                         * the section looks something like:
                                                         *      # NAME
                                                         *      ProjectName -- Brief Decription
                                                         * so we set some flags for the default
                                                         * condition of this case statment to set
                                                         * the .Nm and .Nd mdoc macros.  */
        nameflag = 1;
      }
      break;
    case '[':                                           // Start of an optional argument
      cmd++;                                            /* Eat the bracket */
      dprintf(fd, ".Op ");
      if (*cmd == '-') {
        dprintf(fd, "Fl ");
        do {                                            /* Print the chars until space char */
          ++cmd;                                        /* eat the dash */
          if (*cmd != ' ')
            dprintf(fd, "%c", *cmd);
        } while (*cmd != ' ');
        if (*cmd == ' ') {                              /* If we've found a space, this means we've found an optional argument. */
          dprintf(fd, " Ar ");
          do {                                          /* Print the chars until we find the closing bracket */
            ++cmd;                                      /* eat the space */
            if (*cmd != ']')
              dprintf(fd, "%c", *cmd);
          } while (*cmd != ']');
        }
        ++cmd;                                          /* Eat the last bracket */
      } else {                                          /* Now we have to assume this is just a plain optional arguemnt */
        dprintf(fd, " Ar ");
        do {                                            /* Print the chars until we find the closing bracket */
          if (*cmd != ']')
            dprintf(fd, "%c", *cmd);
          ++cmd;                                        /* Eat the closing bracket */
        } while (*cmd != ']');
      }

      dprintf(fd, "\n");
      break;
    case '-':                                           // A list item or a single dash is a list terminator
      if(cimemcmp("-->", cmd, 3) == 0) {                /* end of a comment block */
        commentflag = 0;
        break;
      }
      ++cmd;                                            /* eat the dash */
      if(listblock == 0) {                              /* Check to see if the `listblock` flag has been set.
                                                           if it hasn't, create the list block and set the flag.  */
        dprintf(fd, ".Bl -tag -width Ds\n");
        listblock = 1;
      }
      if (listblock == 1 && *cmd != '\n')               /* If the listblock flag has been set, and the next char is NOT
                                                           a newline, this is just a list item. */
        dprintf(fd, ".It Fl %s", cmd);
      if (*cmd == '\n' && listblock == 1) {             /* However, if the line was only a dash
                                                           than we need to close the item list. */
        dprintf(fd, ".El\n");
        listblock = 0;
      }
      break;
    case '~':                                           // An alternate list terminator
      dprintf(fd, ".El\n");
      break;
    case '<':                                           // The start of a `no format` section (this is also the
                                                        // symbol used in vim's docformat).
      if (cimemcmp("<!--", cmd, 4) == 0) {              /* Start of a comment block */
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
    case '*':                                           // Bold
      dprintf(fd, ".Sy ");
      do {                                              /* Print the chars until NOT star */
        ++cmd;                                          /* Eat the star */
        if (*cmd != '*')
          dprintf(fd, "%c", *cmd);
      } while (*cmd != '*');
      ++cmd;                                            /* Eat the last star */
      dprintf(fd, "\n");
      break;
    case '_':                                           // Italic
      dprintf(fd, ".Em ");
      do {                                              /* Print this chars until NOT underscore */
        ++cmd;                                          /* Eat the underscore */
        if (*cmd != '_')
          dprintf(fd, "%c", *cmd);
      } while (*cmd != '_');
      ++cmd;                                            /* Eat the last underscore */
      dprintf(fd, "\n");
      break;
    case '^':                                           // Reference
      dprintf(fd, ".Sx ");
      do {                                              /* Print this chars until NOT carret */
        ++cmd;                                          /* Eat the carret */
        if (*cmd != '^')
          dprintf(fd, "%c", *cmd);
      } while (*cmd != '^');
      ++cmd;                                            /* eat the last carret */
      dprintf(fd, "\n");
      break;
    case '`':                                           // Code block
                                                        //   In markdown, READMEs, forum posts, etc.
                                                        //   codeblocks are defined with three (3) backticks.

      ++cmd;                                            /* Eat the first backtick */

      if (*cmd != '`')                                  /* If we havent found another backtick sym, put it back. */
        dprintf(fd, "`");

      if (*cmd == '`') {                                /* If we have another backtick symbol... */
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
        while (isspace((unsigned char)*cmd)) {
          ++cmd;
        }
      }

      if(nameflag == 1) {                               /* If we are supposed to process a name... */
        dprintf(fd, ".Nm ");
        do {                                            /* Print this chars until NOT a dash */
          if (*cmd != '-')
            dprintf(fd, "%c", *cmd);
          ++cmd;                                        /* Eat the char */
          if (*cmd == '-') {                            /* If we've encounted a dash, check for a doubledash. */
            if(cimemcmp(cmd, "--", 2) == 0) {           /* double dashes signifies a `namedescription`. */
              cmd += 2;                                 /* Eat the `--` string */
              dprintf(fd, "\n.Nd");
              do {
                if (*cmd != '\n' || \
                    *cmd != ' ')
                  dprintf(fd, "%c", *cmd);
                ++cmd;
              } while (*cmd != '\n');
            }
          }
        } while (*cmd != '\n');
        nameflag = 0;                                   /* turn off the `nameflag`. */
      }

      dprintf(fd, "%s", cmd);
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
int readline(int fd, char *buf, int nbytes) { /*{{{*/
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
