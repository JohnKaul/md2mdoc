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
