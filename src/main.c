//===---------------------------------------------------*- C -*---===
// File Last Updated: 09.24.24 20:33:55
//
//: md2mdoc
//
// DATE: January 21 2024
// BY  : John Kaul [john.kaul@outlook.com]
//
// DESCRIPTION
// This is a project to convert simple markdown to mdoc (man page)
// format.
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
const char program_version[] = "0.0.2";

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
int codeblock = 0;                                      /* Used for middle of codeblock. */

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
  /* proces command read from file descriptor */
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
  /* Prefom a case independent memory region compare.  */
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
  // This function will be passed the current line of text from the
  // input file and this function will search the string and print the
  // proper mdoc format strings to the output file.

  int c;                                                /* -current character */
  c = *cmd;                                             /* -Start at the beginning of the string */
  int fd = filedescriptors[1];                          /* -The output file */

  switch (c) {
    case '\n':                                          // Newlines are replaced with a break.
      dprintf(fd, ".Pp%s", cmd);
      break;
    case 'a':                                           // Look for the string 'author:'
      if(cimemcmp(cmd, "author:", 7) == 0) {
        cmd += 7;                                       /* -eat the `author:` string. */
        dprintf(fd, ".Au%s", cmd);
        break;
      }
    case 'd':                                           // date
      if(cimemcmp(cmd, "date:", 5) == 0) {
        cmd += 5;
        dprintf(fd, ".Dd%s", cmd);
        break;
      }
    case 't':                                           // Look for the string 'title:'
      if(cimemcmp(cmd, "title:", 6) == 0) {
        cmd += 6;                                       /* -eat the `title:` string. */
        dprintf(fd, ".Dt%s.Os\n", cmd);
        break;
      }
    case '#':                                           // section break (heading)
      if (cimemcmp(cmd, "# OPTIONS", 9) == 0) {
        dprintf(fd, ".Sh %s.Bl -tag -width Ds\n", ++cmd);
        } else {
          dprintf(fd, ".Sh %s", ++cmd);
        }
        break;
    case '[':                                           // Start of a list
        cmd++;
        if (*cmd == '\n')
          dprintf(fd, ".Bl -tag -width Ds\n");
        break;
    case ']':                                           // End of a list
        cmd++;
        if (*cmd == '\n')                               /* -However, if the line was only a r-bracket */
          dprintf(fd, ".El\n");                         /*  then we need to close the item list. */
        break;
    case '-':                                           // A list item / single dash is also a list terminator
      ++cmd;
      if (*cmd == '\n')                                 /* -However, if the line was only a dash */
        dprintf(fd, ".El\n");                           /*  than we need to close the item list. */
      else
        dprintf(fd, ".It Fl %s", cmd);
      break;
    case '~':                                           // A list terminator
      cmd = 0;                                          /* -not sure; *shoulder shrug* */
      dprintf(fd, ".El\n");
      break;
    case '<':                                           // The start of a `no format` section.
      dprintf(fd, ".Bd -literal -offset indent\n");
      stripwhitespace = 0;                              /* -Disable stripwhitespace. */
      break;
    case '>':                                           // The end of a `no format` section
      dprintf(fd, ".Ed\n");
      stripwhitespace = 1;
      break;
    case '*':                                           // Bold
        dprintf(fd, ".Sy ");
        do {                                            /* -print this chars until not star */
          ++cmd;                                        /* eat the star */
          if (*cmd != '*')
            dprintf(fd, "%c", *cmd);
        } while (*cmd != '*');
        ++cmd;                                          /* eat the last star */
//:~          dprintf(fd, "\n.Pp\n");
        dprintf(fd, "\n");
      break;
    case '_':                                           // Italic
        dprintf(fd, ".Em ");
        do {                                            /* -print this chars until not star */
          ++cmd;                                        /* eat the underscore */
          if (*cmd != '_')
            dprintf(fd, "%c", *cmd);
        } while (*cmd != '_');
        ++cmd;                                          /* eat the last underscore */
        dprintf(fd, "\n");
      break;
    case '^':                                           // Reference
        dprintf(fd, ".Sx ");
        do {                                            /* -print this chars until not star */
          ++cmd;                                        /* eat the underscore */
          if (*cmd != '^')
            dprintf(fd, "%c", *cmd);
        } while (*cmd != '^');
        ++cmd;                                          /* eat the last underscore */
        dprintf(fd, "\n");
      break;
    case '`':                                           // Code block -
                                                        //   In markdown, READMEs, forum posts, etc.
                                                        //   codeblocks are defined with three (3) graves.

      ++cmd;                                            /* -Eat the first grave */

      if (*cmd != '`')                                  /* -if we havent found another grave sym, put it back. */
        dprintf(fd, "`");

      if (*cmd == '`') {                                /* -If we have another grave symbol... */
        if(codeblock == 0) {                            /* -Check to see if the `codeblock` flag has been set. */
          dprintf(fd, ".Bd -literal -offset indent\n");
          stripwhitespace = 0;                          /* -Disable stripwhitespace. */
          codeblock = 1;
        } else if (codeblock == 1) {
          dprintf(fd, ".Ed\n");
          stripwhitespace = 1;
          codeblock = 0;
        }
        do                                              /* -Eat the graves until the newline char */
          ++cmd;
        while (*cmd != '\n');
        break;
      }
    default:
      if(stripwhitespace)
        while (isspace((unsigned char)*cmd))
          ++cmd;
      dprintf(fd, "%s", cmd);
      break;
  }
}
/*}}}*/

/*
 * readline --
 *  Reads a line from file descriptor and stores the string in buf.
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
//
// ARGS
//  argc            :   number of args
//  argv[]          :   array of arguments
//
// RETURN
//  int
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
