#ifndef __ASTSPHINX_H
#define __ASTSPHINX_H 1

enum e_reqtype
{
  REQTYPE_GRAMMAR,
  REQTYPE_START,
  REQTYPE_DATA,
  REQTYPE_FINISH
};

void astsphinx_server(int sock);
char *reqtype_to_string(enum e_reqtype e);

#define ASTSPHINX_BUFSIZE 4096


#endif // __ASTSPHINX_H
