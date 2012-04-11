/* prefork.h (c) 2009, Christopher Jansen
 * Stupid-Simple preforking socket server.
 *
 *
 */
#ifndef __PREFORK_H
#define __PREFORK_H 1
int prefork_listen(int port, void (*f_server)(int));
#endif // __PREFORK_H
