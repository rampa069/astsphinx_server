/* prefork.c (c) 2009, Christopher Jansen
 * Simple pre-forking socket server with absolutely stupid
 * restart velocity checking
 */

#include "prefork.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int prefork_listen(int port, void (*f_server)(int))
{
  //port = 10069;
  if (f_server == NULL)
    {
      printf("No function pointer provided.\n");
      return -1;
    }

  struct sockaddr_in saddr;
  int nchildren = 10;
  int sock;
  int res, val, pid;

  int restarts, rtime, ctime, tdiff;
  int restart_limit = 5;
  int restart_time_limit = 2;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  if (res < 0)
    {
      printf("Error setting socket.\n");
      return -1;
    }

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = INADDR_ANY;
  res = bind(sock, (struct sockaddr *)&saddr, sizeof(saddr));
  if (res < 0)
    {
      printf("Error setting listen port.\n");
      return -1;
    }

  res = listen(sock, nchildren);
  if (res < 0)
    {
      printf("WTF can't listen?\n");
      return -1;
    }

  int x = 0;
  rtime = time(NULL);
  restarts = 0;
  while (1)
    {
      for (; x < nchildren; x++)
        {
          if ((pid = fork()) == 0)
            {
              (*f_server)(sock);
              exit(-1);
            }
        }

      wait(NULL);
      restarts++;
      --x;
      ctime = time(NULL);
      tdiff = ctime - rtime;
      float adj_diff = (float)tdiff / (float)restart_time_limit;
      float adj_limit = adj_diff * restart_limit;
      float adj_restarts = restarts / adj_diff;
      if (ctime - rtime > restart_time_limit)
        {
          printf("Restart checking timeout (%d - %d > %d), %d\n", ctime, rtime, restart_time_limit, restarts);
          printf("DIFF: %f, LIMIT: %f, RESTARTS: %f\n", adj_diff, adj_limit, adj_restarts);
          if (adj_restarts > adj_limit)
            {
              printf("Spawning too quickly\n");
              kill(0, 9);
            }
          rtime = time(NULL);
          restarts = 0;
        }
    }
  return 0;
}
