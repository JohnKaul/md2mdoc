//===---------------------------------------------------*- C -*---===
// File Last Updated: 02.02.24 19:29:57
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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------
// Constants Declarations
//-------------------------------------------------------------------
const char program_version[] = "0.0.1";

#define W_FLAGS (O_WRONLY | O_CREAT)                    /* Write flags for file output */
#define W_PERMS (S_IRUSR | S_IWUSR)                     /* Write permissions for file output */

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------
void *processfd(void *arg);                             /* Open the FD and send to find_entries(); */
void monitorfd(int fd[], int numfds);                   /* Create thread to monitor fd's. */
void docommand(char *cmd);                              /* Process one line of text at a time
                                                           from the  input file. */
int readline(int fd, char *buf, int nbytes);            /* read a line of text from file. */

//-------------------------------------------------------------------
// Global Variables
//-------------------------------------------------------------------
char *curfile;                                          /* Current input file name */
int filedescriptors[2];                                 /* Make this tool multithreaded; create
                                                           an array to hold open file descriptors for
                                                           passing to an threaded operation. */
/*
 * TabAbortCode -- Enums for standard errors.
 */
enum TAbortCode { /*{{{*/ abortInvalidCommandLineArgs = -1,
  abortRuntimeError = -2, abortUnimplementedFeature = -3 }; /*}}}*/

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

/*
 * monitorfd --
 *      The `monitorfd` function uses thread to monitor an array of file
 *      descriptors. If `processfd` causes the calling thread to block for
 *      some reason the thread runtime system schedules another runnable
 *      thread. In this way, processing and reading are overlapped in a
 *      natural way. In contrast, blocking of `processfd` in the
 *      single-threaded implementation causes the entire process to block.
 *
 *      If `monitorfd` fails to create thread `i`, it sets the corresponding
 *      thread ID to itself to signify that creation failed. The last loop
 *      uses `pthread_join`, to wait until all threads have completed.
 */
void monitorfd(int fd[], int numfds) { /*{{{*/
  /* create thread to monitor fd's */
  int error, i;
  pthread_t *tid;

  //:~    fprintf(stderr, ".Dd $Mdocdate: today\n.Dt TEST\n.Os\n");

  if ((tid = (pthread_t *)calloc(numfds, sizeof(pthread_t))) == NULL) {
    perror("Failed to allocate space for thread IDs");
    return;
  }
  for (i = 0; i < numfds; ++i) /* create a thread for each file descriptor */
    if ((error = pthread_create(tid + i, NULL, processfd, (fd + i)))) {
      fprintf(stderr, "Failed to create thread %d: %s\n", i, strerror(error));
      tid[i] = pthread_self();
    }
  for (i = 0; i < numfds; ++i) {
    if (pthread_equal(pthread_self(), tid[i]))
      continue;
    if ((error = pthread_join(tid[i], NULL)))
      fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(error));
  }
  free(tid);
  return;
} //:~
/*}}}*/

/* processfd --
 *      Read lines from a given filedescritor and pass them to the
 *      `docommand` function.
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
    docommand(buff /*, (fd + 1), nbytes */);
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
 * docommand --
 *      This process the current line from the file.
 */
void docommand(char *cmd) { /*{{{*/
  // This function will be passed the current line of text from the
  // input file and this function will search the string and print the
  // proper mdoc format strings to the output file.

  int c;                                    /* current character */
  c = *cmd;                                 /* Start at the beginning of the string */
  int fd = filedescriptors[1];              /* The output file */

  switch (c) {
  case '\n':                                /* newlines are replaced with a break. */
    dprintf(fd, ".Pp%s", cmd);
    break;
  case '@':                                 /* author */
    ++cmd;                                  /* eat the @ sym. */
    dprintf(fd, ".Au%s", cmd);
    break;
  case '&':                                 /* date */
    ++cmd;
    dprintf(fd, ".Dd%s", cmd);
    break;
  case '%':                                 /* document tag */
    ++cmd;
    dprintf(fd, ".Dt%s.Os\n", cmd);
    break;
  case '#':                                 /* section break */
    if (cimemcmp(cmd, "# OPTIONS", 9) == 0) {
      dprintf(fd, ".Sh %s.Bl -tag -width Ds\n", ++cmd);
//     } else if (memcmp(cmd, "# LIST", 6) == 0) {
//       dprintf(fd, ".Sh %s.Bl -tag -width Ds\n", ++cmd);
//       break;
// //:~        while (*cmd != '\n') ++cmd;
     } else {
      dprintf(fd, ".Sh %s", ++cmd);
    }
    break;
  case '-':                                 /* a list item */
    ++cmd;
    if (*cmd == '\n') {                     // However, if the line was only a dash
      dprintf(fd, ".El\n");                 // then we need to close the item list.
    } else {
      dprintf(fd, ".It Fl %s", cmd);
    }
    break;
  case '~':
    cmd = 0;                                /* not sure; *shoulder shrug* */
    dprintf(fd, ".El\n");
    break;
  case '<':                                 /* The start of a `no format` section. */
//:~      dprintf(fd, ".in +10\n.nf\n");
    dprintf(fd, ".Bd -literal -offset indent\n");
    break;
  case '>':                                 /* the end of a `no format` section */
//:~      dprintf(fd, ".fi\n");
    dprintf(fd, ".Ed\n");
    break;
  default:
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

  monitorfd(filedescriptors, 2);

  return 0;
} ///:~
